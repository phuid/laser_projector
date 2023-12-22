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

  zmq::message_t received;

  bool repeat = 1;

  while (true)
  {
    bool current_repeat = repeat;
    std::string current_filename = "";

    command_receiver.recv(received, zmq::recv_flags::none);
    std::string received_string = received.to_string();
    if (received_string.rfind("PROJECT:", 0) == 0)
    {
      current_filename = received_string.substr(8);
    }
    else
    {
      std::cout << "invalid command : \"" << received_string << "\"" << std::endl;
      continue;
    }

    while (current_repeat)
    {
      int init_val = lasershow_init(current_filename, points, ildaReader, start);
      if (!init_val)
      {
        while (!lasershow_loop(points, ildaReader, start))
        {
          // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
          // - no,, just exit the loop lol
          // command_receiver.recv(received, zmq::recv_flags::dontwait);
          // if (received.to_string().length() > 0) // or some other message_t function
        }
        ildaReader.closeFile();
        lasershow_cleanup(0);
      }
      else
      {
        std::cout << "failed to init lasershow" << std::endl;
        ildaReader.closeFile();
        lasershow_cleanup(0);
        current_repeat = 0;
      }
    }
  }
}