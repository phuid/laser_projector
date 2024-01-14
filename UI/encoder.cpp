#include <pigpio.h>
#include "encoder.hpp"

constexpr uint16_t BUTTON_DEBOUNCE_DELAY = 50000;

static bool encoder_btn_pressed = 0;

void handle_enc_btn_interrupts(int gpio, int level, uint32_t tick)
{
	static uint32_t last_interrupt_tick = 0;
	if (tick - last_interrupt_tick > BUTTON_DEBOUNCE_DELAY && !gpioRead(encoder_button_pin))
	{
		encoder_btn_pressed = 1;
	}
	last_interrupt_tick = tick;
}

static int16_t encoder_pos = 0;

void handle_enc_interrupts(int gpio, int level, uint32_t tick) // TODO: rewrite, register less interrupts
{
	static bool encoder_pins_last_state[2] = {0, 0};
	bool state[2] = {(bool)gpioRead(encoder_pins[0]), (bool)gpioRead(encoder_pins[1])};

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

bool get_encoder_btn_pressed()
{
	return encoder_btn_pressed;
}

int16_t get_encoder_pos()
{
	return encoder_pos;
}

void clear_encoder_btn_pressed(bool set)
{
	encoder_btn_pressed = set;
}
void set_encoder_pos(int16_t set)
{
	encoder_pos = set;
}