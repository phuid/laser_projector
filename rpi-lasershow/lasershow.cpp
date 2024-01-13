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
constexpr uint16_t BRIGHTNESS_LEVELS[] = {0, 100, 200, 512, 1024, 2047, 4095}; // FIXME: pwm for laser isnt linear (idk random values)

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

void lasershow_start(zmq::socket_t &publisher)
{
    ildaReader.current_frame_index = 0;
    start = std::chrono::system_clock::now();
}

bool lasershow_init(zmq::socket_t &publisher, std::string fileName)
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
    if (ildaReader.current_frame_index < ildaReader.sections.size())
    {
        std::cout << "position\t" << ildaReader.current_frame_index + 1 << "\tof\t" << ildaReader.sections.size() << std::endl;
        publish_message(publisher, "INFO: POS " + std::to_string(ildaReader.current_frame_index + 1) + " OF " + std::to_string(ildaReader.sections.size()));
        uint16_t current_point_index = 0;
        while (true) // always broken by time check
        {
            point &current_point = ildaReader.sections[ildaReader.current_frame_index].points[current_point_index];
            // std::cout << "points[" << current_point_index << "]: x:" << current_point.x << ", y:" << current_point.y << ", R:" << static_cast<int>(current_point.red) << ", G:" << static_cast<int>(current_point.green) << ", B:" << static_cast<int>(current_point.blue) << std::endl;
            // Move galvos to x,y position.
            adcdac.set_dac_raw(current_point.x, 1); // TODO: trapezoid calc
            adcdac.set_dac_raw(current_point.y, 2);

            // TODO: PWM insteal of digitalwrite
            digitalWrite(LASER_PINS[0], BRIGHTNESS_LEVELS[(current_point.red * sizeof(BRIGHTNESS_LEVELS)) / 255]);
            digitalWrite(LASER_PINS[1], BRIGHTNESS_LEVELS[(current_point.green * sizeof(BRIGHTNESS_LEVELS)) / 255]);
            digitalWrite(LASER_PINS[2], BRIGHTNESS_LEVELS[(current_point.blue * sizeof(BRIGHTNESS_LEVELS)) / 255]);

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
                    ildaReader.current_frame_index++;
                    current_point_index = 0;
                }

                break;
            }
            current_point_index = (current_point_index + 1) % ildaReader.sections[ildaReader.current_frame_index].points.size();
        }
        if (!options.paused)
            ildaReader.current_frame_index++;
        return 0;
    }
    else
    {
        std::cout << "end of projection" << std::endl;
        return 2;
    }
}
