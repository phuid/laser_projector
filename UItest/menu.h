#include <vector>
#include <stdint.h>
#include <iostream>

char parent_char[] = {
		0b00000,
		0b00100,
		0b01110,
		0b11111,
		0b00100,
		0b00100,
		0b00111,
		0b00000,
	};
#define PARENT_CHAR_NUM 1

template <typename T>
struct menu_val
{
	T num;
	T min;
	T max;
};

template <typename T>
bool change_val(T *val, T val_min = 0, T val_max = 100) // return 1 if changed
{
	if (encoder_pos > 0)
	{
		if (encoder_pos < val_max - *val)
			*val += encoder_pos;
		else
			*val = val_max;
	}
	else if (encoder_pos < 0)
	{
		if (encoder_pos < *val - val_min)
			*val += encoder_pos;
		else
			*val = val_min;
	}
	else
	{
		return 0;
	}
	encoder_pos = 0;
	return 1;
}

enum menu_option_style
{
	NESTED_MENU = 0,
	VALUE,
	SELECTION, // all childrem are style text, nest_selected is written to value after button is pressed
	FUNCTION,
	TEXT,
	ROOT_MENU,
};

struct menu_option
{
	char *name;
	menu_option_style style;

	std::vector<menu_option> nested_menu_options; // does this work????????
	uint8_t nest_selected = 0;
	uint8_t nest_scroll = 0;
	bool nest_option_active = 0;

	menu_val<int16_t> value;

	void (*function)(void);
};

void print_test() // TODO: remove
{
	std::cout << "print test" << std::endl;
}

void menu_interact(std::vector<menu_option> *menu, uint8_t *menu_selected, uint8_t *menu_scroll, bool *parent_menu_option_active, menu_option_style parent_menu_style, bool redraw = 0)
{
	if (parent_menu_option_active)
	{
		switch ((*menu)[*menu_selected].style)
		{
		case NESTED_MENU:
		case SELECTION:
			menu_interact(&(*menu)[*menu_selected].nested_menu_options, &(*menu)[*menu_selected].nest_selected, &(*menu)[*menu_selected].nest_scroll, &(*menu)[*menu_selected].nest_option_active, (*menu)[*menu_selected].style, redraw);
			break;

		case VALUE:

			break;

		default:
			break;
		}
	}
	else
	{
		bool scrolled = change_val<uint8_t>(menu_selected, 0, (uint8_t)menu->size());
		if (scrolled || encoder_btn_pressed || redraw)
		{
			if (scrolled)
			{
				// handle scroll - prolly totally wrong :skull:
				if (*menu_selected > (*menu_scroll + 1) && *menu_selected - (*menu_scroll + 1) > SCREEN_HEIGHT / 2)
				{
					*menu_scroll += *menu_selected - (*menu_scroll + 1);
				}
				else if (*menu_selected < (*menu_scroll + 1) && (*menu_scroll + 1) - *menu_selected > SCREEN_HEIGHT / 2)
				{
					*menu_scroll -= (*menu_scroll + 1) - *menu_selected;
				}
			}

			if (encoder_btn_pressed)
			{
				// handle button
				// TODO: handle back button
				if ((*menu)[*menu_selected].style == NESTED_MENU || (*menu)[*menu_selected].style == SELECTION) {
					(*menu)[*menu_selected].nest_option_active = 1;
				}
				else if ((*menu)[*menu_selected].style == VALUE) {
					change_val<decltype((*menu)[*menu_selected].value.num)>(&(*menu)[*menu_selected].value.num, (*menu)[*menu_selected].value.min, (*menu)[*menu_selected].value.max);
				}
			}
			// redraw
		}
	}
}