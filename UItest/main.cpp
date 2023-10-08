#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
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
struct menu_val
{
	T num;
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

enum menu_option_style
{
	NESTED_MENU = 0,
	VALUE,
	SELECTION,
	FUNCTION,
	TEXT
};

struct menu_option
{
	char *name;
	menu_option_style style;

	std::vector<menu_option> nested_options; // does this work????????
	uint8_t nest_selected = 0;
	uint8_t nest_scroll = 0;
	menu_val<int16_t> value;
	std::vector<std::string> selection; // value changes with selection
	void (*function)(void);
};

void print_test()
{
	std::cout << "print test" << std::endl;
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

	std::vector<menu_option> menu = {
			{
					.name = (char *)"1 - nest",
					.style = NESTED_MENU,
					.nested_options = {
							{
									.name = (char *)"1.1 - text",
									.style = TEXT,
							},
							{.name = (char *)"1.2 - func",
							 .style = FUNCTION,
							 .function = *print_test},
					},
			},
			{
					.name = (char *)"2 - text",
					.style = TEXT,
			},
			{
					.name = (char *)"3 - val - brightness",
					.style = VALUE,
					.value = {50, 0, 100},
			},
			{
					.name = (char *)"4 - func",
					.style = FUNCTION,
					.function = *print_test,
			},
	};
	uint8_t menu_selected = 0;
	uint8_t menu_scroll = 0;

	while (true)
	{
		lcd_pos(lcd, 0, 0);
		change_val<int16_t *>(menu[2].value.min, menu[2].value.max, &menu[2].value.num);
		lcd_backlight_dim(lcd, (float)menu[2].value.num / 100.f);
		lcd_printf(lcd, (char *)"brightness: %d%% ", menu[2].value.num);
	}

	lcd_backlight_off(lcd);

	return 0;
}
