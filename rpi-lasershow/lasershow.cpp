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
"
using namespace std;

void onInterrupt(int);

int main(int argc, char **argv)
{

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

    // Start the scanner loop with the current time.
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    while (true)
    {
        // Exit if no points found.
        if (points.size == 0)
            break;

        // Move galvos to x,y position. (4096 is to invert horizontally)
        mcp4822_set_voltage(MCP_4822_CHANNEL_A, points.store[points.index * 3]);
        mcp4822_set_voltage(MCP_4822_CHANNEL_B, 4096 - points.store[(points.index * 3) + 1]);

        // Turn on/off laser diode.
        if (points.store[(points.index * 3) + 2] == 1)
            digitalWrite(0, HIGH);
        else
            digitalWrite(0, LOW);

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
                digitalWrite(0, LOW);
                ildaReader.getNextFrame(&points);
            }
        }
    }

    // Cleanup and exit.
    ildaReader.closeFile();
    mcp4822_deinitialize();
    return (0);
}

// Function that is called when program needs to be terminated.
void onInterrupt(int)
{
    printf("Turn off laser diode.\n\r");
    digitalWrite(0, LOW);
    mcp4822_deinitialize();
    printf("Program was interrupted.\n\r");
    exit(1);
}
