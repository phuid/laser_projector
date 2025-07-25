#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <iostream>

#include <lgpio.h>

#include "zmq.hpp"
#include "my_zmq_helper.hpp"

#include "soft_lcd.h"

#include "config.hpp"
#include "encoder.hpp"
#include "menu.hpp"

#include <filesystem>

constexpr int GPIO_CHIP_NUM = 4;

void wifiman_send(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent) {
    send_command(wifiman_sender, parent.nested_menu_options[parent.nest_selected].command_name);
}

void send_option_command(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent)
{
    send_command(command_sender, parent.nested_menu_options[parent.nest_selected].command_name);
    parent.nest_option_active = 0;
}

void shutdown(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent)
{
    system("poweroff");
}

void read_options(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent)
{
    for (auto &&option : parent.nested_menu_options[parent.nest_selected].nested_menu_options)
    {
        send_command(command_sender, "OPTION read " + option.command_name);
    }
}

// return 1 on error - couldnt set option
bool set_option(std::string name, float value, std::vector<menu_option> options)
{
    for (auto &&option : options)
    {
        if (option.name == name)
        {
            option.value.num = value;
            return 0;
        }
    }
    return 1;
}

void project(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent)
{
    std::string path = std::filesystem::current_path().generic_string() + "/../ild" + parent.nested_menu_options[parent.nest_selected].name;
#ifdef DEBUG
    std::cout << "PROJECT " << path << std::endl;
#endif
    send_command(command_sender, "PROJECT " + path);
    std::cout << "project parent: " << parent.name <<std::endl;
    parent.nest_option_active = 0;
    parent.redraw = 1;
}

void fill_with_files(zmq::socket_t &command_sender, zmq::socket_t &wifiman_sender, menu_option &parent)
{
    std::cout << "searching for files" << std::endl;
    std::string dirpath = "../ild";
    parent.nested_menu_options[parent.nest_selected].nested_menu_options.clear();
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
        menu_option tmp = {.name = path, .style = TEXT, .has_function = 1, .function = project};
        parent.nested_menu_options[parent.nest_selected].nested_menu_options.push_back(tmp);
    }
}

class Command
{
public:
    std::string received_string;
    std::string first_word;
    std::vector<std::string> args;

    void parse(std::string string);
    void execute(std::string string, zmq::socket_t &subscriber, menu_option &root, lcd_t *lcd);
};

void Command::parse(std::string string)
{
    args.clear();
    this->received_string = string;

    size_t space_pos = string.find(" ");
    if (space_pos == std::string::npos)
        this->first_word = string;
    else
    {
        this->first_word = string.substr(0, space_pos);
        string = string.substr(space_pos + 1);
    }

    if (space_pos != std::string::npos)
    {
        space_pos = string.find(" ");
        size_t init_pos = 0;
        while (space_pos != std::string::npos)
        {
            this->args.push_back(string.substr(init_pos, space_pos - init_pos));
            init_pos = space_pos + 1;

            space_pos = string.find(" ", init_pos);
        }
        this->args.push_back(string.substr(init_pos, std::min(space_pos, string.size()) - init_pos + 1));
    }

    std::cout << "string: " << this->received_string << std::endl;
    std::cout << "parsed - firstWord: " << this->first_word << "; "
              << "args: ";
    for (auto &&i : args)
    {
        std::cout << "\"" << i << "\", ";
    }
    std::cout << std::endl;
}

void Command::execute(std::string string, zmq::socket_t &subscriber, menu_option &root, lcd_t *lcd)
{
    this->parse(string);
    std::cout << "executing" << std::endl;

    if (this->first_word == "INFO:")
    {
        if (this->args.size() >= 1)
        {
            if (this->args[0] == "FRAME")
            {
                if (this->args.size() >= 4)
                {
                    std::cout << "ROOT OPTION ACTIVE: " << root.nest_option_active << std::endl;
                    if (!(root.nest_option_active && (root.nest_selected == 0)))
                    {
                        root.nested_menu_options[0].value.num = (stof(this->args[1]) / stof(this->args[3])) * 100.f;
                        root.nested_menu_options[0].redraw = 1;
                    }
                    if (!(root.nest_option_active && (root.nest_selected == 1)))
                    {
                        root.nested_menu_options[1].value.num = stof(this->args[1]);
                        root.nested_menu_options[1].redraw = 1;
                    }
                    root.nested_menu_options[1].value.max = stof(this->args[3]);
                }
            }
            else if (this->args[0] == "PAUSE")
            {
                if (this->args.size() >= 2)
                {
                    root.nested_menu_options[5].name = ((stoi(this->args[1])) ? "PAUSE ||" : "PAUSE >");
                    root.nested_menu_options[5].redraw = 1;
                }
            }
            else if (this->args[0] == "PROJECT")
            {
                if (this->args.size() >= 2)
                {
                    std::string filename_noextension = this->args[1];
                    size_t delimeter_pos = filename_noextension.find('/');
                    while (delimeter_pos != std::string::npos)
                    {
                        filename_noextension = filename_noextension.substr(delimeter_pos + 1);
                        delimeter_pos = filename_noextension.find('/');
                    }
                    root.nested_menu_options[0].name = filename_noextension + "%";
                    root.nested_menu_options[0].redraw = 1;

                    root.nested_menu_options[5].name = "PAUSE >";
                }
            }
            else if (this->args[0] == "OPTION")
            {
                if (this->args.size() >= 3)
                {
                    if (this->args[1] == "battery_voltage") {
                        for (auto &&option : root.nested_menu_options) {
                            if (option.name == "battery voltage") {
                                option.value.num = stof(this->args[2]);
                                root.redraw = 1;
                            }
                        }
                    }

                    for (auto &&nest : root.nested_menu_options) {
                        if (nest.name == "options"){
                            for (auto &&option : nest.nested_menu_options)
                            {
                                // std::cout << "ARG: \"" << args[1] << "\"" << ", name: \"" << option.command_name << "\"" << std::endl;
                                if (option.command_name == this->args[1])
                                {
                                    option.value.num = stof(this->args[2]);
                                    nest.redraw = 1;
                                }
                            }
                        }
                    }
                }
            }
            else if (this->args[0] == "STOP") {
                root.nested_menu_options[5].name = "PAUSE";
            }
            root.nested_menu_options[2].name = "I:";
            for (auto &&i : this->args)
            {
                root.nested_menu_options[2].name += i + " ";
            }
            root.nested_menu_options[2].redraw = 1;
            std::cout << root.nested_menu_options[2].name << std::endl;
        }
        else
        {
            std::cout << "!!empty INFO received" << std::endl;
        }
    }
    else if (this->first_word == "ERROR:")
    {
        root.nested_menu_options[2].name = "E:" + this->received_string.substr(std::string("ERROR:").length());
        root.nested_menu_options[2].redraw = 1;
        std::cout << root.nested_menu_options[2].name << std::endl;
    }
    else if (this->first_word == "DISPLAY:") {
        std::string display_string = this->received_string.substr(std::string("DISPLAY:").length());
        zmq::message_t received;
        std::cout << "displaying \"" << display_string << "\"" << std::endl;
        root.redraw = 1;
        while (true) {
            bool ididreceive = 0;
            for (size_t scroll = 0; !get_encoder_btn_pressed(); scroll = (scroll + 1) % (display_string.length() - SCREEN_WIDTH)) {
                if (subscriber.recv(received, zmq::recv_flags::dontwait)){
                    ididreceive = 1;
                    break;
                }
                lcd_clear(lcd);
                lcd_pos(lcd, 0, 0);
                lcd_print(lcd, "press btn to dismiss");
                lcd_pos(lcd, 1, 0);
                lcd_printf(lcd, "%.*s", SCREEN_WIDTH, (display_string.length() < SCREEN_WIDTH) ? display_string.c_str() : display_string.substr(scroll).c_str());
                delay(200); // 0.2s to draw and bin all the other display messages
            }
            if (ididreceive && received.size() > 0) {
                if (received.to_string().substr(0, 8) == "DISPLAY:") {
                    display_string = received.to_string().substr(8);
                }
                else {
                    Command new_command;
                    new_command.execute(received.to_string(), subscriber, root, lcd);
                    break;
                }
            }
            else {
                break;
            }
            std::cout << "ye" << std::endl;
        }
    }
    else if (this->first_word == "ALERT:") // cant be replaced by new command, user has to dismiss it
    {
        std::string display_string = this->received_string.substr(std::string("ALERT:").length());
        zmq::message_t received;
        std::cout << "displaying \"" << display_string << "\"" << std::endl;
        root.redraw = 1;
        for (size_t scroll = 0; !get_encoder_btn_pressed(); scroll = (scroll + 1) % (display_string.length() - SCREEN_WIDTH)) {
            lcd_clear(lcd);
            lcd_pos(lcd, 0, 0);
            lcd_print(lcd, "press btn to dismiss");
            lcd_pos(lcd, 1, 0);
            lcd_printf(lcd, "%.*s", SCREEN_WIDTH, (display_string.length() < SCREEN_WIDTH) ? display_string.c_str() : display_string.substr(scroll).c_str());
            delay(200); // 0.2s to draw and bin all the other display messages
        }
    }
    else
    {
        std::cout << "!!unknown response first word: \"" << this->first_word << "\"" << std::endl;
    }
    std::cout << "done executing" << std::endl;
}

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

    if (wiringPiSetupGpio() < 0) {
        std::cout << "Failed to open gpio chip" << std::endl;
        return 1;
    }
    /* Create a LCD given SCL, SDA and I2C address, 4 lines */
    /* PCF8574 has default address 0x27 */

    lcd_t *lcd = lcd_create(LCD_SCL_PIN, LCD_SDA_PIN, 0x27, SCREEN_HEIGHT);
    for (int8_t i = 10; i >= 0; i++){

    lcd = lcd_create(LCD_SCL_PIN, LCD_SDA_PIN, 0x27, SCREEN_HEIGHT);
    if (lcd != NULL)
    {
        break;
    }
        printf("Cannot set-up LCD. remaining attempts: %d\n", i);
        if (i == 0)
        {
            printf("exit");
        return 1;
        }
        delay(1000);
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
    wiringPiISR(encoder_button_pin, INT_EDGE_RISING, *handle_enc_btn_rising);
    wiringPiISR(encoder_button_pin, INT_EDGE_FALLING, *handle_enc_btn_falling);

    lcd_create_char(lcd, PARENT_CHAR_NUM, parent_char);
    lcd_create_char(lcd, INVERTED_SPACE_CHAR_NUM, inverted_space_char);
    lcd_create_char(lcd, INVERTED_POINTER_CHAR_NUM, inverted_pointer_char);
    lcd_create_char(lcd, UNDER_POINTER_CHAR_NUM, under_pointer_char);

    zmq::context_t ctx(1);

    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5556");
    subscriber.set(zmq::sockopt::subscribe, "");

    zmq::socket_t wifiman_subscriber(ctx, zmq::socket_type::sub);
    wifiman_subscriber.connect("tcp://localhost:5558");
    wifiman_subscriber.set(zmq::sockopt::subscribe, "");

    zmq::message_t received;

    zmq::socket_t command_sender(ctx, zmq::socket_type::pub);
    command_sender.connect("tcp://localhost:5557");

    zmq::socket_t wifiman_sender(ctx, zmq::socket_type::pub);
    wifiman_sender.connect("tcp://localhost:5559");

    menu_option root = {
        .name = "ROOT",
        .style = ROOT_MENU,
        .nested_menu_options = {
            {
                .name = "progress%",
                .command_name = "progress",
                .style = VALUE,
                .value = {0, 0, 100, 0.5},
            },
            {
                .name = "current_frame",
                .command_name = "current_frame",
                .style = VALUE,
                .value = {1, 1, 1, 1},
            },
            {.name = "-no out received-",
             .style = TEXT},
            {
                .name = "battery voltage",
                .style = VALUE,
                .value = {0, 0, 100, 0}
            },
            {.name = "STOP",
             .command_name = "STOP",
             .style = TEXT,
             .has_function = 1,
             .function = send_option_command},
            {.name = "PAUSE", .command_name = "PAUSE", .style = TEXT, .has_function = 1, .function = send_option_command},
            {
                .name = "PROJECT",
                .style = NESTED_MENU,
                .has_function = 1,
                .function = fill_with_files,
            },
            {
                .name = "options",
                .style = NESTED_MENU,
                .nested_menu_options = {
                    {
                        .name = "screen brightness",
                        .command_name = "screen_brightness",
                        .style = VALUE,
                        .value = {50, 0, 100},
                    },
                    {
                        .name = "repeat",
                        .command_name = "repeat",
                        .style = VALUE,
                        .value = {0, 0, 1, 1},
                    },
                    {
                        .name = "point_delay",
                        .command_name = "point_delay",
                        .style = VALUE,
                        .value = {0, 0, 10000, 1},
                    },
                    {
                        .name = "target_frame_time",
                        .command_name = "target_frame_time",
                        .style = VALUE,
                        .value = {0, 0, 10000, 1},
                    },
                    {
                        .name = "trapezoid_horizontal",
                        .command_name = "trapezoid_horizontal",
                        .style = VALUE,
                        .value = {0, -1.f, 1.f, 0.01},
                    },
                    {
                        .name = "trapezoid_vertical",
                        .command_name = "trapezoid_vertical",
                        .style = VALUE,
                        .value = {0, -1.f, 1.f, 0.01},
                    },

{ .name="time_accurate_framing",
.command_name = "time_accurate_framing",
.style = VALUE,
.value = {1, 0.f, 1.f, 1}},
{ .name="scale_x",
.command_name = "scale_x",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="scale_y",
.command_name = "scale_y",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="move_x",
.command_name = "move_x",
.style = VALUE,
.value = {0, -1.f, 1.f, 0.01}},
{ .name="move_y",
.command_name = "move_y",
.style = VALUE,
.value = {0, -1.f, 1.f, 0.01}},
{ .name="laser_brightness",
.command_name = "laser_brightness",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="laser_red_brightness",
.command_name = "laser_red_brightness",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="laser_green_brightness",
.command_name = "laser_green_brightness",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="laser_blue_brightness",
.command_name = "laser_blue_brightness",
.style = VALUE,
.value = {0, 0.f, 1.f, 0.01}},
{ .name="laser_red_br_offset",
.command_name = "laser_red_br_offset",
.style = VALUE,
.value = {0, -255, 255, 1}},
{ .name="laser_green_br_offset",
.command_name = "laser_green_br_offset",
.style = VALUE,
.value = {0, -255, 255, 1}},
{ .name="laser_blue_br_offset",
.command_name = "laser_blue_br_offset",
.style = VALUE,
.value = {0, -255, 255, 1}}

                },
                .has_function = 1,
                .function = read_options,
            },
            {
                .name = "wifi options",
                .command_name = "read",
                .style = NESTED_MENU,
                .nested_menu_options = {
                    {
                        .name = "SET MODE",
                        .command_name = "", // so that wifiman receiving doesnt try to read undefined
                        .style = NESTED_MENU,
                        .nested_menu_options = {
                            {
                                .name = "WiFi OFF",
                                .command_name = "write stealth",
                                .style = TEXT,
                                .has_function = 1,
                                .function = wifiman_send
                            },
                            {
                                .name = "WiFi",
                                .command_name = "write wifi",
                                .style = TEXT,
                                .has_function = 1,
                                .function = wifiman_send
                            },
                            {
                                .name = "Access Point",
                                .command_name = "write hotspot",
                                .style = TEXT,
                                .has_function = 1,
                                .function = wifiman_send
                            },
                        }
                    },
                    {
                        .name = "mode_set:",
                        .style = TEXT,
                    },
                    {
                        .name = "",
                        .command_name = "wifi_setting",
                        .style = TEXT,
                    },
                    {
                        .name = "status:",
                        .style = TEXT,
                    },
                    {
                        .name = "",
                        .command_name = "mode",
                        .style = TEXT,
                    },
                    {
                        .name = "SSID:",
                        .style = TEXT,
                    },
                    {
                        .name = "",
                        .command_name = "wifi_ssid",
                        .style = TEXT,
                    },
                    {
                        .name = "hostname:",
                        .style = TEXT,
                    },
                    {
                        .name = "",
                        .command_name = "hostname",
                        .style = TEXT,
                    }

                },
                .has_function = 1,
                .function = wifiman_send,
            },
            {
                .name = "SOFT SHUTDOWN",
                .command_name = "SHUTDOWN",
                .style = TEXT,
                .has_function = 1,
                .function = send_option_command
            },
            {
                .name = "HARD SHUTDOWN",
                .style = TEXT,
                .has_function = 1,
                .function = shutdown
            },
            }
            };

    float &brightness_val = [](menu_option &root) -> float&
    {
        for (auto &&nest : root.nested_menu_options) {
            if (nest.name == "options") {
                for (auto &&option : nest.nested_menu_options) {
                    if (option.command_name == "screen_brightness") {
                        return option.value.num;
                    }
                }
            }
        }
        std::cerr << "Error: no screen birghtness option found" << std::endl;
        exit(1);
    } (root);
    std::vector<menu_option> &wifiman_options_vector = [](menu_option &root) -> std::vector<menu_option>&
    {
        for (auto &&nest : root.nested_menu_options) {
            if (nest.name == "wifi options") {
                return nest.nested_menu_options;
            }
        }
        std::cerr << "Error: no wifi options menu found" << std::endl;
        exit(1);
    } (root);

    
    size_t last_bat_read = millis();
    send_command(command_sender, "OPTION read battery_voltage");

    bool first_redraw = 1;
    std::cout << "bef" << std::endl;
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder

        lcd_backlight_dim(lcd, brightness_val / 100.f);

        while (subscriber.recv(received, zmq::recv_flags::dontwait))
        {
            Command command;
            command.execute(received.to_string(), subscriber, root, lcd);
        }
        while (wifiman_subscriber.recv(received, zmq::recv_flags::dontwait))
        {
            std::string rec_str = received.to_string();

            size_t col_pos = rec_str.find(":");
            if (col_pos != std::string::npos) {
                std::string first = rec_str.substr(0, col_pos);

                if (rec_str.length() > col_pos + 2) {
                    std::string rest = rec_str.substr(col_pos + 2);
                    std::cout << "first:\"" << first << "\", rest:\"" << rest << "\"" << std::endl;
                    for (auto &&opt : wifiman_options_vector) {
                        if (opt.command_name == first) {
                            opt.name = "\4" + rest;
                        }
                    }
                }
            }
        }

        menu_interact(lcd, command_sender, wifiman_sender, root, first_redraw);
        first_redraw = 0;

        if (millis() > last_bat_read + 10000) {
            last_bat_read = millis();
            send_command(command_sender, "OPTION read battery_voltage");
        }
        usleep(1000);
    }

    lcd_backlight_off(lcd);

    return 0;
}
