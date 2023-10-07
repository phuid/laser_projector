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

const uint8_t encoder_pins[2] = {25, 27};
const uint8_t button_pin = 23;

bool encoder_pins_last_state[2] = {0, 0};

int16_t pos = 0;
bool btn_pressed = 0;

void handle_btn_interrupts()
{
	btn_pressed = 1;
}

void handle_enc_interrupts()
{
	bool state[2] = {digitalRead(encoder_pins[0]), digitalRead(encoder_pins[1])};

	bool A_rising = state[0] && !encoder_pins_last_state[0];
	bool B_rising = state[1] && !encoder_pins_last_state[1];

	bool A_falling = !state[0] && encoder_pins_last_state[0];
	bool B_falling = !state[1] && encoder_pins_last_state[1];

	if (A_rising /* || B_rising || A_falling || B_falling*/) //dont need high precision, my encoder has feelable steps and 4 interrupts in each
	{

		bool dir = 0;

		if (A_rising && !state[1])
		{
			dir = !dir;
			// else do nothing
		}
		// else if (A_falling && state[1])
		// {
		// 	dir = !dir;
		// 	// else do nothing
		// }
		// if (B_rising && state[0])
		// {
		// 	dir = !dir;
		// 	// else do nothing
		// }
		// else if (B_falling && !state[0])
		// {
		// 	dir = !dir;
		// 	// else do nothing
		// }

		// std::cout << "state: " << state[0] << state[1];
		// std::cout << ", dir: " << dir << std::endl;

		pos += (dir ? 1 : -1);
	}
	encoder_pins_last_state[0] = state[0];
	encoder_pins_last_state[1] = state[1];
}

template <typename T>
void change_val(lcd_t *lcd, int16_t val_max, int16_t val_min, T val, char *suffix, uint8_t pos_row, uint8_t pos_column)
{
	pos = 0;
	while (!btn_pressed)
	{
		if (pos > 0)
		{
			if (*val + pos <= val_max)
				*val += pos;
			else
				*val = val_max;
		}
		else if (pos < 0)
		{
			if (*val + pos >= val_min)
				*val += pos;
			else
				*val = val_min;
		}
		pos = 0;

		lcd_pos(lcd, pos_row, pos_column);
		lcd_printf(lcd, "%2d%s ", *val, suffix);

		usleep(1000000 / 30);
	}
	btn_pressed = 0;
	pos = 0;
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

	/* Print a string in each of 4 lines */
	// for (size_t i = 0; i < 4; i++)
	// {
	// 	lcd_print(lcd, (char *)"This is line ");
	// 	lcd_print(lcd, const_cast<char *>(std::to_string(i).c_str()));
	// 	lcd_print(lcd, (char *)"\n");
	// }

	pinMode(encoder_pins[0], INPUT);
	pinMode(encoder_pins[1], INPUT);
	pinMode(button_pin, INPUT);

	pullUpDnControl(encoder_pins[0], PUD_UP);
	pullUpDnControl(encoder_pins[1], PUD_UP);
	pullUpDnControl(button_pin, PUD_UP);

	wiringPiISR(encoder_pins[0], INT_EDGE_BOTH, *handle_enc_interrupts);
	wiringPiISR(encoder_pins[1], INT_EDGE_BOTH, *handle_enc_interrupts);
	wiringPiISR(button_pin, INT_EDGE_FALLING, *handle_btn_interrupts);

	int8_t br = 50; // percent
	lcd_print(lcd, (char *)"brightness:");

	while (true)
	{
		change_val<int8_t *>(lcd, 100, 0, &br, (char *)"%", 0, 12);
		lcd_backlight_dim(lcd, (float)br / 100.f);
	}

	lcd_backlight_off(lcd);

	return 0;
}
