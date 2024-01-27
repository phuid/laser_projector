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

constexpr int GPIO_CHIP_NUM = 0;

void send_option_command(zmq::socket_t &command_sender, menu_option &parent)
{
    send_command(command_sender, parent.nested_menu_options[parent.nest_selected].command_name);
    parent.nest_option_active = 0;
}

void read_options(zmq::socket_t &command_sender, menu_option &parent)
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

void project(zmq::socket_t &command_sender, menu_option &parent)
{
    std::string path = std::filesystem::current_path().generic_string() + "/../ild" + parent.nested_menu_options[parent.nest_selected].name;
#ifdef DEBUG
    std::cout << "PROJECT " << path << std::endl;
#endif
    send_command(command_sender, "PROJECT " + path);
    parent.nest_option_active = 0;
    parent.redraw = 1;
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
    void execute(std::string string, menu_option &root);
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

void Command::execute(std::string string, menu_option &root)
{
    this->parse(string);
    std::cout << "executing" << std::endl;

    if (this->first_word == "INFO:")
    {
        if (this->args.size() >= 1)
        {
            if (this->args[0] == "POS")
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
                    root.nested_menu_options[4].name = ((stoi(this->args[1])) ? "PAUSE ||" : "PAUSE >");
                    root.nested_menu_options[4].redraw = 1;
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

                    root.nested_menu_options[4].name = "PAUSE >";
                }
            }
            else if (this->args[0] == "OPTION")
            {
                if (this->args.size() >= 3)
                {
                    for (auto &&option : root.nested_menu_options[5].nested_menu_options)
                    {
                        if (option.command_name == this->args[2])
                        {
                            option.value.num = stof(this->args[3]);
                        }
                    }
                }
            }
            else if (this->args[0] == "STOP") {
                    root.nested_menu_options[4].name = "PAUSE";
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

    int gpio_chip_handle = lgGpiochipOpen(GPIO_CHIP_NUM);
    if (gpio_chip_handle < 0) {
        std::cout << "Failed to open gpio chip" << std::endl;
        return 1;
    }
    /* Create a LCD given SCL, SDA and I2C address, 4 lines */
    /* PCF8574 has default address 0x27 */
    lcd_t *lcd = lcd_create(gpio_chip_handle, 1, 0x27, SCREEN_HEIGHT);

    if (lcd == NULL)
    {
        printf("Cannot set-up LCD.\n");
        return 1;
    }

    lcd_init(lcd);

    lgGpioClaimInput(gpio_chip_handle, LG_SET_PULL_UP, encoder_pins[0]);
    lgGpioClaimInput(gpio_chip_handle, LG_SET_PULL_UP, encoder_pins[1]);
    lgGpioClaimInput(gpio_chip_handle, LG_SET_PULL_UP, encoder_button_pin);

    lgGpioSetAlertsFunc(gpio_chip_handle, encoder_pins[0], *handle_enc_interrupts, &gpio_chip_handle);
    lgGpioSetAlertsFunc(gpio_chip_handle, encoder_pins[1], *handle_enc_interrupts, &gpio_chip_handle);
    lgGpioSetAlertsFunc(gpio_chip_handle, encoder_button_pin, *handle_enc_btn_interrupts, NULL);

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
                        .value = {0, 0, 10000, 10},
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
                        .value = {0, -1.f, 1.f, 0.05},
                    },
                    {
                        .name = "trapezoid_vertical",
                        .command_name = "trapezoid_vertical",
                        .style = VALUE,
                        .value = {0, -1.f, 1.f, 0.05},
                    },
                },
                .has_function = 1,
                .function = read_options,
            }}};

    float &brightness_val = root.nested_menu_options[6].nested_menu_options[0].value.num;

    bool first_redraw = 1;
    while (true)
    {
        // interact with user via OLED LCD and a rotary encoder

        lcd_backlight_dim(lcd, brightness_val);

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
