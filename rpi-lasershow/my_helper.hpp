#pragma once
#include "zmq.hpp"
#include <string>

void publish_message(zmq::socket_t &publisher, std::string message_string);
void publish_message(std::string message_string);

struct options_struct
{
  bool repeat = 1;
  int pointDelay = 0;
  uint16_t targetFrameTime = 33;
  float trapezoid_horizontal = 0;
  float trapezoid_vertical = 0;

  std::string project_filename;
  bool paused = 0;

  float laser_brightness = 1;

  float laser_red_brightness = 1;
  float laser_green_brightness = 1;
  float laser_blue_brightness = 1;

  uint8_t laser_red_br_offset = 0;
  uint8_t laser_green_br_offset = 0;
  uint8_t laser_blue_br_offset = 0;

  bool loadFromFile(std::string filename)
  {
    return 1; // fail
  }
};