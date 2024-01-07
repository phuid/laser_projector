#pragma once
#include <string>
// Format spec: https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf

struct pair
{
    int16_t first;
    int16_t second;
};

class header
{
public:
    constexpr pair ILDA_STRING_BYTES = {0, 3};
    constexpr std::string ILDA_STRING = "ILDA";
    constexpr int16_t FORMAT_CODE_BYTE = 7;
    constexpr pair NUMBER_OF_RECORDS_BYTES = {24, 25};
    constexpr pair FRAME_NUMBER_BYTES = {26, 27};
    constexpr pair TOTAL_FRAMES_BYTES = {28, 29};
    constexpr int16_t PROJECTOR_NUMBER_BYTE = 30;
};
class indexed_3d
{
public:
    constexpr pair X_COORDINATE_BYTES = {0, 1};
    constexpr pair Y_COORDINATE_BYTES = {2, 3};
    constexpr pair Z_COORDINATE_BYTES = {4, 5};
    constexpr int16_t STATUS_BYTE = 6;
    constexpr int16_t COLOR_INDEX_BYTE = 7;
};
class indexed_2d
{
public:
    constexpr pair X_COORDINATE_BYTES = {0, 1};
    constexpr pair Y_COORDINATE_BYTES = {2, 3};
    constexpr int16_t STATUS_BYTE = 4;
    constexpr int16_t COLOR_INDEX_BYTE = 5;
};
class palette
{
public:
    constexpr int16_t RED_BYTE = 0;
    constexpr int16_t GREEN_BYTE = 1;
    constexpr int16_t BLUE_BYTE = 2;
};
class real_3d
{
public:
    constexpr pair X_COORDINATE_BYTES = {0, 1};
    constexpr pair Y_COORDINATE_BYTES = {2, 3};
    constexpr pair Z_COORDINATE_BYTES = {4, 5};
    constexpr int16_t STATUS_BYTE = 6;
    constexpr int16_t RED_BYTE = 7;
    constexpr int16_t GREEN_BYTE = 8;
    constexpr int16_t BLUE_BYTE = 9;
};
class real_2d
{
public:
    constexpr pair X_COORDINATE_BYTES = {0, 1};
    constexpr pair Y_COORDINATE_BYTES = {2, 3};
    constexpr int16_t STATUS_BYTE = 4;
    constexpr int16_t RED_BYTE = 5;
    constexpr int16_t GREEN_BYTE = 6;
    constexpr int16_t BLUE_BYTE = 7;
};

constexpr class FormatData
{
public:
    constexpr uint8_t NUMBER_OF_HEADER_BYTES = 32;
    constexpr header header;

    constexpr uint8_t NUMBER_OF_RECORD_BYTES[5] = {8, 6, 3, 0, 10, 8};
    constexpr indexed_3d indexed_3d;
    constexpr indexed_2d indexed_2d;
    constexpr palette palette;
    constexpr real_3d real_3d;
    constexpr real_2d real_2d;
};

