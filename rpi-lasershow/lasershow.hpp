#pragma once

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <signal.h>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <chrono>
#include "ABE_ADCDACPi.h"
#include "IldaReader.h"

#include "zmq.hpp"
#include "my_helper.hpp"

void lasershow_cleanup(int);

void lasershow_start(zmq::socket_t &publisher, IldaReader &ildaReader, options_struct &options);

void calculate_points(zmq::socket_t &publisher, options_struct options, IldaReader &ildaReader);

bool lasershow_init(zmq::socket_t &publisher, options_struct &options, IldaReader &ildaReader);

int lasershow_loop(zmq::socket_t &publisher, options_struct options, IldaReader &ildaReader);
