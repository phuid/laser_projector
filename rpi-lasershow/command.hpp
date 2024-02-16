#pragma once
#include <map>
#include <unistd.h>
#include "my_helper.hpp"
#include "IldaReader.h"

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

command_type find_key(std::map<command_type, std::string> dict, std::string value);

class Command
{
public:
  std::string received_string;
  command_type type;
  std::vector<std::string> args;

  void parse(const std::map<command_type, std::string> &command_dict, std::string string);
  int execute(std::string string, zmq::socket_t &publisher, options_struct &options, IldaReader &ildaReader);
};
