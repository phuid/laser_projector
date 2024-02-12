#pragma once

#define DEBUG

constexpr uint8_t ENCODER_PINS[2] = {5, 6};
constexpr uint8_t ENCODER_BUTTON_PIN = 13;

constexpr uint8_t LCD_SDA_PIN = 2;
constexpr uint8_t LCD_SCL_PIN = 3;


constexpr uint8_t SCREEN_HEIGHT = 4;
constexpr uint8_t SCREEN_WIDTH = 20;

constexpr uint8_t INTERRUPTS_PER_ENCODER_STEP = 4;