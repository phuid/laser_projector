#pragma once

#include <wiringPi.h>
#include <stdint.h>
#include "config.hpp"

const uint8_t encoder_pins[2] = {ENCODER_PINS[0], ENCODER_PINS[1]};
const uint8_t encoder_button_pin = ENCODER_BUTTON_PIN;

void handle_enc_btn_falling();
void handle_enc_btn_rising();

void handle_enc_interrupts();

bool get_encoder_btn_pressed();
int16_t get_encoder_pos();

void clear_encoder_btn_pressed(bool set = 0);
void set_encoder_pos(int16_t set);