#pragma once
#include "zmq.hpp"
#include <string>

static void publish_message(zmq::socket_t &publisher, std::string message_string)
{
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}
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