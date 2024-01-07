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

class Command
{
public:
  std::string received_string;
  command_type type;
  std::vector<std::string> args;

  void parse(const std::map<command_type, std::string> &command_dict, std::string string);
  int execute(std::string string, zmq::socket_t &publisher, options_struct &options);
};

void Command::parse(const std::map<command_type, std::string> &command_dict, std::string string)
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

  this->type = find_key(command_dict, first_word);

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

  std::cout << "string: " << this->received_string << std::endl;
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
  std::map<command_type, std::string> command_dict{
      {PROJECT, "PROJECT"},
      {STOP, "STOP"},
      {PAUSE, "PAUSE"},
      {GAME, "GAME"},
      {PRESS, "PRESS"},
      {RELEASE, "RELEASE"},
      {OPTION, "OPTION"},
  };
  this->parse(command_dict, string);

  zmq::message_t msg;
  switch (this->type)
  {
  case PROJECT:
    if (this->args.size() > 0)
    {
      options.project_filename = this->args[0];
      options.paused = 0;
      publish_message(publisher, "INFO: PROJECT " + options.project_filename);
      return 2;
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
    publish_message(publisher, "INFO: PAUSE " + (options.paused ? std::string("1") : std::string("0")));
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
          publish_message(publisher, "INFO: OPTION point_delay " + options.pointDelay);
        }
        else if (this->args[1] == "repeat")
        {
          options.repeat = tmp_options.repeat;
          publish_message(publisher, "INFO: OPTION repeat " + options.repeat);
        }
        else if (this->args[1] == "target_frame_time")
        {
          options.targetFrameTime = tmp_options.targetFrameTime;
          publish_message(publisher, "INFO: OPTION target_frame_time " + std::to_string(options.targetFrameTime));
        }
        else if (this->args[1] == "trapezoid_horizontal")
        {
          options.trapezoid_horizontal = tmp_options.trapezoid_horizontal;
          publish_message(publisher, "INFO: OPTION trapezoid_horizontal " + std::to_string(options.trapezoid_horizontal));
        }
        else if (this->args[1] == "trapezoid_vertical")
        {
          options.trapezoid_vertical = tmp_options.trapezoid_vertical;
          publish_message(publisher, "INFO: OPTION trapezoid_vertical " + std::to_string(options.trapezoid_vertical));
        }
        else
        {
          std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
          publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
          return -1;
        }
      }
      else if (this->args[0] == "read")
      {
        if (this->args[1] == "point_delay")
        {
          publish_message(publisher, "INFO: OPTION point_delay " + options.pointDelay);
        }
        else if (this->args[1] == "repeat")
        {
          publish_message(publisher, "INFO: OPTION repeat " + options.repeat);
        }
        else if (this->args[1] == "target_frame_time")
        {
          publish_message(publisher, "INFO: OPTION target_frame_time " + std::to_string(options.targetFrameTime));
        }
        else if (this->args[1] == "trapezoid_horizontal")
        {
          publish_message(publisher, "INFO: OPTION trapezoid_horizontal " + std::to_string(options.trapezoid_horizontal));
        }
        else if (this->args[1] == "trapezoid_vertical")
        {
          publish_message(publisher, "INFO: OPTION trapezoid_vertical " + std::to_string(options.trapezoid_vertical));
        }
        else
        {
          std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
          publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
          return -1;
        }
      }
      else if (this->args[0] == "write")
      {
        if (this->args.size() >= 3)
        {
          if (this->args[1] == "point_delay")
          {
            options.pointDelay = stoi(this->args[2]);
            publish_message(publisher, "INFO: OPTION point_delay " + options.pointDelay);
          }
          else if (this->args[1] == "repeat")
          {
            options.repeat = stoi(this->args[2]);
            publish_message(publisher, "INFO: OPTION repeat " + options.repeat);
          }
          else if (this->args[1] == "target_frame_time")
          {
            options.targetFrameTime = stoul(this->args[2]);
            publish_message(publisher, "INFO: OPTION target_frame_time " + std::to_string(options.targetFrameTime));
          }
          else if (this->args[1] == "trapezoid_horizontal")
          {
            options.trapezoid_horizontal = stof(this->args[2]);
            publish_message(publisher, "INFO: OPTION trapezoid_horizontal " + std::to_string(options.trapezoid_horizontal));
          }
          else if (this->args[1] == "trapezoid_vertical")
          {
            options.trapezoid_vertical = stof(this->args[2]);
            publish_message(publisher, "INFO: OPTION trapezoid_vertical " + std::to_string(options.trapezoid_vertical));
          }
          else
          {
            std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
            publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
            return -1;
          }
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