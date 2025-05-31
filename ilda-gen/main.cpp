#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>

#include "ildaWriter.h"

class Distance
{
public:
  int32_t x;
  int32_t y;
  int32_t z;

  template <typename T>
  Distance(T from, T to);
};

template <typename T>
Distance::Distance(T from, T to)
{
  this->x = to.x - from.x;
  this->y = to.y - from.y;
  this->z = to.z - from.z;
}

/// @brief
/// @param from
/// @param to
/// @param progress 0-1; the amount of the second color mixed in ie. (0.7 -- 0.3 * from + 0.7 * to
/// @return
RGBColor blendRGBcolors(RGBColor from, RGBColor to, float progress)
{
  return {
      from.r * (1 - progress) + to.r * progress,
      from.g * (1 - progress) + to.g * progress,
      from.b * (1 - progress) + to.b * progress,
  };
};

/// @brief
/// @param from
/// @param to
/// @param progress 0-1; the amount of the second color mixed in ie. (0.7 -- 0.3 * from + 0.7 * to
/// @return
RGBColor blendHSVcolors(HSVColor from, HSVColor to, float progress)
{
  return hsv2rgb(
      from.h * (1 - progress) + to.h * progress,
      from.s * (1 - progress) + to.s * progress,
      from.v * (1 - progress) + to.v * progress);
};

enum line_point_positioning_enum
{
  LINEAR,
  QUADRATIC,
  // EXPONENTIAL,
  SLOW_END // add just a few points near the end of the line to slow the galvos down
};

int16_t blend_coordinate(int16_t from, int32_t distance, uint8_t point_num, uint8_t total_points, line_point_positioning_enum point_positioning)
{
  switch (point_positioning)
  {
  case LINEAR:
    // evenly spaced points
    std::cout << "dis:" << (float)(point_num * distance / (float)total_points) << std::endl;
    return from + (point_num * distance / (float)total_points);
  default:
    return from; // fallback, should not happen
  }
}

template <typename T>
std::vector<Point>
genLine(T from, T to, uint8_t total_points, line_point_positioning_enum point_positioning)
{
  if constexpr (!std::is_same<T, Point>::value && !std::is_same<T, HSVPoint>::value)
  {
    std::cerr << "genLine: T must be Point or HSVPoint" << std::endl;
    exit(1);
  }

  std::vector<Point> line;
  if constexpr (std::is_same<T, Point>::value == true)
    line.push_back(from);
  else
    line.push_back(Point(
        from.x,
        from.y,
        from.z,
        hsv2rgb(from.color),
        from.laser_on));

  // std::cout << "from: [" << from.x << ", " << from.y << ", " << from.z << "], rgb:" << (int)from.color.r << "," << (int)from.color.g << "," << (int)from.color.b << "," << "laseron:" << from.laser_on << std::endl;

  Distance distance(from, to);
  std::cout << "distance: " << distance.x << ", " << distance.y << ", " << distance.z << std::endl;

  for (uint8_t i = 1; i < total_points; i++)
  {
    std::cout << "blend:" << blend_coordinate(from.x, distance.x, i, total_points, point_positioning) << std::endl;

    RGBColor blended_color;
    if constexpr (std::is_same<T, Point>::value == true)
    {
      blended_color = blendRGBcolors(from.color, to.color, (float)i / total_points);
    }
    else
    {
      blended_color = blendHSVcolors(from.color, to.color, (float)i / total_points);
    }

    line.push_back({blend_coordinate(from.x, distance.x, i, total_points, point_positioning),
                    blend_coordinate(from.y, distance.y, i, total_points, point_positioning),
                    blend_coordinate(from.z, distance.z, i, total_points, point_positioning),
                    blended_color,
                    true});
  }

  if constexpr (std::is_same<T, Point>::value == true)
    line.push_back(to);
  else
    line.push_back(Point(
        to.x,
        to.y,
        to.z,
        hsv2rgb(to.color),
        to.laser_on));

  // std::cout << "to: [" << to.x << ", " << to.y << ", " << to.z << "], rgb:" << (int)to.color.r << "," << (int)to.color.g << "," << (int)to.color.b << "," << "laseron:" << to.laser_on << std::endl
  //           << std::endl;
  return line;
}

int main()
{
  std::vector<section> sections;

  constexpr int16_t frames = 150;

  for (uint8_t i = 0; i < frames; i++)
  {
    float progress = (float)i / frames;

    std::cout << "f:" << (int)i << ", p:" << progress << ", MIN:" << ILDA_MIN * progress
              << ", MAX:" << ILDA_MAX * progress << std::endl;

    section sec;
    sec.frame_number = endian_switch(i);

    std::vector<Point> line = genLine(HSVPoint(ILDA_MIN * progress,
                                               ILDA_MIN * progress,
                                               0,
                                               {(0 + 4 * i) % 360, 100, 100},
                                               true),
                                      HSVPoint(ILDA_MAX * progress,
                                               ILDA_MAX * progress,
                                               0,
                                               {(175 + 4 * i) % 360, 100, 100},
                                               true),
                                      20,
                                      LINEAR);
    sec.points.insert(sec.points.end(), line.begin(), line.end());

    line = genLine(Point(ILDA_MAX * progress,
                         ILDA_MAX * progress,
                         0,
                         {0, 0, 0},
                         false),
                   Point(ILDA_MIN * progress,
                         ILDA_MIN * progress,
                         0,
                         {0, 0, 0},
                         false),
                   5,
                   LINEAR);
    sec.points.insert(sec.points.end(), line.begin(), line.end());

    sec.points.back().last_point = 1;
    sec.number_of_records = endian_switch(sec.points.size());

    sections.push_back(sec);
  }

  uint16_t total_frames = endian_switch(sections.size());

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

      point.x = endian_switch(point.x);
      memcpy(record + FormatData::real_2d::X_COORDINATE_BYTES.first,
             &point.x, sizeof(point.x));

      point.y = endian_switch(point.y);
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