#pragma once
#include "zmq.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>

constexpr unsigned CS = 16;
constexpr unsigned MISO = 19;
constexpr unsigned SCLK = 21;


uint8_t bat_raw ();

void publish_message(zmq::socket_t &publisher, std::string message_string);
void publish_message(std::string message_string);

struct options_struct
{
  bool repeat = 1;
  int point_delay = 0;
  uint16_t target_frame_time = 33;
  bool time_accurate_framing = 1;
  bool always_project_full_frames = 1;

  float scale_x = 1;
  float scale_y = 1;

  float move_x = 0;
  float move_y = 0;

  float trapezoid_horizontal = 0;
  float trapezoid_vertical = 0;

  std::string project_filename;
  bool paused = 0;
  std::chrono::time_point<std::chrono::system_clock> start;

  float laser_brightness = 1;

  float laser_red_brightness = 1;
  float laser_green_brightness = 1;
  float laser_blue_brightness = 1;

  int16_t laser_red_br_offset = 0;
  int16_t laser_green_br_offset = 0;
  int16_t laser_blue_br_offset = 0;

  bool loadFromFile(zmq::socket_t &publisher, std::string filename)
  {
    std::ifstream ifile(filename);
    std::vector<std::string> lines;
    if (ifile.is_open())
    {
      for (std::string line; std::getline(ifile, line);)
      {
        if (line.length() > 0 && line[0] != '#')
        {
          lines.push_back(line.substr(line.find('=') + 1));
        }
      }
      if (lines.size() > 17)
      {
        int i = 0;
        repeat = stoi(lines[i]); i++;

        point_delay = stoi(lines[i]); i++;
        target_frame_time = stoi(lines[i]); i++;
        time_accurate_framing = stoi(lines[i]); i++;
        always_project_full_frames = stoi(lines[i]); i++;

        scale_x = stof(lines[i]); i++;
        scale_y = stof(lines[i]); i++;

        move_x = stof(lines[i]); i++;
        move_y = stof(lines[i]); i++;

        trapezoid_horizontal = stof(lines[i]); i++;
        trapezoid_vertical = stof(lines[i]); i++;

        laser_brightness = stof(lines[i]); i++;

        laser_red_brightness = stof(lines[i]); i++;
        laser_green_brightness = stof(lines[i]); i++;
        laser_blue_brightness = stof(lines[i]); i++;

        laser_red_br_offset = stoi(lines[i]); i++;
        laser_green_br_offset = stoi(lines[i]); i++;
        laser_blue_br_offset = stoi(lines[i]); i++;

#define OPTIONS_PRINT_STREAM << "#internal file for lasershow executable - automatically generated" << std::endl \
            << "#lines begining with # will be ignored" << std::endl \
            << "#bool" << std::endl \
            << "repeat=" << std::to_string(repeat) << std::endl \
            << "#int (microseconds)" << std::endl \
            << "point_delay=" << std::to_string(point_delay) << std::endl \
            << "#uint16_t (milliseconds)" << std::endl \
            << "target_frame_time=" << std::to_string(target_frame_time) << std::endl \
            << "#bools" << std::endl \
            << "time_accurate_framing=" << std::to_string(time_accurate_framing) << std::endl \
            << "always_project_full_frames=" << std::to_string(always_project_full_frames) << std::endl \
            << "#float (any, limited at runtime by the projection dimensions)" << std::endl \
            << "scale_x=" << std::to_string(scale_x) << std::endl \
            << "scale_y=" << std::to_string(scale_y) << std::endl \
            << "move_x=" << std::to_string(move_x) << std::endl \
            << "move_y=" << std::to_string(move_y) << std::endl \
            << "#float (-1 -- +1)" << std::endl \
            << "trapezoid_horizontal=" << std::to_string(trapezoid_horizontal) << std::endl \
            << "trapezoid_vertical=" << std::to_string(trapezoid_vertical) << std::endl \
            << "#float" << std::endl \
            << "#diode brightness calculation = static_cast<int>(options.laser_brightness * options.laser_red_brightness * (current_point.color[0] + options.laser_red_br_offset))" << std::endl \
            << "laser_brightness=" << std::to_string(laser_brightness) << std::endl \
            << "laser_red_brightness=" << std::to_string(laser_red_brightness) << std::endl \
            << "laser_green_brightness=" << std::to_string(laser_green_brightness) << std::endl \
            << "laser_blue_brightness=" << std::to_string(laser_blue_brightness) << std::endl \
            << "#int16_t" << std::endl \
            << "laser_red_br_offset=" << std::to_string(laser_red_br_offset) << std::endl \
            << "laser_green_br_offset=" << std::to_string(laser_green_br_offset) << std::endl \
            << "laser_blue_br_offset=" << std::to_string(laser_blue_br_offset) << std::endl

        std::cout OPTIONS_PRINT_STREAM;
      }
      ifile.close();
      return 0;
    }
    else
    {
      std::cout << "config file " << filename << " doesnt exist" << std::endl;
      publish_message(publisher, "ERROR: config file " + filename + " doesnt exist");
      return 1; // fail
    }
  }

  bool saveToFile(std::string filename)
  {
    std::ofstream ofile(filename);
    if (ofile.is_open())
    {
      ofile OPTIONS_PRINT_STREAM;
      return 0;
    }
    else
    {
      std::cout << "config file doesnt exist" << std::endl;
      return 1; // fail
    }
  }
};