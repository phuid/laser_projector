#include "zmq.hpp"
#include "my_helper.hpp"
#include <string>
#include <pigpio.h>

void publish_message(zmq::socket_t &publisher, std::string message_string)
{
  publisher.send(zmq::message_t(message_string.c_str(), message_string.length()), zmq::send_flags::none);
}

void publish_message(std::string message_string)
{
  zmq::context_t ctx(1);

  zmq::socket_t publisher(ctx, zmq::socket_type::pub);
  publisher.connect("tcp://*:5556");
  
  publisher.send(zmq::message_t(message_string.c_str(), message_string.length()), zmq::send_flags::none);
}

uint8_t bat_raw () {

  char buf = 0;
  
  gpioWrite(SCLK, 0);
  gpioDelay(100);
  gpioWrite(CS, 0);
  gpioDelay(100);
  gpioWrite(SCLK, 1);

  gpioDelay(200);
  gpioWrite(SCLK, 0);

  gpioDelay(200);
  if (gpioRead(MISO)) {
    std::cout << "bad communication with adc for bat_raw reading" << std::endl;
    return -1;
  }
  gpioWrite(SCLK, 1);

  gpioDelay(200);
  gpioWrite(SCLK, 0);

  for (uint8_t i = 0; i < 8; i++) {
    gpioDelay(200);
    buf = buf | (gpioRead(MISO) << (7 - i));
    // std::cout << static_cast<bool>(buf & (1 << (7 - i)));
    gpioWrite(SCLK, 1);
    gpioDelay(200);
    gpioWrite(SCLK, 0);
  }
  gpioWrite(CS, 1);
  return buf;
}