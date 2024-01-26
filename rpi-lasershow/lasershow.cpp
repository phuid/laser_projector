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
#include <lgpio.h>
#include "ABE_ADCDACPi.h"
#include "IldaReader.h"
#include "lasershow.hpp"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

constexpr uint8_t LASER_PINS[3] = {22, 27, 17};


//internals
#constexpr uint8_t GPIO_CHIP_NUM = 2;

static ABElectronics_CPP_Libraries::ADCDACPi adcdac;
static IldaReader ildaReader;
static std::chrono::time_point<std::chrono::system_clock> start;
static int gpio_chip_handle;

// Function that is called when program needs to be terminated.
void lasershow_cleanup(int sig)
{
    printf("Turn off laser diode.\n\r");
    for (size_t i = 0; i < 3; i++)
    {
        gpioWrite(LASER_PINS[i], 0);
    }
    adcdac.close_dac();
    if (lgGpiochipClose(gpio_chip_handle) < 0) {
        std::cout << "couldnt close gpio chip";
    }
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
    gpio_chip_handle = lgGpiochipOpen(GPIO_CHIP_NUM);
    if (gpio_chip_handle < 0)
    {
        // pigpio initialisation failed.
        std::cout << "init fail" << std::endl;
        exit(1);
    }

    for (size_t i = 0; i < 3; i++)
    {
        lgGpioClaimOutput(gpio_chip_handle, LG_SET_PULL_DOWN, LASER_PINS[i], 0);
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
            adcdac.set_dac_raw(4096 - current_point.y, 2);

            // TODO: PWM insteal of digitalwrite
            if (current_point.laser_on) // blanking bit (moving the mirrors with laser off)
            {
                lgTxPwm(gpio_chip_handle, LASER_PINS[0], 10000, (static_cast<float>(current_point.red) / 255.f) * 100.f, 0, 0);
                lgTxPwm(gpio_chip_handle, LASER_PINS[1], 10000, (static_cast<float>(current_point.green) / 255.f) * 100.f, 0, 0);
                lgTxPwm(gpio_chip_handle, LASER_PINS[2], 10000, (static_cast<float>(current_point.blue) / 255.f) * 100.f, 0, 0);
                // std::cout << current_point_index << ":" << static_cast<int>(current_point.red) << "," << static_cast<int>(current_point.green) << "," << static_cast<int>(current_point.blue) << "," << std::endl;
            }
            else
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    lgGpioWrite(gpio_chip_handle, LASER_PINS[i], 0);
                }
                // std::cout << current_point_index << ":" << "---------" << std::endl;
            }

            // Maybe wait a while there.
            if (options.pointDelay > 0)
                usleep(options.pointDelay);
            // check the time and move on to the next frame
            if (options.targetFrameTime < std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count())
            {
                start = std::chrono::system_clock::now();
                for (size_t i = 0; i < 3; i++)
                {
                    lgGpioWrite(gpio_chip_handle, LASER_PINS[i], 0);
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
