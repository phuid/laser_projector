#include <iostream>
#include <lgpio.h>

int main () {
    int gpio_chip_handle = lgGpiochipOpen(0);
    if (gpio_chip_handle < 0) {
        std::cout << "Failed to open gpio chip" << std::endl;
        return 1;
    }
    std::cout << "Opened gpio chip" << std::endl;

    int pin = 22;

    if (lgGpioClaimOutput(gpio_chip_handle, LG_SET_PULL_DOWN, pin, 0) < 0) {
        std::cout << "Failed to claim pin" << std::endl;
        return 1;
    }
    uint8_t duty = 0;
    int left_pules = lgTxPwm(gpio_chip_handle, pin, 10000, (static_cast<float>(duty++) / 255.f) * 100.f, 0, 0);
    while (true) {
      std::cout << "left_pules: " << left_pules << std::endl;
      left_pules = lgTxPwm(gpio_chip_handle, pin, 10000, (static_cast<float>(duty++) / 255.f) * 100.f, 0, 0);
    }

    return 0;
}