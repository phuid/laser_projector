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
#include "mcp4822.h"
#include "Points.h"
#include "IldaReader.h"

void lasershow_cleanup(int);

int lasershow_init(string fileName, Points &points, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start);

bool lasershow_loop(Points &points, IldaReader &ildaReader, std::chrono::time_point<std::chrono::system_clock> &start, int pointDelay = 0, double frameDuration = (1/30));
