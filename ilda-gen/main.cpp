#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <math.h>

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
    // std::cout << "dis:" << (float)(point_num * distance / (float)total_points) << std::endl;
    return from + (point_num * distance / (float)total_points);
  default:
    return from; // fallback, should not happen
  }
}

template <typename T>
std::vector<Point>
genLine(T from, T to, uint8_t total_points, line_point_positioning_enum point_positioning, bool omit_first_point = false)
{
  if constexpr (!std::is_same<T, Point>::value && !std::is_same<T, HSVPoint>::value)
  {
    std::cerr << "genLine: T must be Point or HSVPoint" << std::endl;
    exit(1);
  }

  if (!from.laser_on)
  {
    from.color = {0, 0, 0}; // if the laser is off, set the color to black
  }
  if (!to.laser_on)
  {
    to.color = {0, 0, 0}; // if the laser is off, set the color to black
  }

  std::vector<Point> line;
  if (!omit_first_point)
  {
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
  }

  Distance distance(from, to);
  // std::cout << "distance: " << distance.x << ", " << distance.y << ", " << distance.z << std::endl;

  for (uint8_t i = 1; i < total_points; i++)
  {
    // std::cout << "blend:" << blend_coordinate(from.x, distance.x, i, total_points, point_positioning) << std::endl;

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

std::vector<Point> genSineWave(HSVPoint top_left, HSVPoint bottom_right, uint8_t total_points, float x_multi = 2 * 3.14, float phase = 0)
{
  int32_t height = bottom_right.y - top_left.y;
  int32_t width = bottom_right.x - top_left.x;

  std::vector<Point> points;

  for (uint8_t i = 0; i < total_points; i++)
  {
    float progress = (float)i / (total_points - 1);
    int16_t x = top_left.x + progress * width;
    int16_t y = top_left.y + height / 2 + height / 2 * sin(progress * x_multi + phase);

    points.push_back(Point(x,
                           y,
                           top_left.z,
                           hsv2rgb(top_left.color.h, top_left.color.s, top_left.color.v),
                           true));
  }
  return points;
}

std::vector<Point> genRectangle(Point top_left, Point bottom_right, RGBColor color)
{
  std::vector<Point> points;
  std::vector<Point> line;

  line = genLine(Point(top_left.x, top_left.y, 0, color, true),
                 Point(top_left.x, bottom_right.y, 0, color, true),
                 4,
                 LINEAR);
  points.insert(points.end(), line.begin(), line.end());
  line = genLine(Point(top_left.x, bottom_right.y, 0, color, true),
                 Point(bottom_right.x, bottom_right.y, 0, color, true),
                 4,
                 LINEAR);
  points.insert(points.end(), line.begin(), line.end());
  line = genLine(Point(bottom_right.x, bottom_right.y, 0, color, true),
                 Point(bottom_right.x, top_left.y, 0, color, true),
                 4,
                 LINEAR);
  points.insert(points.end(), line.begin(), line.end());
  line = genLine(Point(bottom_right.x, top_left.y, 0, color, true),
                 Point(top_left.x, top_left.y, 0, color, true),
                 4,
                 LINEAR);
  points.insert(points.end(), line.begin(), line.end());

  return points;
}

std::vector<Point> genLetter(char c, Point top_left, Point bottom_right, RGBColor color, Point last_point)
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
  bool last_moveto = false;

  std::vector<Point> points;

  auto moveto = [&](int16_t x, int16_t y)
  {
    // std::cout << "--moveto: " << x << ", " << y << std::endl;
    Point tmp = Point(x, y, 0, color, false);
    last_point.laser_on = false; // moveto does not turn on the laser
    std::vector<Point> line = genLine(last_point,
                                      tmp,
                                      2,
                                      LINEAR,
                                      false);
    points.insert(points.end(), line.begin(), line.end());
    last_point = tmp;
    last_moveto = true;
  };
  auto lineto = [&](int16_t x, int16_t y)
  {
    //   std::cout << "--lineto: " << x << ", " << y << std::endl;
    Point tmp = Point(x, y, 0, color, true);
    last_point.laser_on = true; // lineto turns on the laser
    std::vector<Point> line = genLine(last_point,
                                      tmp,
                                      2,
                                      LINEAR,
                                      !last_moveto);
    points.insert(points.end(), line.begin(), line.end());
    last_point = tmp;
    last_moveto = false;
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

/// @brief
/// @param text
/// @param top_left
/// @param bottom_right
/// @param color
/// @param space_width space width relative to the width of a character
/// @param last_point
/// @param skip_last_chars number of characters to not draw at the end of the text
/// @return
std::vector<Point> genText(std::string text, Point top_left, Point bottom_right, RGBColor color, float space_width, Point last_point, uint8_t skip_last_chars = 0)
{
  std::vector<Point> points;
  std::cout << "skip_last_chars: " << (int)skip_last_chars << std::endl;
  for (size_t i = 0; i < text.size(); i++)
  {
    if (text.size() - i - 1 < skip_last_chars)
      continue; // skip the last characters

    char c = text[i];
    uint16_t text_width = bottom_right.x - top_left.x;
    float total_segments = text.size() + (text.size() - 1) * space_width; // number of segments in the text, including spaces

    Point top_left_c = top_left;
    top_left_c.x += (i + i * space_width) * (text_width / total_segments);

    Point bottom_right_c = bottom_right;
    size_t back_i = text.size() - i - 1; // reverse index for the bottom right corner
    bottom_right_c.x -= (back_i + back_i * space_width) * (text_width / total_segments);

    auto letter = genLetter(c, top_left_c, bottom_right_c, color, last_point);
    points.insert(points.end(), letter.begin(), letter.end());
    if (!letter.empty())
      last_point = letter.back();
  }

  if (points.empty())
  {
    std::cout << "empty text, returning last point" << std::endl;
    points.push_back(last_point); // if no points were generated, return the last point
  }

  return points;
}

#define WRITE_SEC                                           \
  sec.points.back().last_point = 1;                         \
  sec.number_of_records = endian_switch(sec.points.size()); \
                                                            \
  sections.push_back(sec);

#define INIT_SEC \
  section sec;   \
  sec.frame_number = endian_switch(i);

int main()
{
  std::vector<section> sections;

  for (uint16_t i = 150; i < 300; i++)
  {
    std::cout << "Generating section " << i << std::endl;

    INIT_SEC

    auto last_point = [&](bool turn_off_laser = false)
    {
      if (sec.points.empty())
        if (sections.empty() || sections.back().points.empty())
          return Point(0, 0, 0, {0, 0, 0}, false);
        else
          return Point(sections.back().points.back());

      Point p = Point(sec.points.back());
      if (turn_off_laser)
        p.laser_on = false;
      return p;
    };

    auto letters_appear_skip = [&](uint8_t num_letters, uint16_t first_frame, uint16_t last_frame, uint16_t curr_frame)
    {
      if (curr_frame < first_frame)
      {
        return (uint8_t)num_letters;
      }
      if (curr_frame > last_frame)
      {
        return (uint8_t)0;
      }
      if (first_frame > last_frame)
      {
        std::cerr << "first_frame is greater than last_frame" << std::endl;
        exit(1);
      }

      uint16_t num_frames = last_frame - first_frame;

      float back_progress = (float)(last_frame - curr_frame) / num_frames;

      // std::cout << "back_progress: " << back_progress << std::endl;

      return (uint8_t)(back_progress * num_letters);
    };

    constexpr float START_PERIOD = 3.14 * 8; // period of the sine wave in radians

    if (i == 0)
    {
      sec.points = {Point(ILDA_MIN, ILDA_MIN, ILDA_MIN, {0, 0, 0}, false),
                    Point(ILDA_MAX, ILDA_MAX, ILDA_MAX, {0, 0, 0}, false)};
    }
    if (i < 15)
    {
      float progress = (float)i / 15;

      sec.append(genSineWave(HSVPoint(ILDA_MIN * progress,
                                      ILDA_MIN * progress,
                                      0,
                                      {(0 + 4 * i) % 360, 100, 100},
                                      true),
                             HSVPoint(ILDA_MAX * progress,
                                      ILDA_MAX * progress,
                                      0,
                                      {(180 + 4 * i) % 360, 100, 100},
                                      true),
                             40,
                             START_PERIOD,
                             (float)i / 4));

      sec.append(genLine(last_point(true),
                         Point(ILDA_MIN * progress,
                               0,
                               0,
                               {0, 0, 0},
                               false),
                         2,
                         LINEAR,
                         true));

      // // growing line at the beggining
      // std::vector<Point> line = genLine(HSVPoint(ILDA_MIN * progress,
      //                                            ILDA_MIN * progress,
      //                                            0,
      //                                            {(0 + 4 * i) % 360, 100, 100},
      //                                            true),
      //                                   HSVPoint(ILDA_MAX * progress,
      //                                            ILDA_MAX * progress,
      //                                            0,
      //                                            {(180 + 4 * i) % 360, 100, 100},
      //                                            true),
      //                                   20,
      //                                   LINEAR);
      // sec.points.insert(sec.points.end(), line.begin(), line.end());

      // line = genLine(Point(ILDA_MAX * progress,
      //                      ILDA_MAX * progress,
      //                      0,
      //                      {0, 0, 0},
      //                      false),
      //                Point(ILDA_MIN * progress,
      //                      ILDA_MIN * progress,
      //                      0,
      //                      {0, 0, 0},
      //                      false),
      //                5,
      //                LINEAR);
      // sec.points.insert(sec.points.end(), line.begin(), line.end());
    }
    else if (i < 150)
    {
      sec.append(genSineWave(HSVPoint(ILDA_MIN,
                                      ILDA_MIN,
                                      0,
                                      {(0 + 4 * i) % 360, 100, 100},
                                      true),
                             HSVPoint(ILDA_MAX,
                                      ILDA_MAX,
                                      0,
                                      {(180 + 4 * i) % 360, 100, 100},
                                      true),
                             40,
                             START_PERIOD,
                             (float)i / 4));

      sec.append(genLine(last_point(true),
                         Point(ILDA_MIN,
                               0,
                               0,
                               {0, 0, 0},
                               false),
                         2,
                         LINEAR));

      // // line cycling colors for the rest of the frames
      // std::vector<Point> line = genLine(HSVPoint(ILDA_MIN,
      //                                            ILDA_MIN,
      //                                            0,
      //                                            {(0 + i) % 360, 100, 100},
      //                                            true),
      //                                   HSVPoint(ILDA_MAX,
      //                                            ILDA_MAX,
      //                                            0,
      //                                            {(180 + i) % 360, 100, 100},
      //                                            true),
      //                                   20,
      //                                   LINEAR);
      // sec.points.insert(sec.points.end(), line.begin(), line.end());

      // line = genLine(Point(ILDA_MAX,
      //                      ILDA_MAX,
      //                      0,
      //                      {0, 0, 0},
      //                      false),
      //                Point(ILDA_MIN,
      //                      ILDA_MIN,
      //                      0,
      //                      {0, 0, 0},
      //                      false),
      //                5,
      //                LINEAR);
      // sec.points.insert(sec.points.end(), line.begin(), line.end());
    }
    else if (i < 300)
    {
      uint16_t tunnel_frame = i - 150;
      uint8_t rec_count = 4;
      uint16_t rec_cycle_frames = 40;
      sec.points.push_back(Point(ILDA_MIN, ILDA_MIN, 0, {0, 0, 0}, false));
      for (size_t u = 0; u < rec_count; u++)
      {
        if ((rec_cycle_frames * u / rec_count) < tunnel_frame)
        {
          std::cout << "rec" << u << std::endl;
          float rec_size = ((tunnel_frame + (rec_cycle_frames * u / rec_count)) % rec_cycle_frames) / (float)rec_cycle_frames;
          sec.append(genRectangle(Point(ILDA_MIN * rec_size, ILDA_MIN * rec_size, 0, {0, 0, 0}, false),
                                  Point(ILDA_MAX * rec_size, ILDA_MAX * rec_size, 0, {0, 0, 0}, false),
                                  hsv2rgb((u * 100) % 360, 100, 100)));
        }
      }
    }
    else if (i < 400)
    {
      sec.append(genText("1956",
                         Point(ILDA_MIN, ILDA_MIN / 4, 0, {0, 0, 0}, false),
                         Point(ILDA_MAX, ILDA_MAX / 4, 0, {0, 0, 0}, false),
                         hsv2rgb((i * 10) % 360, 100, 100),
                         0.3,
                         last_point(),
                         letters_appear_skip(4, 300, 350, i)));
      sec.append(last_point(true));
      sec.append(genSineWave(HSVPoint(ILDA_MIN, ILDA_MIN, 0, {(i * 10) % 360, 255, 255}, true),
                             HSVPoint(ILDA_MAX, ILDA_MIN / 10 * 9, 0, {(i * 10) % 360, 255, 255}, true), 20, 3.14 * 8, (float)i / 2));
      sec.append(last_point(true));
      sec.append(genSineWave(HSVPoint(ILDA_MIN, ILDA_MAX, 0, {(i * 10) % 360, 255, 255}, true),
                             HSVPoint(ILDA_MAX, ILDA_MAX / 10 * 9, 0, {(i * 10) % 360, 255, 255}, true), 20, 3.14 * 8, (float)i / 2));
    }
    else
    {
      sec.append(genText("PR.N.L.",
                         Point(ILDA_MIN, ILDA_MIN / 4, 0, {0, 0, 0}, false),
                         Point(ILDA_MAX, ILDA_MAX / 4, 0, {0, 0, 0}, false),
                         hsv2rgb((i * 10) % 360, 100, 100),
                         0.3,
                         last_point(),
                         /*(i < 300) ? letters_appear_skip(6, 200, 300, i) :*/ 0));

      sec.append(last_point(true));
      sec.append(genSineWave(HSVPoint(ILDA_MIN, ILDA_MIN, 0, {(i * 10) % 360, 255, 255}, true),
                             HSVPoint(ILDA_MAX, ILDA_MIN / 10 * 9, 0, {(i * 10) % 360, 255, 255}, true), 20, 3.14 * 8, (float)i / 2));
      sec.append(last_point(true));
      sec.append(genSineWave(HSVPoint(ILDA_MIN, ILDA_MAX, 0, {(i * 10) % 360, 255, 255}, true),
                             HSVPoint(ILDA_MAX, ILDA_MAX / 10 * 9, 0, {(i * 10) % 360, 255, 255}, true), 20, 3.14 * 8, (float)i / 2));
    }

    WRITE_SEC
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