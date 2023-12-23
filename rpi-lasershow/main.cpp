#include "lasershow.hpp"
#include "Points.h"
#include "IldaReader.h"
#include <wiringPi.h>
#include "ABE_ADCDACPi.h"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <map>

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

std::map<command_type, std::string> dict{
    {PROJECT, "PROJECT"},
    {STOP, "STOP"},
    {PAUSE, "PAUSE"},
    {GAME, "GAME"},
    {PRESS, "PRESS"},
    {RELEASE, "RELEASE"},
    {OPTION, "OPTION"},
};

command_type find_key(std::map<command_type, std::string> dict, std::string value)
{
  for (auto &i : dict)
  {
    if (i.second == value)
    {
      return i.first;
    }
  }
  return INVALID_CMD;
}

struct options_struct
{
  bool repeat = 1;
  int pointDelay = 0;
  double targetFrameTime = 0.033;
  int trapezoid_horizontal = 0;
  int trapezoid_vertical = 0;

  std::string project_filename;
  bool paused = 0;

  int loadFromFile(std::string filename)
  {
    return 1; // fail
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

  this->type = find_key(dict, first_word);

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

  std::cout << "parsed - type: " << this->type << "; "
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
  switch (this->type)
  {
  case PROJECT:
    if (this->args.size() > 0)
    {
      options.project_filename = this->args[0];
      options.paused = 0;
      publish_message(publisher, "INFO: PROJECT " + options.project_filename);
      return 1;
    }
    else
    {
      std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
      publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
      return -1;
    }
    break;
  case STOP:
    options.paused = 1;
    publish_message(publisher, "INFO: STOP");
    return 1;
    break;
  case PAUSE:
    options.paused = !options.paused;
    publish_message(publisher, "INFO: PAUSE" + options.paused);
    return 0;
    break;
  case OPTION:
    if (this->args.size() >= 2)
    {
      if (this->args[0] == "reset")
      {
        options_struct tmp_options;
        if (this->args[1] == "point_delay")
        {
          options.pointDelay = tmp_options.pointDelay;
        }
        ... publish_message(publisher, "INFO: OPTION point_delay")
      }
      else if (this->args[0] == "read")
      {
        if
      }
      else if (this->args[0] == "write")
      {
        if (this->args.size() >= 3)
        {
        }
      }
    }
    else
    {
      std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
      publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
      return -1;
    }
    break;

  default:
    std::cout << "invalid command: \"" << received_string << "\"" << std::endl;
    publish_message(publisher, "ERROR: INVALID_CMD \"" + received_string + "\"");
    return -1;
    break;
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
  if (options.loadFromFile("lasershow.cfg"))
  {
    std::cout << "options couldnt be loaded from file" << std::endl;
  }

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
      int init_val = lasershow_init(publisher, options.project_filename);
      if (!init_val)
      {
        while (true)
        {
          // maybe receive messages here, then youd need atleast the ildareader(filename) here in main..
          // - no,, just exit the loop lol
          command_receiver.recv(received, zmq::recv_flags::dontwait);
          if (received.size() > 0)
          {
            int exec_val = command.execute(received.to_string(), publisher, options);
            if (exec_val == 1)
            {
              break;
            }
          }
          // draw, if an error or end of file is reached, break
          int loop_val = lasershow_loop(publisher, options.paused, options.pointDelay, options.targetFrameTime);
          if (loop_val == 2)
            break;
          else if (loop_val == 1)
          {
            options.repeat = 0;
            break;
          }
        }
      }
      else
      {
        std::cout << "failed to init lasershow" << std::endl;
        publish_message(publisher, "ERR: failed to init lasershow");
        break;
      }

      lasershow_cleanup(0);
      publish_message(publisher, "INFO: lasershow cleanup");
      if (options.paused == 1) // stopped
        break;
    }
  }
}