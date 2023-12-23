#pragma once

#include "Points.h"

#include "zmq.hpp"

#include <string>
#include <fstream>
using namespace std;

class IldaReader {
    public:
        IldaReader();
        bool readFile(string fileName);
        bool checkHeader();
        int getNextFrame(zmq::socket_t &publisher, Points* points);
        void closeFile();

    public:
        std::ifstream file;
        // ilda_type type;
};
