#include "lasershow.hpp"
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <string>

int main()
{
  //  Prepare our context and sockets for communication
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.bind("tcp://*:5556");

  zmq::socket_t command_receiver(ctx, zmq::socket_type::sub);
  command_receiver.bind("tcp://*:5557");
  command_receiver.set(zmq::sockopt::subscribe, "");
}