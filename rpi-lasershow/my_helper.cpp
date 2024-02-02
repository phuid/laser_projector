#include "zmq.hpp"
#include "my_helper.hpp"
#include <string>

void publish_message(zmq::socket_t &publisher, std::string message_string)
{
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}

void publish_message(std::string message_string)
{
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.connect("tcp://*:5556");
  
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}