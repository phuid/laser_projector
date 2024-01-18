#include "IldaReader.h"
#include "FormatData.h"

#include <fstream>
#include <string>

#include <iostream> //debug

int16_t combine_bytes(char first, char second)
{
    return (first << 8) | second;
}

// Helper function to map a value between two value ranges.
int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return ((x - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
}

// Helpter function to get arbitrary bit value from byte.
bool getBit(char b, int bitNumber) {
    return (b & (1 << (bitNumber - 1))) != 0;
}

IldaReader::IldaReader() {}

// return: 0-success, 1-error
bool IldaReader::readFile(zmq::socket_t &publisher, std::string fileName)
{
    // Open file.
    this->file = std::ifstream(fileName, std::ifstream::binary);
    if (!this->file)
    {
        return 1;
    }

    // get size of file:
    this->file.seekg(0, this->file.end);
    this->file_size = this->file.tellg();
    this->file.seekg(0);

    bool return_val = read_sections(publisher);
    this->closeFile();
    return return_val;
}

// Read first 32 header bytes from the file and check if the first 4 bytes in ASCII are "ILDA".
// return: 0-success, 1-error
bool section::read_header(char buf[FormatData::NUMBER_OF_HEADER_BYTES])
{
    for (int i = FormatData::header::ILDA_STRING_BYTES.first; i < FormatData::header::ILDA_STRING_BYTES.second; i++)
    {
        if (buf[i] != FormatData::header::ILDA_STRING[i])
        {
            std::cout << "invalid ildastring" << i << (int)buf[i] << "," << (int)FormatData::header::ILDA_STRING[i];
            return 1; // Invalid file format.
        }
    }

    // read section format code
    if (buf[FormatData::header::FORMAT_CODE_BYTE] == 3)
    {
        return 1; // Invalid file format.
    }
    else
    {
        this->format = static_cast<section_format>(buf[FormatData::header::FORMAT_CODE_BYTE]);
    }

    // read number of records
    this->number_of_records = combine_bytes(buf[FormatData::header::NUMBER_OF_RECORDS_BYTES.first], buf[FormatData::header::NUMBER_OF_RECORDS_BYTES.second]);

    // read frame number
    this->frame_number = combine_bytes(buf[FormatData::header::FRAME_NUMBER_BYTES.first], buf[FormatData::header::FRAME_NUMBER_BYTES.second]);

    // read projector number
    this->projector_number = buf[FormatData::header::PROJECTOR_NUMBER_BYTE];

    return 0;
}

// return: 0-success, 1-error
bool IldaReader::read_sections(zmq::socket_t &publisher)
{
    this->sections.clear();
    this->current_frame_index = 0;
    while (true) // only broken when reading header or by EOF
    {
        section section;

        if (static_cast<size_t>(this->file.tellg()) + FormatData::NUMBER_OF_HEADER_BYTES > file_size)
        {
            std::cout << "WARNING: unexpected EOF - stopped file parsing, but continuing to execution" << std::endl;
            return 0; // EOF
        }
        char header_buf[FormatData::NUMBER_OF_HEADER_BYTES];
        this->file.read(header_buf, FormatData::NUMBER_OF_HEADER_BYTES);

        // read_header, quit if invalid file format is detected
        if (section.read_header(header_buf))
        {
            std::cout << "invalid header" << std::endl;
            return 1;
        }
        if (section.number_of_records == 0)
            return 0; // EOF
        if (section.format == ILDA_COLOR_PALETTE)
        {
            this->palette.colors.clear();
        }

        // read records
        for (size_t i = 0; i < section.number_of_records; i++)
        {
            point point;

            if (static_cast<size_t>(this->file.tellg()) + FormatData::NUMBER_OF_RECORD_BYTES[section.format] > file_size)
            {
                std::cout << "WARNING: unexpected EOF - stopped file parsing, but continuing to execution" << std::endl;
                return 0; // EOF
            }

            char buf[FormatData::NUMBER_OF_RECORD_BYTES[section.format]];
            this->file.read(buf, FormatData::NUMBER_OF_RECORD_BYTES[section.format]);

            switch (section.format)
            {
            case ILDA_2D_INDEXED:
                point.x = combine_bytes(buf[FormatData::indexed_2d::X_COORDINATE_BYTES.first], buf[FormatData::indexed_2d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::indexed_2d::Y_COORDINATE_BYTES.first], buf[FormatData::indexed_2d::Y_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::indexed_2d::STATUS_BYTE];
                point.laser_on = !getBit(point.status, FormatData::BLANKING_BIT);
                point.red = this->palette.colors[buf[FormatData::indexed_2d::COLOR_INDEX_BYTE]].r;
                point.green = this->palette.colors[buf[FormatData::indexed_2d::COLOR_INDEX_BYTE]].g;
                point.blue = this->palette.colors[buf[FormatData::indexed_2d::COLOR_INDEX_BYTE]].b;
                break;
            case ILDA_3D_INDEXED:
                point.x = combine_bytes(buf[FormatData::indexed_3d::X_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::indexed_3d::Y_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::Y_COORDINATE_BYTES.second]);
                point.z = combine_bytes(buf[FormatData::indexed_3d::Z_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::Z_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::indexed_3d::STATUS_BYTE];
                point.laser_on = !getBit(point.status, FormatData::BLANKING_BIT);
                point.red = this->palette.colors[buf[FormatData::indexed_3d::COLOR_INDEX_BYTE]].r;
                point.green = this->palette.colors[buf[FormatData::indexed_3d::COLOR_INDEX_BYTE]].g;
                point.blue = this->palette.colors[buf[FormatData::indexed_3d::COLOR_INDEX_BYTE]].b;
                break;
            case ILDA_COLOR_PALETTE:
                this->palette.colors.push_back({buf[FormatData::palette::RED_BYTE], buf[FormatData::palette::GREEN_BYTE], buf[FormatData::palette::BLUE_BYTE]});
                break;
            case ILDA_2D_REAL:
                point.x = combine_bytes(buf[FormatData::real_2d::X_COORDINATE_BYTES.first], buf[FormatData::real_2d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::real_2d::Y_COORDINATE_BYTES.first], buf[FormatData::real_2d::Y_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::real_2d::STATUS_BYTE];
                point.laser_on = !getBit(point.status, FormatData::BLANKING_BIT);
                point.red = buf[FormatData::real_2d::RED_BYTE];
                point.green = buf[FormatData::real_2d::GREEN_BYTE];
                point.blue = buf[FormatData::real_2d::BLUE_BYTE];
                break;
            case ILDA_3D_REAL:
                point.x = combine_bytes(buf[FormatData::real_3d::X_COORDINATE_BYTES.first], buf[FormatData::real_3d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::real_3d::Y_COORDINATE_BYTES.first], buf[FormatData::real_3d::Y_COORDINATE_BYTES.second]);
                point.z = combine_bytes(buf[FormatData::real_3d::Z_COORDINATE_BYTES.first], buf[FormatData::real_3d::Z_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::real_3d::STATUS_BYTE];
                point.laser_on = !getBit(point.status, FormatData::BLANKING_BIT);
                point.red = buf[FormatData::real_3d::RED_BYTE];
                point.green = buf[FormatData::real_3d::GREEN_BYTE];
                point.blue = buf[FormatData::real_3d::BLUE_BYTE];
                break;
            default:
                return 1; // Invalid file format.
            }

            if (section.format != ILDA_COLOR_PALETTE)
            {
                // ILDA format has a weird way of storing values. Positive numbers are stored nomally
                // but negative numbers are stored in second part of 'unsigned short' (>32768) so e.g.
                // the number "-1" is storead as "65535".
                if (point.x > 32768)
                    point.x = point.x - 65536;
                if (point.y > 32768)
                    point.y = point.y - 65536;

                // Map ILDA values to DAC value range and store the data to array.
                point.x = map(point.x, -32768, +32767, 0, 4095);
                point.y = map(point.y, -32768, +32767, 0, 4095);

                section.points.push_back(point);
            }
        }
        if (section.format != ILDA_COLOR_PALETTE)
        {
            this->sections.push_back(section);
        }
    }
    return 0;
}

void IldaReader::closeFile()
{
    this->file.close();
}
