#include <wiringPi.h>
#include "ABE_ADCDACPi.h"

#include "zmq.hpp"
#include "lasershow.hpp"
#include "command.hpp"
#include "my_zmq_helper.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <map>

int main()
{
  // Setup ipc communication
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.bind("tcp://*:5556");

  zmq::socket_t command_receiver(ctx, zmq::socket_type::sub);
  command_receiver.bind("tcp://*:5557");
  command_receiver.set(zmq::sockopt::subscribe, "");

  zmq::message_t received;
  zmq::message_t msg_to_send;

  options_struct options;
  if (options.loadFromFile("lasershow.cfg"))
  {
    std::cout << "options couldnt be loaded from file" << std::endl;
  }

  publish_message(publisher, "INFO: lasershow ready");

  while (true)
  {
    // options.project_filename = "";
    command_receiver.recv(received, zmq::recv_flags::none); // blocking
    Command command;
    if (command.execute(received.to_string(), publisher, options) == -1)
    {
      continue;
    }

    bool first_repeat = 1;
    while (options.repeat || first_repeat)
    {
      first_repeat = 0;
      if (lasershow_init(publisher, options.project_filename) == 0)
      {
        while (first_repeat == 0) // also used just as a break flag (if 1 break)
        {
          // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
          // - no,, just exit the loop lol
          command_receiver.recv(received, zmq::recv_flags::dontwait);
          while (received.size() > 0)
          {
            int exec_val = command.execute(received.to_string(), publisher, options);
            if (exec_val == 1)
            {
              options.repeat = 0;
              first_repeat = 1;
              break;
            }
            else if (exec_val == 2)
            {
              first_repeat = 1;
              break;
            }
            command_receiver.recv(received, zmq::recv_flags::dontwait);
          }
          if (first_repeat == 0)
          {
            // draw, if an error or end of file is reached, break
            int loop_val = lasershow_loop(publisher, options);
            if (loop_val == 2)
              break;
            else if (loop_val == 1)
            {
              options.repeat = 0;
              break;
            }
          }
        }
      }
      else
      {
        std::cout << "failed to init lasershow" << std::endl;
        publish_message(publisher, "ERROR: failed to init lasershow");
        break;
      }

      lasershow_cleanup(0);
      publish_message(publisher, "INFO: lasershow cleanup");
      if (options.paused == 1) // stopped
        break;
    }
  }
}