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
#include "ABE_ADCDACPi.h"
#include "Points.h"
#include "IldaReader.h"
#include "lasershow.hpp"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

using namespace std;

void lasershow_cleanup(int sig);

constexpr uint8_t laser_pins[3] = {0, 2, 3};
static uint8_t pin_index = 0;

static ABElectronics_CPP_Libraries::ADCDACPi adcdac;
static IldaReader ildaReader;
static Points points;
static std::chrono::time_point<std::chrono::system_clock> start;

int lasershow_init(zmq::socket_t &publisher, string fileName)
{
    // Setup hardware communication stuff.
    wiringPiSetup();

    for (size_t i = 0; i < 3; i++)
    {
        pinMode(laser_pins[i], OUTPUT); // laser
    }

    if (adcdac.open_dac() == -1)
    {
        printf("Failed to initialize MCP4822.\n\r");
        publish_message(publisher, "ERROR: OTHER MCP4822 init fail");
        return -1;
    }
    adcdac.set_dac_gain(2);

    if (ildaReader.readFile(fileName))
    {
        printf("Provided file is a valid ILDA file.\n\r");
        publish_message(publisher, "INFO: valid ILDA file");
        ildaReader.getNextFrame(publisher, &points);
        printf("Points loaded in first frame: %d\n\r", points.size);
    }
    else
    {
        printf("Error opening ILDA file.\n\rfilename: %s", fileName.c_str());
        publish_message(publisher, "ERR: EINVAL error opening ILDA filename: \"" + fileName + "\"");
        return (1);
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, lasershow_cleanup);

    // Start the scanner loop with the current time.
    start = std::chrono::system_clock::now();

    return 0;
}

// return 1 == break;
int lasershow_loop(zmq::socket_t &publisher, bool paused, int pointDelay, double frameDuration)
{
    // In case there's no more points in the current frame check if it's time to load next frame.
    while (points.next())
    {
        // Exit if no points found.
        if (points.size != 0)
        {
            // Move galvos to x,y position.
            adcdac.set_dac_raw(4096 - points.store[points.index * 3], 1);
            adcdac.set_dac_raw(4096 - points.store[(points.index * 3) + 1], 2);

            // Turn on/off laser diode.
            if (points.store[(points.index * 3) + 2] == 1)
            {
                digitalWrite(laser_pins[pin_index], 0);
                digitalWrite(laser_pins[++pin_index % 3], 1);
                pin_index = pin_index % 3;
                // printf("pin: %u", laser_pins[pin_index]);
            }
            else
                digitalWrite(laser_pins[pin_index], 0);

            // Maybe wait a while there.
            if (pointDelay > 0)
                usleep(pointDelay);
            if (frameDuration < static_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - start).count())
            {
                start = std::chrono::system_clock::now();
                for (size_t i = 0; i < 3; i++)
                {
                    digitalWrite(laser_pins[i], 0);
                }
                if (paused)
                {
                    points.index = 0;
                }
                else
                {
                    int getnext_val = ildaReader.getNextFrame(publisher, &points);
                    if (getnext_val)
                    {
                        return getnext_val;
                    }
                }
            }
        }
    }
    return 0;
}

// Function that is called when program needs to be terminated.
void lasershow_cleanup(int sig)
{
    printf("Turn off laser diode.\n\r");
    for (size_t i = 0; i < 3; i++)
    {
        digitalWrite(laser_pins[i], 0);
    }
    adcdac.close_dac();
    ildaReader.closeFile();
    printf("lasershow cleanup done.\n\r");
    if (sig != 0)
    {
        printf("stopped on interrupt\n\r");
        exit(1);
    }
}
