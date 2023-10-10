#include "encoder.h"
#include <stdbool.h>

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