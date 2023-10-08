/* Program to test LCD basic usage.
 *
 * Electronicayciencia
 * https://github.com/electronicayciencia/wPi_soft_lcd
 *
 * Compile this way:
 * gcc -lwiringPi -o example_basic example_basic.c soft_lcd.c soft_i2c.c
 *
 * Reinoso G.   25/07/2018
 */

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <wiringPi.h>
#include <iostream>
#include "soft_lcd.h"

#define ENCODER_PINS \
	{                  \
		25, 27           \
	}
#define ENCODER_BUTTON_PIN 23
#include "encoder.h"

template <typename T>
struct menu_val {
	T val;
	T min;
	T max;
};

template <typename T>
void change_val(int16_t val_min, int16_t val_max, T val)
{
	if (encoder_pos > 0)
	{
		if (*val + encoder_pos <= val_max)
			*val += encoder_pos;
		else
			*val = val_max;
	}
	else if (encoder_pos < 0)
	{
		if (*val + encoder_pos >= val_min)
			*val += encoder_pos;
		else
			*val = val_min;
	}
	encoder_pos = 0;
}

int main()
{
	wiringPiSetup();

	/* Create a LCD given SCL, SDA and I2C address, 4 lines */
	/* PCF8574 has default address 0x27 */
	lcd_t *lcd = lcd_create(9, 8, 0x27, 4);

	if (lcd == NULL)
	{
		printf("Cannot set-up LCD.\n");
		return 1;
	}

	lcd_init(lcd);

	pinMode(encoder_pins[0], INPUT);
	pinMode(encoder_pins[1], INPUT);
	pinMode(encoder_button_pin, INPUT);

	pullUpDnControl(encoder_pins[0], PUD_UP);
	pullUpDnControl(encoder_pins[1], PUD_UP);
	pullUpDnControl(encoder_button_pin, PUD_UP);

	wiringPiISR(encoder_pins[0], INT_EDGE_BOTH, *handle_enc_interrupts);
	wiringPiISR(encoder_pins[1], INT_EDGE_BOTH, *handle_enc_interrupts);
	wiringPiISR(encoder_button_pin, INT_EDGE_FALLING, *handle_btn_interrupts);

	menu_val<int8_t> screen_brightness = {50, 0, 100};

	while (true)
	{
		lcd_pos(lcd, 0, 0);
		change_val<int8_t *>(screen_brightness.min, screen_brightness.max, &screen_brightness.val);
		lcd_backlight_dim(lcd, (float)screen_brightness.val / 100.f);
		lcd_printf(lcd, (char *)"brightness:%d%% ", screen_brightness.val);
	}

	lcd_backlight_off(lcd);

	return 0;
}
