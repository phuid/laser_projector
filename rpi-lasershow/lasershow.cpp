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
#include "IldaReader.h"
#include "lasershow.hpp"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

constexpr uint8_t LASER_PINS[3] = {0, 2, 3};


static ABElectronics_CPP_Libraries::ADCDACPi adcdac;
static IldaReader ildaReader;
static std::chrono::time_point<std::chrono::system_clock> start;

// Function that is called when program needs to be terminated.
void lasershow_cleanup(int sig)
{
    printf("Turn off laser diode.\n\r");
    for (size_t i = 0; i < 3; i++)
    {
        digitalWrite(LASER_PINS[i], 0);
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

void lasershow_start(zmq::socket_t &publisher){
    ildaReader.current_frame = 0;
    start = std::chrono::system_clock::now();
}

bool
lasershow_init(zmq::socket_t &publisher, std::string fileName)
{
    // Setup hardware communication stuff.
    wiringPiSetup();

    for (size_t i = 0; i < 3; i++)
    {
        pinMode(LASER_PINS[i], OUTPUT); // laser
    }

    if (adcdac.open_dac() == -1)
    {
        printf("Failed to initialize MCP4822.\n\r");
        publish_message(publisher, "ERROR: OTHER MCP4822 init fail");
        return 1;
    }
    adcdac.set_dac_gain(2);

    if (ildaReader.readFile(publisher, fileName) == 0)
    {
        printf("Provided file is a valid ILDA file.\n\r");
        publish_message(publisher, "INFO: succesful file read");
    }
    else
    {
        printf("Error opening ILDA file.\n\rfilename: %s", fileName.c_str());
        publish_message(publisher, "ERROR: EINVAL error opening ILDA filename: \"" + fileName + "\"");
        return 1;
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, lasershow_cleanup);

    // Start the scanner loop with the current time.
    start = std::chrono::system_clock::now();

    return 0;
}

// return: 0-success, 1-error, 2-end of projection
int lasershow_loop(zmq::socket_t &publisher, options_struct options)
{
    if (ildaReader.current_frame < ildaReader.sections.size())
    {
        std::cout << "position\t" << ildaReader.current_frame + 1 << "\tof\t" << ildaReader.sections.size() << std::endl;
        publish_message(publisher, "POS " + std::to_string(ildaReader.current_frame + 1) + " OF " + std::to_string(ildaReader.sections.size()));
        uint16_t current_point = 0;
        while (true) // always broken by time check
        {
            std::cout << "points[" << current_point << "]: x:" << ildaReader.sections[ildaReader.current_frame].points[current_point].x << ", y:" << ildaReader.sections[ildaReader.current_frame].points[current_point].y << ", R:" << static_cast<int>(ildaReader.sections[ildaReader.current_frame].points[current_point].red) << ", G:" << static_cast<int>(ildaReader.sections[ildaReader.current_frame].points[current_point].green) << ", B:" << static_cast<int>(ildaReader.sections[ildaReader.current_frame].points[current_point].blue) << std::endl;
            // Move galvos to x,y position.
            adcdac.set_dac_raw(ildaReader.sections[ildaReader.current_frame].points[current_point].x, 1); // TODO: trapezoid calc
            adcdac.set_dac_raw(ildaReader.sections[ildaReader.current_frame].points[current_point].y, 2);

            digitalWrite(LASER_PINS[0], ildaReader.sections[ildaReader.current_frame].points[current_point].red);
            digitalWrite(LASER_PINS[1], ildaReader.sections[ildaReader.current_frame].points[current_point].green);
            digitalWrite(LASER_PINS[2], ildaReader.sections[ildaReader.current_frame].points[current_point].blue);

            // Maybe wait a while there.
            if (options.pointDelay > 0)
                usleep(options.pointDelay);
            // check the time and move on to the next frame
            if (options.targetFrameTime < std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count())
            {
                start = std::chrono::system_clock::now();
                for (size_t i = 0; i < 3; i++)
                {
                    digitalWrite(LASER_PINS[i], 0);
                }
                if (!options.paused)
                {
                    ildaReader.current_frame++;
                    current_point = 0;
                }

                break;
            }
            current_point = (current_point + 1) % ildaReader.sections[ildaReader.current_frame].points.size();
        }
        ildaReader.current_frame++;
        return 0;
    }
    else
    {
        std::cout << "end of projection" << std::endl;
        return 2;
    }
}
