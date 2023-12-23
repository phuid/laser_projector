#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <wiringPi.h>
#include <iostream>

#include <sys/stat.h>
#include <errno.h>

#include "soft_lcd.h"

#include "encoder.hpp"
#include "menu.hpp"

int main()
{
    /* WARNING: Setting PWM status as a non root user
     * may crash some versions of Raspbian. */
    if (geteuid() != 0)
    {
        puts("This program must be run as root.");
        return 1;
    }

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

    menu_option root = {
        .nested_menu_options = {
            {
                .name = const_cast<char *>(std::string("nest").c_str()),
                .style = NESTED_MENU,
                .nested_menu_options = {
                    {
                        .name = const_cast<char *>(std::string("text").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("text1").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("text2").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("text3").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("func").c_str()),
                        .style = FUNCTION,
                        .function = *print_test,
                    },
                },
            },
            {
                .name = const_cast<char *>(std::string("text").c_str()),
                .style = TEXT,
            },
            {
                .name = const_cast<char *>(std::string("brightness").c_str()),
                .style = VALUE,
                .value = {50, 0, 100},
            },
            {
                .name = const_cast<char *>(std::string("val2").c_str()),
                .style = VALUE,
                .value = {50, INT16_MIN, INT16_MAX},
            },
            {
                .name = const_cast<char *>(std::string("value3").c_str()),
                .style = VALUE,
                .value = {4000, INT16_MIN, INT16_MAX},
            },
            {
                .name = const_cast<char *>(std::string("value4").c_str()),
                .style = VALUE,
                .value = {12345, INT16_MIN, INT16_MAX},
            },
            {
                .name = const_cast<char *>(std::string("func").c_str()),
                .style = FUNCTION,
                .function = *print_test,
            },
            {
                .name = const_cast<char *>(std::string("select").c_str()),
                .style = SELECTION,
                .nested_menu_options = {
                    {
                        .name = const_cast<char *>(std::string("option1").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("option2").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("option3").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("option4").c_str()),
                        .style = TEXT,
                    },
                    {
                        .name = const_cast<char *>(std::string("option5").c_str()), // FIXME: segfault when less than 4 option
                        .style = TEXT,
                    },
                },
            },
            {
                .name = const_cast<char *>(std::string("text1 je hustej").c_str()),
                .style = TEXT,
            },
            {
                .name = const_cast<char *>(std::string("text2 neni ani trochu").c_str()),
                .style = TEXT,
            },
            {
                .name = const_cast<char *>(std::string("text3 mozna malinko je").c_str()),
                .style = TEXT,
            },
            {
                .name = const_cast<char *>(std::string("text4 skoro jako tvoje mama").c_str()),
                .style = TEXT,
            },
            {
                .name = const_cast<char *>(std::string("text5 zkratka").c_str()),
                .style = TEXT,
            },
        }};

    int16_t &brightness_val = root.nested_menu_options[2].value.num;

    // create fifo in temporary folder
    if (mkfifo("/tmp/laser_projector.fifo", S_IRWXU) != 0) 
        perror("mkfifo() error");

    menu_interact(lcd, root, true);
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder
        menu_interact(lcd, root);
        lcd_backlight_dim(lcd, (float)brightness_val / 100.f);

        // separate root menu element for drawing
        // maybe some placeholders to be replaced in the definition
        // .name = "%name%"
        // 
        // menu_interact(lcd, playing_root);
    }

    lcd_backlight_off(lcd);

    return 0;
}
