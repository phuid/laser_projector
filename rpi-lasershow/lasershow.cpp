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
#include "lasershow.hpp"

using namespace std;

void lasershow_cleanup(int);

int lasershow_init(string fileName, Points &points, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start)
{
    // Setup hardware communication stuff.
    wiringPiSetup();
    pinMode(0, OUTPUT);

    if (!mcp4822_initialize())
    {
        printf("Failed to initialize MCP4822.\n\r");
        return -1;
    }

    if (ildaReader.readFile(fileName))
    {
        printf("Provided file is a valid ILDA file.\n\r");
        ildaReader.getNextFrame(&points);
        printf("Points loaded in first frame: %d\n\r", points.size);
    }
    else
    {
        printf("Error opening ILDA file.\n\rfilename: %s", fileName.c_str());
        return (1);
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, lasershow_cleanup);

    // Start the scanner loop with the current time.
    start = std::chrono::system_clock::now();

    return 0;
}

// return 1 == break;
bool lasershow_loop(Points &points, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start, int pointDelay, double frameDuration)
{
    // In case there's no more points in the current frame check if it's time to load next frame.
    while (points.next())
    {
        // Exit if no points found.
        if (points.size == 0)
            break;

        // Move galvos to x,y position.
        mcp4822_set_voltage(MCP_4822_CHANNEL_A, 4096 - points.store[points.index * 3]);
        mcp4822_set_voltage(MCP_4822_CHANNEL_B, points.store[(points.index * 3) + 1]);

        // Turn on/off laser diode.
        if (points.store[(points.index * 3) + 2] == 1)
            digitalWrite(0, HIGH);
        else
            digitalWrite(0, LOW);

        // Maybe wait a while there.
        if (pointDelay > 0)
            usleep(pointDelay);
        if (frameDuration < static_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - start).count())
        {
            start = std::chrono::system_clock::now();
            digitalWrite(0, LOW);
            if (ildaReader.getNextFrame(&points))
            {
                return 1;
            }
        }
    }
    return 0;
}

// Function that is called when program needs to be terminated.
void lasershow_cleanup(int)
{
    printf("Turn off laser diode.\n\r");
    digitalWrite(0, LOW);
    mcp4822_deinitialize();
    printf("lasershow cleanup done.\n\r");
    exit(1);
}
