#include "lasershow.hpp"
#include "Points.h"
#include "IldaReader.h"
#include <wiringPi.h>
#include "ABE_ADCDACPi.h"

#include "zmq.hpp"

#include <string>
#include <iostream>
#include <vector>

enum command_type
{
  PROJECT = 0,
  STOP,
  PAUSE,
  GAME,
  PRESS,
  RELEASE,
  OPTION,
  INVALID_CMD
};

struct options_struct
{
  bool repeat = 0;
  int pointDelay = 0;
  double targetFrameTime = 0.033;

  std::string project_filename;
  bool paused = 0;

  int loadFromFile(std::string filename)
  {
  }
};

class Command
{
public:
  std::string received_string;
  command_type type;
  std::vector<std::string> args;

  void parse(std::string string);
  int execute(std::string string, zmq::socket_t &publisher, options_struct &options);
};

void Command::parse(std::string string)
{
  args.clear();
  this->received_string = string;

  size_t space_pos = string.find(" ");
  std::string first_word;
  if (space_pos == std::string::npos)
    first_word = string;
  else
  {
    first_word = string.substr(0, space_pos);
    string = string.substr(space_pos + 1);
  }

  if (first_word == "PROJECT")
    this->type = PROJECT;
  else if (first_word == "STOP")
    this->type = STOP;
  else if (first_word == "PAUSE")
    this->type = PAUSE;
  else if (first_word == "GAME")
    this->type = GAME;
  else if (first_word == "PRESS")
    this->type = PRESS;
  else if (first_word == "RELEASE")
    this->type = RELEASE;
  else if (first_word == "OPTION")
    this->type = OPTION;
  else
    this->type = INVALID_CMD;

  if (space_pos != std::string::npos)
  {
    space_pos = string.find(" ");
    size_t init_pos = 0;
    while (space_pos != std::string::npos)
    {
      this->args.push_back(string.substr(init_pos, space_pos - init_pos));
      init_pos = space_pos + 1;

      space_pos = string.find(" ", init_pos);
    }
    this->args.push_back(string.substr(init_pos, std::min(space_pos, string.size()) - init_pos + 1));
  }

  std::cout << "parsed" << std::endl
            << "type: " << this->type << std::endl
            << "args: ";
  for (auto &&i : args)
  {
    std::cout << i << ", ";
  }
  std::cout << std::endl;
}

int Command::execute(std::string string, zmq::socket_t &publisher, options_struct &options)
{
  this->parse(string);

  zmq::message_t msg;
  if (this->type == PROJECT && this->args.size() > 0)
  {
    options.project_filename = this->args[0];
  }
  else if (this->type == STOP)
  {
    return 1;
  }
  else if (this->type == PAUSE)
  {
    options.paused = !options.paused;
  }
  else if (this->type == INVALID_CMD)
  {
    std::cout << "invalid command: \"" << received_string << "\"" << std::endl;
    msg.rebuild("ERROR: INVALID_CMD \"" + received_string + "\"");
    publisher.send(msg, zmq::send_flags::none);
    return -1;
  }
  else
  {
    std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
    msg.rebuild("ERROR: INVALID_ARGS \"" + received_string + "\"");
    publisher.send(msg, zmq::send_flags::none);
    return -1;
  }

  return 0;
}

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
  options.loadFromFile("lasershow.cfg");

  while (true)
  {
    command_receiver.recv(received, zmq::recv_flags::none); // blocking
    Command command;
    if (command.execute(received.to_string(), publisher, options))
    {
      continue;
    }

    while (options.repeat)
    {
      int init_val = lasershow_init(options.project_filename);
      if (!init_val)
      {
        while (true)
        {
          // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
          // - no,, just exit the loop lol
          command_receiver.recv(received, zmq::recv_flags::dontwait);
          if (received.size() > 0) 
          {
            if (command.execute(received.to_string(), publisher, options) == 1)
            {
              break;
            }
          }
          if (!options.paused)
          {
            if (lasershow_loop()) // draw, if an error or end of file is reached, break
              break;
          }
        }
      }
      else
      {
        std::cout << "failed to init lasershow" << std::endl;
        options.repeat = 0;
      }
      lasershow_cleanup(0);
    }
  }
}