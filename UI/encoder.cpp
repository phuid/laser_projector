#include <wiringPi.h>
#include "encoder.hpp"
#include <iostream>

static bool encoder_btn_pressed = 0;

static uint16_t last_interrupt = 0;
void handle_enc_btn_falling()
{
	uint16_t interrupt_time = millis();

	if (interrupt_time - last_interrupt > 200)
	{
		// std::cout << "-------------------------------------------------";
		encoder_btn_pressed = 1;
	}
	// std::cout << "interrupt:" << interrupt_time << "last:" << last_interrupt;
	// std::cout << std::endl;
	last_interrupt = interrupt_time;
}

void handle_enc_btn_rising()
{
	last_interrupt = millis();
}

static int16_t encoder_pos = 0;

void handle_enc_interrupts()
{
	static bool encoder_pins_last_state[2] = {0, 0};
	bool state[2] = {(bool)digitalRead(encoder_pins[0]), (bool)digitalRead(encoder_pins[1])};

	bool A_rising = state[0] && !encoder_pins_last_state[0];
	bool B_rising = state[1] && !encoder_pins_last_state[1];

	bool A_falling = !state[0] && encoder_pins_last_state[0];
	bool B_falling = !state[1] && encoder_pins_last_state[1];

	if (A_rising || B_rising || A_falling  || B_falling)
	{

		bool dir = 0;

		if (A_rising && !state[1])
		{
			dir = !dir;
			// else do nothing
		}
		else  if ( A_falling &&  state[1])
		{
			dir = !dir;
			// else do nothing
		}
		if (B_rising && state[0])
		{
			dir = !dir;
			// else do nothing
		}
		else if (B_falling && !state[0])
		{
			dir = !dir;
			// else do nothing
		}

		// std::cout << "state: " << state[0] << state[1];
		// std::cout << ", dir: " << dir << std::endl;

		encoder_pos += (dir ? 1 : -1);
	}
	encoder_pins_last_state[0] = state[0];
	encoder_pins_last_state[1] = state[1];
}

bool get_encoder_btn_pressed()
{
	return encoder_btn_pressed;
}

int16_t get_encoder_pos()
{
	return encoder_pos / INTERRUPTS_PER_ENCODER_STEP; // 4 interrupts per step
}

void clear_encoder_btn_pressed(bool set)
{
	encoder_btn_pressed = set;
}
void set_encoder_pos(int16_t set)
{
	encoder_pos = set;
}