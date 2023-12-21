#include "lasershow.hpp"
#include "Points.h"
#include "IldaReader.h"
#include <chrono>

#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <string>
#include <iostream>

int main()
{
  //  Prepare our context and sockets for communication
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.bind("tcp://*:5556");

  zmq::socket_t command_receiver(ctx, zmq::socket_type::sub);
  command_receiver.bind("tcp://*:5557");
  command_receiver.set(zmq::sockopt::subscribe, "");

  // Setup ILDA reader.
  Points points;
  IldaReader ildaReader;
  std::chrono::time_point<std::chrono::system_clock> start;

  while (true)
  {
    // if (got_something_to_project?)
    if (!lasershow_init("../ild/clock.ild", points, ildaReader, start))
    {
      while (!lasershow_loop(points, ildaReader, start))
      {
        // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
      }
      ildaReader.closeFile();
      lasershow_cleanup(0);
    }
    else
    {
      std::cout << "failed to init lasershow" << std::endl;
    }
  }
}