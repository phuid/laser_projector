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
#include <wiringPi.h>
#include "ABE_ADCDACPi.h"
#include "IldaReader.h"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

void lasershow_cleanup(int);

lasershow_start(zmq::socket_t &publisher, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start);

bool lasershow_init(zmq::socket_t &publisher, ABElectronics_CPP_Libraries::ADCDACPi &adcdac, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start, std::string fileName);

int lasershow_loop(zmq::socket_t &publisher, ABElectronics_CPP_Libraries::ADCDACPi &adcdac, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start, options_struct options);
