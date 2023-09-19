#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <signal.h>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <chrono>
#include <wiringPi.h>
#include "mcp4822.h"
#include "Points.h"
#include "IldaReader.h"
#include "ColorPalette.h"

using namespace std;

// use FIFOs for communication with node (+ remake wifi_man into smth for all)
// https://www.youtube.com/watch?v=2hba3etpoJg

void onInterrupt(int);

#define REPEAT_ONE 1

#define R_LASER 0
#define G_LASER 2
#define B_LASER 3

enum COLOR_SCHEME {
    RGB = 0,
    R,
    G,
    B
}

COLOR_SCHEME color_scheme = RGB;

// struct menu_option {
//     string text;
//     bool has_function;
//     void (*function_ptr)(int);
// };


// int SelectDelay() {
//     NumberSlider(&delay);
// }


// menu_option menu[] {
//     {"Project File", true, displayLaserOptionsMenu},
//     {"Laser options", false, void},
//     {"WiFi options", false, void},
// };
// menu_option submenu[][] {
//     { //project file
//     },
//     { //laser options
//         {"Select color scheme", true, SelectColorScheme}, // <RGB> (<R> <G> <B>)
//         {"Select delay (higher delay, lower speed)", true, SelectDelay},
//         {},
//     },
// }

int main(int argc, char **argv)
{

    //have a menu
    // every cycle:
    // - if projecting - project
    // - display output to an oled display
    // - read communication from node.js processes (prolly FIFOs)
    // - check if user turned encoder (have interrupt in the background that just adds to an int,, now in the cycle check that int)



    // Validate arguments.
    if (argc < 3)
    {
        cout << "ERROR: Arguments missing." << endl;
        cout << "Required: [pointDelay] [fileName]" << endl;
        return 1;
    }

    // kill previous instance
    ifstream pid_in("/tmp/lasershow.pid");
    if (!pid_in.is_open())
    {
        cerr << "failed to open pid file /tmp/lasershow.pid for reading" << endl;
        cout << "continuing without killing previous instance" << endl;
    }
    else
    {
        string pidstr;
        getline(pid_in, pidstr);
        pid_in.close();
        kill(stoi(pidstr), SIGINT);
    }

    ofstream pid_out("/tmp/lasershow.pid");
    if (!pid_out.is_open())
    {
        cerr << "failed to open pid file /tmp/lasershow.pid for writing" << endl;
        exit(1);
    }
    int pid = getpid();
    pid_out << pid << std::endl;
    pid_out.close();

    // Read arguments.
    int pointDelay = atoi(argv[1]);
    string fileName = argv[2];
    double frameDuration = 0.033; // ~30fps (1/30=0.033..).

    // Setup hardware communication stuff.
    wiringPiSetup();
    pinMode(0, OUTPUT);

    if (!mcp4822_initialize())
    {
        printf("Failed to initialize MCP4822.\n\r");
        return -1;
    }

    // Setup ILDA reader.
    Points points;
    IldaReader ildaReader;
    if (ildaReader.readFile(fileName))
    {
        printf("Provided file is a valid ILDA file.\n\r");
        ildaReader.getNextFrame(&points);
        printf("Points loaded in first frame: %d\n\r", points.size);
    }
    else
    {
        printf("Error opening ILDA file.\n\r");
        return (1);
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, onInterrupt);

    try {
        // Start the scanner loop with the current time.
        std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
        while (true)
        {
            // Exit if no points found. (used them all, points.next() decreases the sice of points)
            if (points.size == 0)
                break;

            // Move galvos to x,y position. (4096 is to invert horizontally)
            mcp4822_set_voltage(MCP_4822_CHANNEL_A, points.store[points.index * 3]);
            mcp4822_set_voltage(MCP_4822_CHANNEL_B, 4096 - points.store[(points.index * 3) + 1]);

            // Turn on/off laser diode.
            digitalWrite(0, points.store[(points.index * 3) + 2]);

            // Maybe wait a while there.
            if (pointDelay > 0)
                usleep(pointDelay);

            // In case there's no more points in the current frame check if it's time to load next frame.
            if (!points.next())
            {
                std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now() - start;
                if (elapsedSeconds.count() > frameDuration)
                {
                    start = std::chrono::system_clock::now();
                    digitalWrite(R_LASER, LOW);
                    digitalWrite(G_LASER, LOW);
                    digitalWrite(B_LASER, LOW);
                    break;
                }
            }
        }
        file.seekg(0);
    }
    catch (e) {
        cerr << e << endl;
    }

    // Cleanup and exit.
    digitalWrite(R_LASER, LOW);
    digitalWrite(G_LASER, LOW);
    digitalWrite(B_LASER, LOW);
    ildaReader.closeFile();
    mcp4822_deinitialize();
    return (0);
}

// Function that is called when program needs to be terminated.
void onInterrupt(int)
{
    printf("Turn off laser diode.\n\r");
    digitalWrite(R_LASER, LOW);
    digitalWrite(G_LASER, LOW);
    digitalWrite(B_LASER, LOW);
    ildaReader.closeFile();
    mcp4822_deinitialize();
    printf("Program was interrupted.\n\r");
    exit(1);
}
