#include <map>
#include <string>
#include <iostream>
#include "my_helper.hpp"
#include "command.hpp"

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
//return: 0 - continue projection / continue doing nothing, 1 - STOP projecting and wait for another command, 2 - restart PROJECTing with a new file, 3 - recalculate points
int Command::execute(std::string string, zmq::socket_t &publisher, options_struct &options)
{
  int to_return = 0;

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
      return 0;
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
          publish_message(publisher, "INFO: OPTION point_delay " + std::to_string(options.pointDelay));
        }
        else if (this->args[1] == "repeat")
        {
          publish_message(publisher, "INFO: OPTION repeat " + std::to_string(options.repeat));
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
            publish_message(publisher, "INFO: OPTION point_delay " + std::to_string(options.pointDelay));
          }
          else if (this->args[1] == "repeat")
          {
            options.repeat = stoi(this->args[2]);
            publish_message(publisher, "INFO: OPTION repeat " + std::to_string(options.repeat));
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
            to_return = 3;
          }
          else if (this->args[1] == "trapezoid_vertical")
          {
            options.trapezoid_vertical = stof(this->args[2]);
            publish_message(publisher, "INFO: OPTION trapezoid_vertical " + std::to_string(options.trapezoid_vertical));
            to_return = 3;
          }
          else if (this->args[1] == "scale_up")
          {
            options.scale_up = stoi(this->args[2]);
            publish_message(publisher, "INFO: OPTION scale_up " + std::to_string(options.scale_up));
            to_return = 3;
          }
          else if (this->args[1] == "scale_up_proportionally")
          {
            options.scale_up_proportionally = stoi(this->args[2]);
            publish_message(publisher, "INFO: OPTION scale_up_proportionally " + std::to_string(options.scale_up_proportionally));
            to_return = 3;
          }
          else
          {
            std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
            publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
            return -1;
          }
        }
        else
        {
          std::cout << "invalid args: \"" << received_string << "\"" << std::endl;
          publish_message(publisher, "ERROR: EINVAL \"" + received_string + "\"");
          return -1;
        }
      }
      options.saveToFile("./lasershow.cfg");
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

  return to_return;
}