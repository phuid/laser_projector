#include <lgpio.h>
#include "encoder.hpp"

static bool encoder_btn_pressed = 0;

void handle_enc_btn_interrupts(int num_alerts, lgGpioAlert_p alerts, void *userdata)
{
	for (size_t i = 0; i < num_alerts; i++)
	{
		lgGpioReport_t &alert_report = alerts[i].report;
		/*
		* typedef struct
		*	{
    * 	uint64_t timestamp; // alert time in nanoseconds
    * 	uint8_t chip;       // gpiochip device number
    * 	uint8_t gpio;       // offset into gpio device
    * 	uint8_t level;      // 0=low, 1=high, 2=watchdog
   	* 	uint8_t flags;      // none defined, ignore report if non-zero
		* } lgGpioReport_t;
		*/


		static uint64_t last_interrupt_ns = 0;
		if (alert_report.timestamp - last_interrupt_ns > 50 * 1000*1000 && !alert_report.level)
		{
			encoder_btn_pressed = 1;
		}
		last_interrupt_ns = alert_report.timestamp;
	}
}

static int16_t encoder_pos = 0;

void handle_enc_interrupts(int num_alerts, lgGpioAlert_p alerts, void *userdata) // TODO: rewrite, register less interrupts
{
	int gpio_chip_handle = *static_cast<int*>(userdata);

	static bool encoder_pins_last_state[2] = {0, 0};
	bool state[2] = {(bool)lgGpioRead(gpio_chip_handle, encoder_pins[0]), (bool)lgGpioRead(gpio_chip_handle, encoder_pins[1])};

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