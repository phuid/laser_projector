#include "IldaReader.h"
#include "Frame.h"
#include "FormatData.h"
#include "Points.h"

#include <fstream>
#include <string>

IldaReader::IldaReader() {}

bool IldaReader::readFile(std::string fileName)
{
    // Open file.
    this->file = std::ifstream(fileName, std::ifstream::binary);
    if (!this->file)
    {
        return false;
    }

    // get size of file:
    this->file.seekg(0, this->file.end);
    size_t this->file_size = this->file.tellg();
    this->file.seekg(0);

    // Read first 32 header bytes from the file and check if the first 4 bytes in ASCII are "ILDA".
    return read_sections();
}

// Read first 32 header bytes from the file and check if the first 4 bytes in ASCII are "ILDA".
bool section::read_header(uint8_t header_buf[FormatData::header::NUMBER_OF_HEADER_BYTES])
{
    for (int i = FormatData::header::ILDA_STRING_BYTES.first; i < FormatData::header::ILDA_STRING_BYTES.second; i++)
    {
        if (buf[i] != FormatData::header::ILDA_STRING[i])
            return 1; // Invalid file format.
    }

    // read section format code
    if (buf[FormatData::header::FORMAT_CODE_BYTE] == 3)
    {
        return 1; // Invalid file format.
    }
    else
    {
        this->format = buf[FormatData::header::FORMAT_CODE_BYTE];
    }

    // read number of records
    this->number_of_records = combine_bytes(buf[FormatData::header::NUMBER_OF_RECORDS_BYTES.first], buf[FormatData::header::NUMBER_OF_RECORDS_BYTES.second]);

    // read frame number
    this->frame_number = combine_bytes(buf[FormatData::header::FRAME_NUMBER_BYTES.first], buf[FormatData::header::FRAME_NUMBER_BYTES.second]);

    // read projector number
    this->projector_number = buf[FormatData::header::PROJECTOR_NUMBER_BYTE];

    return 0;
}

bool IldaReader::read_sections(zmq::socket_t &publisher, std::ifstream &file)
{
    for (size_t i = 0; i < this->total_frames; i++)
    {
        section section;

        if (this->file.tellg() + FormatData::NUMBER_OF_HEADER_BYTES > file_size)
        {
            return 0; // EOF
        }
        uint8_t header_buf[FormatData::header::NUMBER_OF_HEADER_BYTES];
        file.read(header_buf, FormatData::NUMBER_OF_HEADER_BYTES);

        // read_header, quit if invalid file format is detected
        if (section.read_header(header_buf))
            return 1;
        if (section.format == ILDA_COLOR_PALETTE)
        {
            this->color_palette.colors.clear();
        }

        // read records
        for (size_t i = 0; i < section.number_of_records; i++)
        {
            point point;

            if (this->file.tellg() + FormatData::NUMBER_OF_RECORD_BYTES[this->format] > file_size)
            {
                return 0; // EOF
            }
            uint8_t buf[FormatData::NUMBER_OF_RECORD_BYTES[section.format]];

            switch (section.format)
            {
            case ILDA_2D_INDEXED:
                point.x = combine_bytes(buf[FormatData::indexed_2d::X_COORDINATE_BYTES.first], buf[FormatData::indexed_2d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::indexed_2d::Y_COORDINATE_BYTES.first], buf[FormatData::indexed_2d::Y_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::indexed_2d::STATUS_BYTE];
                point.color = this->palette[buf[FormatData::indexed_2d::COLOR_INDEX_BYTE]];
                break;
            case ILDA_3D_INDEXED:
                point.x = combine_bytes(buf[FormatData::indexed_3d::X_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::indexed_3d::Y_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::Y_COORDINATE_BYTES.second]);
                point.z = combine_bytes(buf[FormatData::indexed_3d::Z_COORDINATE_BYTES.first], buf[FormatData::indexed_3d::Z_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::indexed_3d::STATUS_BYTE];
                point.color = buf[FormatData::indexed_3d::COLOR_INDEX_BYTE];
                break;
            case ILDA_COLOR_PALETTE:
                this->palette.colors.push_back(color(buf[FormatData::palette::RED_BYTE], buf[FormatData::palette::GREEN_BYTE], buf[FormatData::palette::BLUE_BYTE]));
                break;
            case ILDA_2D_REAL:
                point.x = combine_bytes(buf[FormatData::real_2d::X_COORDINATE_BYTES.first], buf[FormatData::real_2d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::real_2d::Y_COORDINATE_BYTES.first], buf[FormatData::real_2d::Y_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::real_2d::STATUS_BYTE];
                point.red = buf[FormatData::real_2d::RED_BYTE];
                point.green = buf[FormatData::real_2d::GREEN_BYTE];
                point.blue = buf[FormatData::real_2d::BLUE_BYTE];
                break;
            case ILDA_3D_REAL:
                point.x = combine_bytes(buf[FormatData::real_3d::X_COORDINATE_BYTES.first], buf[FormatData::real_3d::X_COORDINATE_BYTES.second]);
                point.y = combine_bytes(buf[FormatData::real_3d::Y_COORDINATE_BYTES.first], buf[FormatData::real_3d::Y_COORDINATE_BYTES.second]);
                point.z = combine_bytes(buf[FormatData::real_3d::Z_COORDINATE_BYTES.first], buf[FormatData::real_3d::Z_COORDINATE_BYTES.second]);
                point.status = buf[FormatData::real_3d::STATUS_BYTE];
                point.red = buf[FormatData::real_3d::RED_BYTE];
                point.green = buf[FormatData::real_3d::GREEN_BYTE];
                point.blue = buf[FormatData::real_3d::BLUE_BYTE];
                break;
            default:
                return 1; // Invalid file format.
            }

            if (section.format != ILDA_COLOR_PALETTE)
            {
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
