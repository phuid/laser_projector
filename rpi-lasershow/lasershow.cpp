#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <signal.h>
#include <stdexcept>
#include <time.h>
#include <cmath>
#include <unistd.h>
#include <iostream>
#include <string>
#include <chrono>
#include <pigpio.h>
#include "ABE_ADCDACPi.h"
#include "IldaReader.h"
#include "lasershow.hpp"

#include "zmq.hpp"
#include "my_helper.hpp"

constexpr uint8_t LASER_PINS[3] = {22, 27, 17};

constexpr int ILDA_MAX = 4095;
constexpr int DAC_OUT_RAW_MAX = 4095; // 12bit dac - 2^12 - 1 = 4096 - 1 = 4095
constexpr float TRAPEZOID_MAX = 1; // prolly defined in options

// internals
static ABElectronics_CPP_Libraries::ADCDACPi adcdac;

// Function that is called when program needs to be terminated.
void lasershow_cleanup(int sig)
{
    printf("Turn off laser diode.\n\r");
    for (size_t i = 0; i < 3; i++)
    {
        gpioWrite(LASER_PINS[i], 0);
    }
    adcdac.close_dac();
    gpioTerminate();
    printf("lasershow cleanup done.\n\r");
    publish_message("INFO: lasershow cleanup done.");
    if (sig != 0)
    {
        printf("stopped on interrupt\n\r");
        exit(1);
    }
}

void lasershow_start(zmq::socket_t &publisher, IldaReader &ildaReader, options_struct &options)
{
    ildaReader.current_frame_index = 0;
    options.start = std::chrono::system_clock::now();
}

void calculate_points(zmq::socket_t &publisher, options_struct options, IldaReader &ildaReader)
{
    publish_message(publisher, "DISPLAY: recalculating points...");

    int lowest_x;
    int highest_x;
    int lowest_y;
    int highest_y;

    ildaReader.projection_sections = ildaReader.sections_from_file;

    lowest_x = ildaReader.sections_from_file[0].points[0].x;
    highest_x = ildaReader.sections_from_file[0].points[0].x;
    lowest_y = ildaReader.sections_from_file[0].points[0].y;
    highest_y = ildaReader.sections_from_file[0].points[0].y;
    // find furthest points in all frames and all directions (x+, x-, y+, y-)
    for (size_t i = 0; i < ildaReader.sections_from_file.size(); i++)
    {
        for (size_t u = 0; u < ildaReader.sections_from_file[i].points.size(); u++)
        {
            // std::cout << "point" << u << "lx:" << lowest_x << "hx:" << highest_x << "ly:" << lowest_y << "hy:" << highest_y << std::endl;
            if (ildaReader.sections_from_file[i].points[u].x > highest_x)
            {
                highest_x = ildaReader.sections_from_file[i].points[u].x;
            }
            else if (ildaReader.sections_from_file[i].points[u].x < lowest_x)
            {
                lowest_x = ildaReader.sections_from_file[i].points[u].x;
            }

            if (ildaReader.sections_from_file[i].points[u].y > highest_y)
            {
                highest_y = ildaReader.sections_from_file[i].points[u].y;
            }
            else if (ildaReader.sections_from_file[i].points[u].y < lowest_y)
            {
                lowest_y = ildaReader.sections_from_file[i].points[u].y;
            }
        }
    }

    // center the image -- also needed for scaling
    int center_x = (lowest_x + highest_x) / 2;
    int center_y = (lowest_y + highest_y) / 2;

    for (size_t i = 0; i < ildaReader.sections_from_file.size(); i++)
    {
        for (size_t u = 0; u < ildaReader.sections_from_file[i].points.size(); u++)
        {
            ildaReader.sections_from_file[i].points[u].x += ((ILDA_MAX / 2) - center_x);
            ildaReader.sections_from_file[i].points[u].y += ((ILDA_MAX / 2) - center_y);
        }
    }
    highest_x += ((ILDA_MAX / 2) - center_x);
    lowest_x += ((ILDA_MAX / 2) - center_x);
    highest_y += ((ILDA_MAX / 2) - center_y);
    lowest_y += ((ILDA_MAX / 2) - center_y);

    int lowest_x_after_scale = map(lowest_x, s_l_x, s_h_x, (DAC_RAW_MAX / 2) - options.scale_x * (DAC_RAW_MAX / 2), (DAC_RAW_MAX / 2) + options.scale_x * (DAC_RAW_MAX / 2));
    int highest_x_after_scale = map(highest_x, s_l_x, s_h_x, (DAC_RAW_MAX / 2) - options.scale_x * (DAC_RAW_MAX / 2), (DAC_RAW_MAX / 2) + options.scale_x * (DAC_RAW_MAX / 2));
    
    int lowest_y_after_scale = map(lowest_y, s_l_y, s_h_y, (DAC_RAW_MAX / 2) - options.scale_y * (DAC_RAW_MAX / 2), (DAC_RAW_MAX / 2) + options.scale_y * (DAC_RAW_MAX / 2));
    int highest_y_after_scale = map(highest_y, s_l_y, s_h_y, (DAC_RAW_MAX / 2) - options.scale_y * (DAC_RAW_MAX / 2), (DAC_RAW_MAX / 2) + options.scale_y * (DAC_RAW_MAX / 2));


    // if (options.scale_up_proportionally) {
    //     if (lowest_x > lowest_y) {
    //         lowest_x = lowest_y;
    //     }
    //     else { //idc if theyre the same - then nothing changes
    //         lowest_y = lowest_x;
    //     }
    //     if (highest_x < highest_y) {
    //         highest_x = highest_y;
    //     }
    //     else { //idc if theyre the same - then nothing changes
    //         highest_y = highest_x;
    //     }
    // }
    int move_x = options.move_x * ILDA_MAX;
    if (move_x != 0)
    {
        // check if the translation doesnt go out of bounds ? set to max possible value
        if (highest_x + move_x > ILDA_MAX)
        {
            publish_message(publisher, "WARN: X translation set to max possible value");
            move_x = ILDA_MAX - highest_x;
        }
        else if (lowest_x + move_x < 0)
        {
            publish_message(publisher, "WARN: X translation set to min possible value");
            move_x = -lowest_x;
        }
    }
    int move_y = options.move_y * ILDA_MAX;
    if (move_y != 0)
    {
        std::cout << "highY: " << highest_y << "    moveY: " << move_y <<std::endl;
        if (highest_y + move_y > ILDA_MAX)
        {
            publish_message(publisher, "WARN: Y translation set to max possible value");
            move_y = ILDA_MAX - highest_y;
        }
        else if (lowest_y + move_y < 0)
        {
            publish_message(publisher, "WARN: Y translation set to min possible value");
            move_y = -lowest_y;
        }
    }
    for (size_t i = 0; i < ildaReader.projection_sections.size(); i++)
    {
        section &current_section = ildaReader.projection_sections[i];
        for (size_t u = 0; u < current_section.points.size(); u++)
        {
            point &current_point = current_section.points[u];

            if (options.scale_x != 1)
            {
                // current_point.x = (current_point.x - (DAC_RAW_MAX / 2)) * options.scale_x + (DAC_RAW_MAX / 2);
                // lowest_x = (lowest_x - (DAC_RAW_MAX / 2)) * options.scale_x + (DAC_RAW_MAX / 2);
                // highest_x = (highest_x - (DAC_RAW_MAX / 2)) * options.scale_x + (DAC_RAW_MAX / 2);
                
                current_point.x = map(current_point.x, lowest_x, highest_x, (DAC_RAW_MAX / 2) - options.scale_x * (DAC_RAW_MAX / 2), (DAC_RAW_MAX / 2) + options.scale_x * (DAC_RAW_MAX / 2));
            }
            if (options.scale_y != 1)
            {
                // current_point.y = (current_point.y - (DAC_RAW_MAX / 2)) * options.scale_y + (DAC_RAW_MAX / 2);
                // lowest_y = (lowest_y - (DAC_RAW_MAX / 2)) * options.scale_y + (DAC_RAW_MAX / 2);
                // highest_y = (highest_y - (DAC_RAW_MAX / 2)) * options.scale_y + (DAC_RAW_MAX / 2);
                
                current_point.y = map(current_point.y, lowest_y, highest_y, (DAC_RAW_MAX / 2) - options.scale_y * ((DAC_RAW_MAX / 2) - lowest_y), (DAC_RAW_MAX / 2) + options.scale_y * (highest_y - (DAC_RAW_MAX / 2)));
            }

            if (move_x != 0)
            {
                current_point.x = current_point.x + move_x;
            }
            if (move_y != 0)
            {
                current_point.y = current_point.y + move_y;
            }

            int calc_coord_x = current_point.x;
            int calc_coord_y = current_point.y;

            if (options.trapezoid_horizontal != 0)
            {
                float tr = fabs(options.trapezoid_horizontal);

                int y = (options.trapezoid_horizontal > 0) ? (current_point.y - lowest_y) : (highest_y - current_point.y);
                float ycoef = static_cast<float>(y) / (highest_y - lowest_y);
                int offset = tr * ((highest_y - lowest_y) / 2) * ycoef;
                std::cout << "ycoef" << ycoef << std::endl;

                calc_coord_x = map(current_point.x, lowest_x, highest_x, lowest_x + offset, highest_x - offset);
            }
            if (options.trapezoid_vertical != 0)
            {
                float tr = fabs(options.trapezoid_vertical);

                int x = (options.trapezoid_vertical > 0) ? (current_point.x - lowest_x) : (highest_x - current_point.x);
                float xcoef = static_cast<float>(x) / (highest_x - lowest_x);
                int offset = tr * ((highest_x - lowest_x) / 2) * xcoef;

                calc_coord_y = map(current_point.y, lowest_y, highest_y, lowest_y + offset, highest_y - offset);
            }
            current_point.x = calc_coord_x;
            current_point.y = calc_coord_y;

            if (current_point.x > ILDA_MAX) {
                current_point.x = ILDA_MAX;
            }
            else if (current_point.x < 0) {
                current_point.x = 0;
            }
            if (current_point.y > ILDA_MAX) {
                current_point.y = ILDA_MAX;
            }
            else if (current_point.y < 0) {
                current_point.y = 0;
            }

            if (current_point.laser_on) // blanking bit (moving the mirrors with laser off)
            {
                // std::cout << "r:" << static_cast<int>(current_point.color[0]) << "g:" << static_cast<int>(current_point.color[1]) << "b:" << static_cast<int>(current_point.color[2]) << "-->";
                int calc_brs[3] = {
                    static_cast<int>(options.laser_brightness * options.laser_red_brightness * (current_point.color[0] + options.laser_red_br_offset)),
                    static_cast<int>(options.laser_brightness * options.laser_green_brightness * (current_point.color[1] + options.laser_green_br_offset)),
                    static_cast<int>(options.laser_brightness * options.laser_blue_brightness * (current_point.color[2] + options.laser_blue_br_offset))};

                for (uint8_t i = 0; i < 3; i++)
                {
                    if (calc_brs[i] > 255)
                        calc_brs[i] = 255;
                    if (calc_brs[i] < 0)
                        calc_brs[i] = 0;
                }
                for (uint8_t i = 0; i < 3; i++)
                {
                    current_point.color[i] = calc_brs[i];
                }
                // std::cout << "r:" << static_cast<int>(current_point.color[0]) << "g:" << static_cast<int>(current_point.color[1]) << "b:" << static_cast<int>(current_point.color[2]) << std::endl;
                // std::cout << current_point_index << ":" << static_cast<int>(current_point.red) << "," << static_cast<int>(current_point.green) << "," << static_cast<int>(current_point.blue) << "," << std::endl;
            }
            else
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    current_point.color[i] = 0;
                }
                // std::cout << current_point_index << ":" << "---------" << std::endl;
            }
            
            // Map ILDA values to DAC value range and store the data to vector
            current_point.x = map(current_point.x, 0, 65535, 0, 4095);
            current_point.y = map(current_point.y, 0, 65535, 0, 4095);
        }
    }
}

bool lasershow_init(zmq::socket_t &publisher, options_struct &options, IldaReader &ildaReader)
{
    for (size_t i = 0; i < 3; i++)
    {
        gpioSetMode(LASER_PINS[i], PI_OUTPUT);
        gpioSetPullUpDown(LASER_PINS[i], PI_PUD_DOWN);
        gpioSetPWMfrequency(LASER_PINS[i], 100000);
    }

    if (!adcdac.open_dac())
    {
        printf("Failed to initialize MCP4822.\n\r");
        publish_message(publisher, "ERROR: OTHER mpc4822 init fail");
        return 1;
    }

    if (ildaReader.readFile(publisher, options.project_filename) == 0 && ildaReader.sections_from_file.size() != 0)
    {
        printf("succesful file read\n\r");
        publish_message(publisher, "INFO: succesful file read");
    }
    else
    {
        if (ildaReader.sections_from_file.size() == 0)
        {
            printf("no frames loaded, stopped file opening\n\r");
            publish_message(publisher, "INFO: no frames loaded, stopped file opening");
            return 1;
        }
        printf("Error opening ILDA file.\n\rfilename: %s", options.project_filename.c_str());
        publish_message(publisher, "ERROR: EINVAL error opening ILDA filename: \"" + options.project_filename + "\"");
        return 1;
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, lasershow_cleanup);

    calculate_points(publisher, options, ildaReader);

    // Start the scanner loop with the current time.
    lasershow_start(publisher, ildaReader, options);

    return 0;
}

uint16_t get_frame_by_time(zmq::socket_t &publisher, const options_struct &options)
{
    uint16_t frame_index = 0;
    uint16_t frame_time = options.target_frame_time;
    if (frame_time < 1)
    {
        publish_message(publisher, "WARNING: frame index calculaed from target_frame_time 1, instead of 0, which is the actual value.");
        frame_time = 1;
    }
    std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
    int64_t millis_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - options.start).count();
    frame_index = millis_since_start / options.target_frame_time;

    return frame_index;
}

// return: 0-success, 1-error, 2-end of projection
int lasershow_loop(zmq::socket_t &publisher, options_struct options, IldaReader &ildaReader)
{
    if (!options.paused)
    {
        ildaReader.current_frame_index = get_frame_by_time(publisher, options);
    }
    if (ildaReader.current_frame_index < ildaReader.projection_sections.size())
    {
        std::cout << "frame\t" << ildaReader.current_frame_index + 1 << "\tof\t" << ildaReader.projection_sections.size() << std::endl;
        publish_message(publisher, "INFO: FRAME " + std::to_string(ildaReader.current_frame_index + 1) + " OF " + std::to_string(ildaReader.projection_sections.size()));
        for (point &current_point : ildaReader.projection_sections[ildaReader.current_frame_index].points) // always broken by time check
        {
            // std::cout << "points[" << current_point_index << "]: x:" << current_point.x << ", y:" << current_point.y << ", R:" << static_cast<int>(current_point.red) << ", G:" << static_cast<int>(current_point.green) << ", B:" << static_cast<int>(current_point.blue) << std::endl;
            // Move galvos to x,y position.
            adcdac.set_dac_raw(DAC_OUT_RAW_MAX - current_point.y, 1);
            adcdac.set_dac_raw(current_point.x, 2);

            for (uint8_t i = 0; i < 3; i++)
            {
                gpioPWM(LASER_PINS[i], current_point.color[i]);
            }

            // Maybe wait a while there.
            if (options.point_delay > 0)
                gpioDelay(options.point_delay);
            // check the time and move on to the next frame
            if (!options.always_project_full_frames && options.target_frame_time * (ildaReader.current_frame_index + 1) < std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - options.start).count())
            {
                options.start = std::chrono::system_clock::now();
                for (size_t i = 0; i < 3; i++)
                {
                    gpioWrite(LASER_PINS[i], 0);
                }

                break;
            }
        }
        if (options.paused) {
            options.start = std::chrono::system_clock::now() - std::chrono::milliseconds(options.target_frame_time * ildaReader.current_frame_index);
            std::cout << "time:" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - options.start).count() << std::endl;
        }

        return 0;
    }
    else
    {
        std::cout << "end of projection" << std::endl;
        return 2;
    }
}
