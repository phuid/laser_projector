#include <iostream>
#include <lgpio.h>

int main()
{
    int gpio_chip_handle = lgGpiochipOpen(4);
    if (gpio_chip_handle < 0)
    {
        std::cout << "Failed to open gpio chip" << std::endl;
        return 1;
    }
    std::cout << "Opened gpio chip" << std::endl;

    int pins[] = {17, 22, 27};
    for (size_t i = 0; i < 3; i++)
    {
        if (lgGpioClaimOutput(gpio_chip_handle, LG_SET_PULL_DOWN, pins[i], 0) < 0)
        {
            std::cout << "Failed to claim pin " << static_cast<int>(pins[i]) << std::endl;
            return 1;
        }
    }
    uint8_t color[3] = {255, 0, 0};
    uint8_t color_cycle = 0;

    while (true)
    {
        lguSleep(0.01);
        for (size_t i = 0; i < 3; i++)
        {
            lgTxPwm(gpio_chip_handle, pins[i], 10000, (static_cast<float>(color[i]) / 255.f) * 100.f, 0, 0);
        }

        color[color_cycle]--;
        color[(color_cycle + 1) % 3]++;
        
        for (size_t i = 0; i < 3; i++)
        {
            if (color[i] == 255)
            {
                color_cycle = (color_cycle + 1) % 3;
            }
        }
    }

    return 0;
}