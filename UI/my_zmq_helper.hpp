#pragma once
#include "zmq.hpp"
#include <string>

void send_command(zmq::socket_t &publisher, std::string message_string);