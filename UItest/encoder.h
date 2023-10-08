#include <wiringPi.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef ENCODER_H
#define ENCODER_H

const uint8_t encoder_pins[2]/* = {25, 27}*/;
const uint8_t button_pin/* = 23*/;

bool encoder_pins_last_state[2]/* = {0, 0}*/;

int16_t pos = 0;
bool btn_pressed = 0;
#endif