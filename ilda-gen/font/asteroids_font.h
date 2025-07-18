/** \file
 * Asteroids simple font.
 */
#ifndef _asteroids_font_h_
#define _asteroids_font_h_

#include <stdint.h>

typedef struct
{
	uint8_t points[8]; // 4 bits x, 4 bits y
} asteroids_char_t;

#define FONT_UP 0xFE
#define FONT_LAST 0xFF

#define FONT_NUM_OF_CHARACTERS 69

#ifdef __cplusplus
extern "C"
{
#endif

	extern const asteroids_char_t asteroids_font[];

	extern const char asteroids_font_chars[];

#ifdef __cplusplus
}
#endif

#endif