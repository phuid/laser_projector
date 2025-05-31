#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>

#include "ildaWriter.h"
#include "font/asteroids_font.h"

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
  if (from.h > to.h)
  {
    from.h += 360; // ensure that the hue is always increasing
  }
  return hsv2rgb(
      (int)(from.h * (1 - progress) + to.h * progress) % 360,
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

std::vector<Point> genLetter(char c, Point top_left, Point bottom_right, RGBColor color)
{
  // The source code is in github.com/vst/teensyv/asteroid_font.c and has been compressed to use very little memory in representing the font in memory.
  // The drawing routine extracts the 4-bit X and Y offsets from the array and generates the moveto() or lineto() commands:

  uint8_t c_pos = 255; // default to not found

  for (uint8_t i = 0; i < FONT_NUM_OF_CHARACTERS; i++)
  {
    if (asteroids_font_chars[i] == c)
    {
      c_pos = i;
      break;
    }
  }
  if (c_pos == 255)
  {
    std::cerr << "Character '" << c << "' not found in asteroids font." << std::endl;
    exit(1);
  }

  const uint8_t *const pts = asteroids_font[c_pos].points;
  int next_moveto = 1;

  std::vector<Point> points;
  auto moveto = [&](int16_t x, int16_t y)
  {
    points.push_back(Point(x, y, 0, {0, 0, 0}, false));
  };
  auto lineto = [&](int16_t x, int16_t y)
  {
    points.push_back(Point(x, y, 0, color, true));
  };

  for (int i = 0; i < 8; i++)
  {
    uint8_t delta = pts[i];
    if (delta == FONT_LAST)
      break;
    if (delta == FONT_UP)
    {
      next_moveto = 1;
      continue;
    }

    unsigned dx = ((delta >> 4) & 0xF) * (bottom_right.x - top_left.x) / 8;
    unsigned dy = ((delta >> 0) & 0xF) * (bottom_right.y - top_left.y) / 12;

    if (next_moveto)
      moveto(top_left.x + dx, top_left.y + dy);
    else
      lineto(top_left.x + dx, top_left.y + dy);

    next_moveto = 0;
  }

  return points;
}

int main()
{
  std::vector<section> sections;

  constexpr uint16_t frames = 600;

  for (uint16_t i = 0; i < frames; i++)
  {
    section sec;
    sec.frame_number = endian_switch(i);

    if (i < 15)
    {
      float progress = (float)i / 15;

      // growing line at the beggining
      std::vector<Point> line = genLine(HSVPoint(ILDA_MIN * progress,
                                                 ILDA_MIN * progress,
                                                 0,
                                                 {(0 + 4 * i) % 360, 100, 100},
                                                 true),
                                        HSVPoint(ILDA_MAX * progress,
                                                 ILDA_MAX * progress,
                                                 0,
                                                 {(180 + 4 * i) % 360, 100, 100},
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
    }
    else if (i < 150)
    {
      // line cycling colors for the rest of the frames
      std::vector<Point> line = genLine(HSVPoint(ILDA_MIN,
                                                 ILDA_MIN,
                                                 0,
                                                 {(0 + i) % 360, 100, 100},
                                                 true),
                                        HSVPoint(ILDA_MAX,
                                                 ILDA_MAX,
                                                 0,
                                                 {(180 + i) % 360, 100, 100},
                                                 true),
                                        20,
                                        LINEAR);
      sec.points.insert(sec.points.end(), line.begin(), line.end());

      line = genLine(Point(ILDA_MAX,
                           ILDA_MAX,
                           0,
                           {0, 0, 0},
                           false),
                     Point(ILDA_MIN,
                           ILDA_MIN,
                           0,
                           {0, 0, 0},
                           false),
                     5,
                     LINEAR);
      sec.points.insert(sec.points.end(), line.begin(), line.end());
    }
    else if (i < 200)
    {
      // single blank point = delay
      sec.points.push_back(Point(0, 0, 0, {0, 0, 0}, false));
    }
    else
    {
      // letters
      sec.points = genLetter('A',
                             Point(ILDA_MIN, ILDA_MIN, 0, {0, 0, 0}, false),
                             Point(ILDA_MAX, ILDA_MAX, 0, {0, 0, 0}, false),
                             hsv2rgb((i * 10) % 360, 100, 100));
    }

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