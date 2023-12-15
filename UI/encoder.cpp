#include "encoder.hpp"

static bool encoder_btn_pressed = 0;

void handle_enc_btn_interrupts()
{
	static uint16_t last_interrupt = 0;
	uint16_t interrupt_time = millis();
	if (interrupt_time - last_interrupt > 50 && !digitalRead(encoder_button_pin))
	{
		encoder_btn_pressed = 1;
	}
	last_interrupt = interrupt_time;
}

static int16_t encoder_pos = 0; // TODO: static in .cpp

void handle_enc_interrupts() //TODO: rewrite, register less interrupts
{
	static bool encoder_pins_last_state[2] = {0, 0};
	bool state[2] = {(bool)digitalRead(encoder_pins[0]), (bool)digitalRead(encoder_pins[1])};

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