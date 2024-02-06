#pragma once
#include "zmq.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>

void publish_message(zmq::socket_t &publisher, std::string message_string);
void publish_message(std::string message_string);

struct options_struct
{
  bool repeat = 1;
  int pointDelay = 0;
  uint16_t targetFrameTime = 33;

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

  uint8_t laser_red_br_offset = 0;
  uint8_t laser_green_br_offset = 0;
  uint8_t laser_blue_br_offset = 0;

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
      if (lines.size() > 13)
      {
        repeat = stoi(lines[0]);

        pointDelay = stoi(lines[1]);
        targetFrameTime = stoi(lines[2]);

        scale_x = stof(lines[3]);
        scale_y = stof(lines[4]);

        move_x = stof(lines[5]);
        move_y = stof(lines[6]);

        trapezoid_horizontal = stof(lines[6]);
        trapezoid_vertical = stof(lines[7]);

        laser_brightness = stof(lines[8]);

        laser_red_brightness = stof(lines[9]);
        laser_green_brightness = stof(lines[10]);
        laser_blue_brightness = stof(lines[11]);

        laser_red_br_offset = stoi(lines[12]);
        laser_green_br_offset = stoi(lines[13]);
        laser_blue_br_offset = stoi(lines[14]);

        std::cout
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
      ofile << "#internal file for lasershow executable - automatically generated" << std::endl
            << "#lines begining with # will be ignored" << std::endl
            << "#bool" << std::endl
            << "repeat=" << std::to_string(repeat) << std::endl
            << "#int (microseconds)" << std::endl
            << "pointDelay=" << std::to_string(pointDelay) << std::endl
            << "#uint16_t (milliseconds)" << std::endl
            << "targetFrameTime=" << std::to_string(targetFrameTime) << std::endl
            << "#float (any, limited at runtime by the projection dimensions)" << std::endl
            << "scale_x=" << std::to_string(scale_x) << std::endl
            << "scale_y=" << std::to_string(scale_y) << std::endl
            << "move_x=" << std::to_string(move_x) << std::endl
            << "move_y=" << std::to_string(move_y) << std::endl
            << "#float (-1 -- +1)" << std::endl
            << "trapezoid_horizontal=" << std::to_string(trapezoid_horizontal) << std::endl
            << "trapezoid_vertical=" << std::to_string(trapezoid_vertical) << std::endl
            << "#float" << std::endl
            << "#diode brightness calculation = static_cast<int>(options.laser_brightness * options.laser_red_brightness * (current_point.color[0] + options.laser_red_br_offset))" << std::endl
            << "laser_brightness=" << std::to_string(laser_brightness) << std::endl
            << "laser_red_brightness=" << std::to_string(laser_red_brightness) << std::endl
            << "laser_green_brightness=" << std::to_string(laser_green_brightness) << std::endl
            << "laser_blue_brightness=" << std::to_string(laser_blue_brightness) << std::endl
            << "#uint8_t" << std::endl
            << "laser_red_br_offset=" << std::to_string(laser_red_br_offset) << std::endl
            << "laser_green_br_offset=" << std::to_string(laser_green_br_offset) << std::endl
            << "laser_blue_br_offset=" << std::to_string(laser_blue_br_offset) << std::endl;
      return 0;
    }
    else
    {
      std::cout << "config file doesnt exist" << std::endl;
      return 1; // fail
    }
  }
};