#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <wiringPi.h>
#include <iostream>

#include "zmq.hpp"
#include "soft_lcd.h"

#include "encoder.hpp"
#include "menu.hpp"

#include <filesystem>

void print_test(menu_option &nest)
{
    std::cout << "print test: " << nest.name << std::endl;
}

void fill_with_files(menu_option &nest)
{
    std::cout << "searching for files" << std::endl;
    std::string dirpath = "../ild";
    for (const auto &entry : std::filesystem::directory_iterator(dirpath))
    {
        std::string path = entry.path();
        std::size_t found = path.find_last_of("/\\");
        if (found != std::string::npos)
        {
            path = path.substr(found);
        }
#ifdef DEBUG
        std::cout << path << std::endl;
#endif
        menu_option tmp = {.name = path, .style = TEXT, .has_function = 1, .function = print_test};
        nest.nested_menu_options.push_back(tmp);
    }
}

int main()
{
    /* WARNING: Setting PWM status as a non root user
     * may crash some versions of Raspbian. */
    // if (geteuid() != 0)
    // {
    //     puts("This program must be run as root.");
    //     return 1;
    // }

    // TODO: add SIGINT handler + cleanup function

    wiringPiSetup();

    /* Create a LCD given SCL, SDA and I2C address, 4 lines */
    /* PCF8574 has default address 0x27 */
    lcd_t *lcd = lcd_create(9, 8, 0x27, SCREEN_HEIGHT);

    if (lcd == NULL)
    {
        printf("Cannot set-up LCD.\n");
        return 1;
    }

    lcd_init(lcd);

    pinMode(encoder_pins[0], INPUT);
    pinMode(encoder_pins[1], INPUT);
    pinMode(encoder_button_pin, INPUT);

    pullUpDnControl(encoder_pins[0], PUD_UP);
    pullUpDnControl(encoder_pins[1], PUD_UP);
    pullUpDnControl(encoder_button_pin, PUD_UP);

    wiringPiISR(encoder_pins[0], INT_EDGE_BOTH, *handle_enc_interrupts);
    wiringPiISR(encoder_pins[1], INT_EDGE_BOTH, *handle_enc_interrupts);
    wiringPiISR(encoder_button_pin, INT_EDGE_BOTH, *handle_enc_btn_interrupts);

    lcd_create_char(lcd, PARENT_CHAR_NUM, parent_char);
    lcd_create_char(lcd, INVERTED_SPACE_CHAR_NUM, inverted_space_char);
    lcd_create_char(lcd, INVERTED_POINTER_CHAR_NUM, inverted_pointer_char);

    zmq::context_t ctx(1);

    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5556");
    subscriber.set(zmq::sockopt::subscribe, "");

    zmq::socket_t command_sender(ctx, zmq::socket_type::pub);
    command_sender.connect("tcp://localhost:5557");

    menu_option root = {
        .style = ROOT_MENU,
        .nested_menu_options = {
            {
                .name = "project",
                .style = NESTED_MENU,
                .has_function = 1,
                .function = fill_with_files,
            },
            {.name = "options",
             .style = NESTED_MENU,
             .nested_menu_options = {
                 {
                     .name = "screen brightness",
                     .style = VALUE,
                     .value = {50, 0, 100},
                 },
                 {
                     .name = "trapezoid_horizontal",
                     .style = VALUE,
                     .value = {0, -1.f, 1.f},
                 },
             }},
        }};

    float &brightness_val = root.nested_menu_options[1].nested_menu_options[0].value.num;

    menu_interact(lcd, command_sender, root, true);
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder
        menu_interact(lcd, command_sender, root);

        // lcd_backlight_dim(lcd, brightness_val);

        // separate root menu element for drawing
        // maybe some placeholders to be replaced in the definition
        // .name = "%name%"
        //
        // menu_interact(lcd, playing_root);
    }

    lcd_backlight_off(lcd);

    return 0;
}
