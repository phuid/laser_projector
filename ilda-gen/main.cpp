#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>

#include "ildaWriter.h"

int main()
{
  std::vector<section> sections;

  for (uint8_t i = 0; i < 60; i++)
  {
    float progress = (float)i / 60.0f;

    std::cout << "f:" << (int)i << ", p:" << progress << ", MIN:" << little_endian_to_big_endian(ILDA_MIN * progress)
 << ", MAX:" << little_endian_to_big_endian(ILDA_MAX * progress) << std::endl;

    section sec;
    sec.frame_number = little_endian_to_big_endian(i);

    sec.points.push_back({little_endian_to_big_endian(ILDA_MIN * progress),
                          little_endian_to_big_endian(ILDA_MIN * progress),
                          0,
                          hsv2rgb((progress * 360.0f), 100.0f, 100.0f),
                          false,
                          true});
    sec.points.push_back({little_endian_to_big_endian(ILDA_MAX * progress),
                          little_endian_to_big_endian(ILDA_MIN * progress),
                          0,
                          hsv2rgb((progress * 360.0f), 100.0f, 100.0f),
                          false,
                          false});
    sec.points.push_back({little_endian_to_big_endian(ILDA_MAX * progress),
                          little_endian_to_big_endian(ILDA_MAX * progress),
                          0,
                          hsv2rgb((progress * 360.0f), 100.0f, 100.0f),
                          false,
                          true});
    sec.points.push_back({little_endian_to_big_endian(ILDA_MIN * progress),
                          little_endian_to_big_endian(ILDA_MAX * progress),
                          0,
                          hsv2rgb((progress * 360.0f), 100.0f, 100.0f),
                          true,
                          true});

    sec.number_of_records = little_endian_to_big_endian(sec.points.size());

    sections.push_back(sec);
  }

  uint16_t total_frames = little_endian_to_big_endian(sections.size());

  std::ofstream file("../ild/generated.ild", std::ios::binary);
  if (!file.is_open())
  {
    std::cerr << "Failed to open file for writing." << std::endl;
    return 1;
  }

  char header[FormatData::NUMBER_OF_HEADER_BYTES];
  memset(header, 0, FormatData::NUMBER_OF_HEADER_BYTES);

  memcpy(header, FormatData::header::ILDA_STRING, 4);
  header[FormatData::header::FORMAT_CODE_BYTE] = (char)ILDA_2D_REAL;

  for (auto &&section : sections)
  {
    memcpy(header + FormatData::header::NUMBER_OF_RECORDS_BYTES.first,
           &section.number_of_records, sizeof(section.number_of_records));
    memcpy(header + FormatData::header::FRAME_NUMBER_BYTES.first,
           &section.frame_number, sizeof(section.frame_number));
    memcpy(header + FormatData::header::TOTAL_FRAMES_BYTES.first,
           &total_frames, sizeof(total_frames));
    header[FormatData::header::PROJECTOR_NUMBER_BYTE] = section.projector_number;
    file.write(header, FormatData::NUMBER_OF_HEADER_BYTES);
    
    for (auto &&point : section.points)
    {
      char record[FormatData::NUMBER_OF_RECORD_BYTES[ILDA_2D_REAL]];
      memset(record, 0, FormatData::NUMBER_OF_RECORD_BYTES[ILDA_2D_REAL]);

      memcpy(record + FormatData::real_2d::X_COORDINATE_BYTES.first,
             &point.x, sizeof(point.x));
      memcpy(record + FormatData::real_2d::Y_COORDINATE_BYTES.first,
             &point.y, sizeof(point.y));
      record[FormatData::real_2d::STATUS_BYTE] = (point.laser_on ? 0 : FormatData::BLANKING_MASK) | ((point.last_point ? 0b10000000 : 0));
      record[FormatData::real_2d::RED_BYTE] = point.color.r;
      record[FormatData::real_2d::GREEN_BYTE] = point.color.g;
      record[FormatData::real_2d::BLUE_BYTE] = point.color.b;

      file.write(record, FormatData::NUMBER_OF_RECORD_BYTES[ILDA_2D_REAL]);
    }
  }
  memset(header + 4, 0, FormatData::NUMBER_OF_HEADER_BYTES - 4);
  file.write(header, FormatData::NUMBER_OF_HEADER_BYTES);
  
  file.close();
}