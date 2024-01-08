#pragma once

#include "Points.h"

#include "zmq.hpp"

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

struct section {
    section_format format;
    uint16_t number_of_records;
    uint16_t frame_number;

    std::vector<point> points;

    bool read_header();
};

uint16_t combine_bytes(uint8_t first, uint8_t second) {
    return (first << 8) | second;
}

class IldaReader
{
public:
    IldaReader();
    int read_sections(zmq::socket_t &publisher);
    bool readFile(std::string fileName);
    void closeFile();

public:
    std::ifstream file;
    size_t file_size;
    std::vector<section> sections;
    color_palette palette;
};
