#pragma once
#include "zmq.hpp"
#include <string>

void publish_message(zmq::socket_t &publisher, std::string message_string);

struct options_struct
{
  bool repeat = 1;
  int pointDelay = 0;
  uint16_t targetFrameTime = 33;
  float trapezoid_horizontal = 0;
  float trapezoid_vertical = 0;

  std::string project_filename;
  bool paused = 0;

  int loadFromFile(std::string filename)
  {
    return 1; // fail
  }
};