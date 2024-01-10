#include "zmq.hpp"
#include "my_zmq_helper.hpp"
#include <string>

void send_command(zmq::socket_t &publisher, std::string message_string)
{
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}