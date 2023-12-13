#pragma once

#include <wiringPi.h>
#include <stdint.h>
#include <stdbool.h> //TODO: lol why include stdbool in c++?

// TODO: call static and work with static???

class encoder
{
private:
  uint8_t encoder_pins[2] = {ENCODER_PINS[0], ENCODER_PINS[1]};
  uint8_t encoder_button_pin = ENCODER_BUTTON_PIN;

  bool encoder_btn_pressed = 0;
  uint16_t last_interrupt = 0;

  int16_t encoder_pos = 0; // TODO: static in .cpp
  bool encoder_pins_last_state[2] = {0, 0};

public:
  encoder(uint8_t pin_a, uint8_t pin_b, uint8_t btn_pin);
  ~encoder();

  int16_t get_pos()
  {
    return encoder_pos;
  }
  void set_pos(int16_t pos)
  {
    encoder_pos = pos;
  }

  int16_t get_btn_pressed()
  {
    return encoder_btn_pressed;
  }
  void clear_btn()
  {
    encoder_btn_pressed = 0;
  }

  void handle_enc_btn_interrupts()
  {
    uint16_t interrupt_time = millis();
    if (interrupt_time - this->last_interrupt > 50 && !digitalRead(this->encoder_button_pin))
    {
      this->encoder_btn_pressed = 1;
    }
    this->last_interrupt = interrupt_time;
  }

  void handle_enc_interrupts() // TODO: rewrite, register less interrupts
  {
    bool state[2] = {(bool)digitalRead(this->encoder_pins[0]), (bool)digitalRead(this->encoder_pins[1])};

    // bool A_rising = state[0] && !encoder_pins_last_state[0];
    // bool B_rising = state[1] && !encoder_pins_last_state[1];

    bool A_falling = !state[0] && this->encoder_pins_last_state[0];
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

      this->encoder_pos += (dir ? 1 : -1);
    }
    this->encoder_pins_last_state[0] = state[0];
    this->encoder_pins_last_state[1] = state[1];
  }
};

encoder::encoder(uint8_t pin_a, uint8_t pin_b, uint8_t btn_pin)
{
  encoder_pins[0] = pin_a;
  encoder_pins[1] = pin_b;
  encoder_button_pin = btn_pin;
}

encoder::~encoder()
{
}