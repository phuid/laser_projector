#pragma once
#include "Points.h"

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

#include <fstream>
using namespace std;

// Format spec: https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf

class Frame {

    public:
        Frame();
        int getNext(zmq::socket_t &publisher, std::ifstream& file, Points* points);

    private:
        bool isLastPoint(char* bytes);
        bool getBit(char b, int bitNumber);
        int map(int x, int in_min, int in_max, int out_min, int out_max);

    public:
        static const int NUMBER_OF_HEADER_BYTES = 32;

};
