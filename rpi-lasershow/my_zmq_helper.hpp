#pragma once
#include "zmq.hpp"
#include <string>

static void publish_message(zmq::socket_t &publisher, std::string message_string)
{
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}
