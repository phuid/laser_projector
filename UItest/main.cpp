#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <wiringPi.h>
#include <iostream>
#include "soft_lcd.h"

#define ENCODER_PINS \
    {                \
        25, 27       \
    }
#define ENCODER_BUTTON_PIN 23
#include "encoder.hpp"

#define SCREEN_HEIGHT 4
#define SCREEN_WIDTH 20
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
    wiringPiISR(encoder_button_pin, INT_EDGE_FALLING, *handle_enc_btn_interrupts);

    lcd_create_char(lcd, PARENT_CHAR_NUM, parent_char);

    std::vector<menu_option> menu = {
        {
            .name = (char *)"nest",
            .style = NESTED_MENU,
            .nested_menu_options = {
                {
                    .name = (char *)"text",
                    .style = TEXT,
                },
                {
                    .name = (char *)"func",
                    .style = FUNCTION,
                    .function = *print_test,
                },
            },
        },
        {
            .name = (char *)"text",
            .style = TEXT,
        },
        {
            .name = (char *)"val - brightnessssssssss",
            .style = VALUE,
            .value = {50, 0, 100},
        },
        {
            .name = (char *)"val2",
            .style = VALUE,
            .value = {50, 0, 100},
        },
        {
            .name = (char *)"value3",
            .style = VALUE,
            .value = {4000, INT16_MIN, INT16_MAX},
        },
        {
            .name = (char *)"value4",
            .style = VALUE,
            .value = {12345, INT16_MIN, INT16_MAX},
        },
        {
            .name = (char *)"func",
            .style = FUNCTION,
            .function = *print_test,
        },
        {
            .name = (char *)"select",
            .style = SELECTION,
            .nested_menu_options = {
                {
                    .name = (char *)"text",
                    .style = TEXT,
                },
                {
                    .name = (char *)"text",
                    .style = TEXT,
                },
                {
                    .name = (char *)"text",
                    .style = TEXT,
                },
            },
        },
        {
            .name = (char *)"text1 je hustej",
            .style = TEXT,
        },
        {
            .name = (char *)"text2 neni ani trochu",
            .style = TEXT,
        },
        {
            .name = (char *)"text3 mozna malinko je",
            .style = TEXT,
        },
        {
            .name = (char *)"text4 skoro jako tvoje mama",
            .style = TEXT,
        },
        {
            .name = (char *)"text5 zkratka",
            .style = TEXT,
        },
    };
    uint8_t menu_selected = 0;
    uint8_t menu_scroll = 0;
    bool menu_option_active = 0; // selected option clicked

    bool redraw = 1;

    while (true)
    {
        //   lcd_pos(lcd, 0, 0);
        menu_interact(lcd, &menu, &menu_selected, &menu_scroll, &menu_option_active, ROOT_MENU, redraw);
        redraw = 0;
        lcd_backlight_dim(lcd, (float)menu[2].value.num / 100.f);
        // lcd_printf(lcd, (char *)"brightness: %d%% ", menu[2].value.num);
    }

    lcd_backlight_off(lcd);

    return 0;
}
