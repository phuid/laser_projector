#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <wiringPi.h>
#include <iostream>

#include <sys/stat.h>
#include <errno.h>

#include "soft_lcd.h"

#define DEBUG

constexpr uint8_t ENCODER_PINS[2] = {25, 27};
constexpr uint8_t ENCODER_BUTTON_PIN = 23;
#include "encoder.hpp"

constexpr uint8_t SCREEN_HEIGHT = 4;
constexpr uint8_t SCREEN_WIDTH = 20;
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

    const menu_option root = {
        .nested_menu_options = {
            {
                .name = const_cast<char *> "nest",
                .style = NESTED_MENU,
                .nested_menu_options = {
                    {
                        .name = (char *)"text",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"text1",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"text2",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"text3",
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
                .name = (char *)"brightness",
                .style = VALUE,
                .value = {50, 0, 100},
            },
            {
                .name = (char *)"val2",
                .style = VALUE,
                .value = {50, INT16_MIN, INT16_MAX},
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
                        .name = (char *)"option1",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"option2",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"option3",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"option4",
                        .style = TEXT,
                    },
                    {
                        .name = (char *)"option5", // FIXME: segfault when less than 4 options
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
        }};

    int16_t &brightness_val = root.nested_menu_options[2].value.num;

    // create fifo in temporary folder
    if (mkfifo("/tmp/laser_projector.fifo", S_IRWXU) != 0) 
        perror("mkfifo() error");

    menu_interact(lcd, &root, true);
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder
        menu_interact(lcd, root);
        lcd_backlight_dim(lcd, (float)brightness_val / 100.f);

        // read instruction(s) from other runtimes
        // via FIFO
        // else
        // {
        //     if ((rfd = open(fn, O_RDONLY | O_NONBLOCK)) < 0)
        //         perror("open() error for read end");
        //     else
        //     {
        //         if ((wfd = open(fn, O_WRONLY)) < 0)
        //             perror("open() error for write end");
        //         else
        //         {
        //             if (write(wfd, out, strlen(out) + 1) == -1)
        //                 perror("write() error");
        //             else if (read(rfd, in, sizeof(in)) == -1)
        //                 perror("read() error");
        //             else
        //                 printf("read '%s' from the FIFO\n", in);
        //             close(wfd);
        //         }
        //         close(rfd);
        //     }
        //     unlink(fn);
        // }
    }

    lcd_backlight_off(lcd);

    return 0;
}
