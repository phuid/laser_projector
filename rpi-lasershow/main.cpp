#include "ABE_ADCDACPi.h"

#include "zmq.hpp"
#include "lasershow.hpp"
#include "command.hpp"
#include "my_helper.hpp"

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <pigpio.h>

int main()
{
  constexpr uint8_t LASER_PINS[3] = {22, 27, 17};

  for (size_t i = 0; i < 3; i++)
  {
    gpioSetMode(LASER_PINS[i], PI_OUTPUT);
    gpioSetPullUpDown(LASER_PINS[i], PI_PUD_DOWN);
    gpioSetPWMfrequency(LASER_PINS[i], 100000);
    gpioWrite(LASER_PINS[i], 0);
  }

  // Setup ipc communication
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.bind("tcp://*:5556");

  zmq::socket_t command_receiver(ctx, zmq::socket_type::sub);
  command_receiver.bind("tcp://*:5557");
  command_receiver.set(zmq::sockopt::subscribe, "");

  zmq::message_t received;

  options_struct options;
  if (options.loadFromFile(publisher, "./lasershow.cfg"))
  {
    std::cout << "options couldnt be loaded from file" << std::endl;
    publish_message(publisher, "ALERT: options load fail");
  }

  publish_message(publisher, "INFO: lasershow ready");

  IldaReader ildaReader;

  if (gpioInitialise() < 0)
  {
    // pigpio initialisation failed.
    std::cout << "init fail" << std::endl;
    return 1;
  }

  gpioSetMode(SCLK, PI_OUTPUT);
  gpioSetMode(CS, PI_OUTPUT);
  gpioSetMode(MISO, PI_INPUT);

  //needed to set the pins
  std::cout << "start bat_voltage:" << static_cast<int>(bat_raw()) << std::endl;

  bool pass_next_command_read = 0;
  while (true)
  {
    Command command;
    if (!pass_next_command_read)
    {
      // options.project_filename = "";
      command_receiver.recv(received, zmq::recv_flags::none); // blocking
      int exec_val = command.execute(received.to_string(), publisher, options, ildaReader);
      if (exec_val == 1) {
        for (size_t i = 0; i < 3; i++)
        {
          gpioWrite(LASER_PINS[i], 0);
        }
      }
      if (exec_val != 2) 
      {
        continue;
      }
      //else PROJECT
    }
    else
    {
      pass_next_command_read = 0;
    }

    if (lasershow_init(publisher, options, ildaReader) != 0)
    {
      std::cout << "failed to init lasershow" << std::endl;
      publish_message(publisher, "ERROR: failed to init lasershow");
      continue;
    }

    bool first_repeat_or_break = 1;
    while (options.repeat || first_repeat_or_break)
    {
      first_repeat_or_break = 0;
      lasershow_start(publisher, ildaReader, options);

      while (first_repeat_or_break == 0) // also used just as a break flag (if 1 break)
      {
        // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
        // - no,, just exit the loop lol
        command_receiver.recv(received, zmq::recv_flags::dontwait);
        while (received.size() > 0)
        {
          int exec_val = command.execute(received.to_string(), publisher, options, ildaReader);
          std::cout << "exec_val: " << exec_val << std::endl;
          if (exec_val == 1) // stop
          {
            first_repeat_or_break = 1; // to get out of loop
            for (size_t i = 0; i < 3; i++)
            {
              gpioWrite(LASER_PINS[i], 0);
            }
            break;
          }
          else if (exec_val == 2) // projecting again
          {
            pass_next_command_read = 1; // load a new file and start projecting again
            if (lasershow_init(publisher, options, ildaReader) != 0)
              {
                std::cout << "failed to init lasershow" << std::endl;
                publish_message(publisher, "ERROR: failed to init lasershow");
                first_repeat_or_break = 1; // to get out of loop
                break;
              }
            first_repeat_or_break = 1;
            break;
          }
          else if (exec_val == 3)
          {
            calculate_points(publisher, options, ildaReader);
          }
          command_receiver.recv(received, zmq::recv_flags::dontwait);
        }
        if (first_repeat_or_break == 0)
        {
          // draw, if an error or end of file is reached, break
          int loop_val = lasershow_loop(publisher, options, ildaReader);
          if (loop_val == 2)
            break;
          else if (loop_val == 1)
          {
            options.repeat = 0;
            break;
          }
        }
      }

      if (options.paused == 1) // stopped
        break;
    }
    // lasershow_cleanup(0);
    // publish_message(publisher, "INFO: lasershow cleanup done");
  }
}