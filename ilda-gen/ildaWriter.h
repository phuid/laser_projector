#include <vector>
#include "FormatData.h"
#include <math.h>

#define ILDA_MIN -32768
#define ILDA_MAX 32767

enum section_format
{
    ILDA_3D_INDEXED = 0,
    ILDA_2D_INDEXED = 1,
    ILDA_COLOR_PALETTE = 2,
    ILDA_3D_REAL = 4,
    ILDA_2D_REAL = 5,
};

struct RGBColor {
  uint8_t r; // Red component (0-255)
  uint8_t g; // Green component (0-255)
  uint8_t b; // Blue component (0-255)
};

struct point {
    int16_t x;
    int16_t y;
    int16_t z;

    RGBColor color;

    bool last_point; // Indicates if this is the last point in a frame.
    bool laser_on;
};

struct section {
    section_format format = ILDA_2D_REAL;
    uint16_t number_of_records;
    uint16_t frame_number;
    uint8_t projector_number = 0;

    std::vector<point> points;
};

RGBColor hsv2rgb(float H, float S, float V) {
	float r, g, b;
	
	float h = H / 360;
	float s = S / 100;
	float v = V / 100;
	
	int i = floor(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	
	RGBColor color;
	color.r = r * 255;
	color.g = g * 255;
	color.b = b * 255;
	
	return color;
}

int16_t little_endian_to_big_endian(int16_t value) {
  return (value >> 8) | (value << 8);
}