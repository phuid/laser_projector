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
  GAME,
  PRESS,
  RELEASE,
  OPTION,
  INVALID_CMD
};

class Command
{
public:
  command_type type;
  std::vector<std::string> args;
  Command(std::string string);
};

Command::Command(std::string string)
{
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

  bool repeat = 0;

  while (true)
  {
    bool current_repeat = repeat;
    std::string current_filename = "";

    command_receiver.recv(received, zmq::recv_flags::none);
    std::string received_string = received.to_string();
    Command command(received_string);
    if (command.type == PROJECT && command.args.size() > 0)
    {
      current_filename = command.args[0];
    }
    else if (command.type == INVALID_CMD)
    {
      std::cout << "invalid command: \"" << received_string << "\"" << std::endl;
      msg_to_send.rebuild("ERROR: INVALID_CMD \"" + received_string + "\"");
      publisher.send(msg_to_send, zmq::send_flags::none);
      continue;
    }
    else {
      std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
      msg_to_send.rebuild("ERROR: INVALID_ARGS \"" + received_string + "\"");
      publisher.send(msg_to_send, zmq::send_flags::none);
      continue;
    }

    while (current_repeat)
    {
      int init_val = lasershow_init(current_filename);
      if (!init_val)
      {
        while (!lasershow_loop())
        {
          // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
          // - no,, just exit the loop lol
          // command_receiver.recv(received, zmq::recv_flags::dontwait);
          // if (received.to_string().length() > 0) // or some other message_t function
        }
      }
      else
      {
        std::cout << "failed to init lasershow" << std::endl;
        current_repeat = 0;
      }
      lasershow_cleanup(0);
    }
  }
}