#pragma once

#include <wiringPi.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t encoder_pins[2] = ENCODER_PINS;
uint8_t encoder_button_pin = ENCODER_BUTTON_PIN;

bool encoder_pins_last_state[2] /* = {0, 0}*/;

int16_t encoder_pos = 0;
bool encoder_btn_pressed = 0;

void handle_enc_btn_interrupts()
{
	usleep(20000); //TODO: handle debounce in a more inteligent way, like set a timer to check this instead of waiting
	if (!digitalRead(encoder_button_pin))
		encoder_btn_pressed = 1;
}

void handle_enc_interrupts()
{
	bool state[2] = {digitalRead(encoder_pins[0]), digitalRead(encoder_pins[1])};

	// bool A_rising = state[0] && !encoder_pins_last_state[0];
	// bool B_rising = state[1] && !encoder_pins_last_state[1];

	bool A_falling = !state[0] && encoder_pins_last_state[0];
	// bool B_falling = !state[1] && encoder_pins_last_state[1];

	if (/*A_rising  || B_rising ||*/ A_falling /* || B_falling*/) // dont need high precision, my encoder has feelable steps and 4 interrupts in each
	{

		bool dir = 0;

		// if (A_rising && !state[1])
		// {
		// 	dir = !dir;
		// 	// else do nothing
		// }
		/*else */ if (/* A_falling && */ state[1])
		{
			dir = !dir;
			// else do nothing
		}
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

		encoder_pos += (dir ? 1 : -1);
	}
	encoder_pins_last_state[0] = state[0];
	encoder_pins_last_state[1] = state[1];
}