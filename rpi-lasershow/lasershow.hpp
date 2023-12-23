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
#include "Points.h"
#include "IldaReader.h"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

void lasershow_cleanup(int);

int lasershow_init(zmq::socket_t &publisher, string fileName);

int lasershow_loop(zmq::socket_t &publisher, bool paused, int pointDelay = 0, double frameDuration = 0.033);
