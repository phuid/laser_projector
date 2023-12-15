#pragma once

#include <wiringPi.h>
#include <stdint.h>
#include <stdbool.h> //TODO: lol why include stdbool in c++?

const uint8_t encoder_pins[2] = {ENCODER_PINS[0], ENCODER_PINS[1]};
const uint8_t encoder_button_pin = ENCODER_BUTTON_PIN;

void handle_enc_btn_interrupts();

void handle_enc_interrupts();
