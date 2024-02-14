#include <pigpio.h>
#include <iostream>

int main() {
  if (gpioInitialise() < 0) {
    std::cout << "yikes" << std::endl;
    exit(1);
  }

  // int h = spiOpen(2, 115200, 0b0000000000000101100010);

  // if (h < 0) {
  //   std::cout << "nohandle" << std::endl;
  //   exit(1);
  // }

  // char buf;
  // while (true) {
  //   int r = spiRead(h, &buf, 1);
  //   std::cout << "r:" << r <<std::endl;
  //   if (r > 0) {
  //     std::cout << "gott: " << static_cast<int>(buf) << std::endl;
  //   }
  //   else {
  //     std::cout << "weird" << std::endl;
  //   }
  // }


  constexpr unsigned CS = 16;
  constexpr unsigned MISO = 19;
  constexpr unsigned SCLK = 21;

  gpioSetMode(SCLK, PI_OUTPUT);
  gpioSetMode(CS, PI_OUTPUT);
  gpioSetMode(MISO, PI_INPUT);


  while (true) {
    uint8_t buf = 0;
    
    gpioWrite(SCLK, 0);
    gpioDelay(100);
    gpioWrite(CS, 0);
    gpioDelay(100);
    gpioWrite(SCLK, 1);

    gpioDelay(200);
    gpioWrite(SCLK, 0);

    gpioDelay(200);
    if (gpioRead(MISO)) {
      std::cout << "meh---";
    }
    gpioWrite(SCLK, 1);
    gpioDelay(200);
    gpioWrite(SCLK, 0);

    for (uint8_t i = 0; i < 8; i++) {
    gpioDelay(200);
    buf = buf | (static_cast<bool>(gpioRead(MISO)) << (7 - i));
    std::cout << static_cast<bool>(buf & (1 << (7 - i)));
    gpioWrite(SCLK, 1);
    gpioDelay(200);
    gpioWrite(SCLK, 0);
    }
    gpioWrite(CS, 1);
    std::cout << " out:" << static_cast<int>(buf) << " === U:" << (5.f/256.f) * buf + 5 << std::endl;
    gpioDelay(1000000);
  }
}