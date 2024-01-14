#pragma once

#include "zmq.hpp"
#include "colorPalette.hpp"
#include "FormatData.h"

#include <string>
#include <fstream>

enum section_format
{
    ILDA_2D_INDEXED = 0,
    ILDA_3D_INDEXED = 1,
    ILDA_COLOR_PALETTE = 2,
    ILDA_2D_REAL = 4,
    ILDA_3D_REAL = 5,
};

struct point {
    int16_t x;
    int16_t y;
    int16_t z;

    uint8_t red;
    uint8_t green;
    uint8_t blue;

    uint8_t status;
    bool laser_on;
};

struct section {
    section_format format;
    uint16_t number_of_records;
    uint16_t frame_number;
    uint8_t projector_number;

    std::vector<point> points;

    bool read_header(char buf[FormatData::NUMBER_OF_HEADER_BYTES]);
};

int16_t combine_bytes(char first, char second);
int map(int x, int in_min, int in_max, int out_min, int out_max);

class IldaReader
{
public:
    IldaReader();
    bool read_sections(zmq::socket_t &publisher);
    bool readFile(zmq::socket_t &publisher, std::string fileName);
    void closeFile();

    std::ifstream file;
    size_t file_size;
    std::vector<section> sections;
    uint16_t current_frame_index = 0;
    color_palette palette;
};
