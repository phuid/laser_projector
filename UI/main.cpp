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

void print_test(zmq::socket_t &command_sender, menu_option &parent)
{
    send_command(command_sender, parent.nested_menu_options[parent.nest_selected].name);
    parent.nest_option_active = 0;
}

void read_options(zmq::socket_t &command_sender, menu_option &parent)
{
    for (auto &&option : parent.nested_menu_options[parent.nest_selected].nested_menu_options)
    {
        send_command(command_sender, "OPTION read " + option.name);
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

void project(zmq::socket_t &command_sender, menu_option &parent)
{
    std::string path = std::filesystem::current_path().generic_string() + "/../ild" + parent.nested_menu_options[parent.nest_selected].name;
#ifdef DEBUG
    std::cout << "PROJECT " << path << std::endl;
#endif
    send_command(command_sender, "PROJECT " + path);
    parent.nest_option_active = 0;
}

void fill_with_files(zmq::socket_t &command_sender, menu_option &parent)
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
    int execute(std::string string, menu_option &root);
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

    std::cout << "parsed - firstWord: " << this->first_word << "; "
              << "args: ";
    for (auto &&i : args)
    {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
}

int Command::execute(std::string string, menu_option &root)
{
    this->parse(string);

    if (this->first_word == "INFO:")
    {
        if (this->args.size() >= 1)
        {
            if (this->args[0] == "POS")
            {
                if (this->args.size() >= 4)
                    if (!(root.nest_option_active && root.nest_selected == 0))
                    {
                        root.nested_menu_options[0].value.num = (stof(this->args[1]) / stof(this->args[3])) * 100.f;
                        root.nested_menu_options[0].redraw = 1;
#ifdef DEBUG
                        std::cout << "set-";
#endif
                    }
#ifdef DEBUG
                std::cout << "progress::" << static_cast<float>(stoi(this->args[1]) / stoi(this->args[3])) * 100.f << std::endl;
#endif
            }
            else if (this->args[0] == "STOP")
            {
            }
            else if (this->args[0] == "PAUSE")
            {
                if (this->args.size() >= 2) {
                    (this->args[1] ? "||" : ">")
                }
            }
            else if (this->args[0] == "PROJECT")
            {
                if (this->args.size() >= 2) {
                    this->args[1]
                }
            }
            else if (this->args[0] == "OPTION")
            {
                if (this->args.size() >= 3) {
                    if (this->args[2] == "point_delay") {

                    }
                    else if (this->args[2] == "repeat") {

                    }
                    else if (this->args[2] == "target_frame_time") {

                    }
                    else if (this->args[2] == "trapezoid_horizontal") {

                    }
                    else if (this->args[2] == "trapezoid vertical") {

                    }
                    else {
                        exit(0); // FIXME: only for debug, REMOVE
                    }
                }
            }
            else
            {
                exit(0); // FIXME: only for debug, REMOVE
            }
        }
        else
        {
            exit(0); // FIXME: only for debug, REMOVE
        }
    }
    else
    {
        std::cout << "first word: \"" << this->first_word << "\"" << std::endl;
        exit(0); // FIXME: only for debug, REMOVE
    }
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

    zmq::message_t received;

    zmq::socket_t command_sender(ctx, zmq::socket_type::pub);
    command_sender.connect("tcp://localhost:5557");

    menu_option root = {
        .name = "ROOT",
        .style = ROOT_MENU,
        .nested_menu_options = {
            {
                .name = "progress%",
                .style = VALUE,
                .value = {0, 0, 100, 0.5},
            },
            {.name = "-no out received-",
             .style = TEXT},
            {.name = "STOP",
             .style = TEXT},
            {.name = "PAUSE", .style = TEXT, .has_function = 1, .function = print_test},
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
                        .style = VALUE,
                        .value = {50, 0, 100},
                    },
                    {
                        .name = "repeat",
                        .style = VALUE,
                        .value = {0, 0, 1, 1},
                    },
                    {
                        .name = "point_delay",
                        .style = VALUE,
                        .value = {0, 0, 10000, 10},
                    },
                    {
                        .name = "target_frame_time",
                        .style = VALUE,
                        .value = {0, 0, 10000, 1},
                    },
                    {
                        .name = "trapezoid_horizontal",
                        .style = VALUE,
                        .value = {0, -1.f, 1.f, 0.05},
                    },
                    {
                        .name = "trapezoid_vertical",
                        .style = VALUE,
                        .value = {0, -1.f, 1.f, 0.05},
                    },
                },
                .has_function = 1,
                .function = read_options,
            }}};

    float &brightness_val = root.nested_menu_options[5].nested_menu_options[0].value.num;

    bool first_redraw = 1;
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder

        lcd_backlight_dim(lcd, brightness_val / 100);

        subscriber.recv(received, zmq::recv_flags::dontwait);
        while (received.size() > 0)
        {
            Command command;
            command.execute(received.to_string(), root);

            subscriber.recv(received, zmq::recv_flags::dontwait);
        }

        menu_interact(lcd, command_sender, root, first_redraw);
        first_redraw = 0;
    }

    lcd_backlight_off(lcd);

    return 0;
}
