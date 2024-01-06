#include <vector>
#include <stdint.h>
#include "encoder.hpp"
// #include <string.h>
// debug
#include <iostream>
#include <stdio.h>
#include <string>
#include "config.hpp"

#include "zmq.hpp"

static void send_command(zmq::socket_t &publisher, std::string message_string)
{
  zmq::message_t msg;
  msg.rebuild(message_string);
  publisher.send(msg, zmq::send_flags::none);
}

template <typename T>
uint8_t num_digits(T n)
{
  T r = 1;
  if (n < 0)
  {
    r++;
    n *= -1;
  }
  while (n > 9)
  {
    n /= 10;
    r++;
  }
  return r;
}

char parent_char[] = {
    0b11111,
    0b11011,
    0b10001,
    0b00000,
    0b11011,
    0b11011,
    0b11000,
    0b11111,
};
#define PARENT_CHAR_NUM 1
char inverted_space_char[] = {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
};
#define INVERTED_SPACE_CHAR_NUM 2
char inverted_pointer_char[] = {
    0b10111,
    0b11011,
    0b11101,
    0b11110,
    0b11101,
    0b11011,
    0b10111,
    0b00000,
};
#define INVERTED_POINTER_CHAR_NUM 3

template <typename T>
struct menu_val
{
  T num;
  T min;
  T max;
  T change_by = 1;
};

template <typename T>
bool change_val(T &val, T val_min = 0, T val_max = 100, T change_by = 1) // return 1 if changed
{
  if (get_encoder_pos() > 0)
  {
#ifdef DEBUG
    std::cout << "change_val: " << get_encoder_pos() * change_by << " <= " << (int)val_max << " - " << (int)val << std::endl;
#endif
    if (get_encoder_pos() * change_by <= val_max - val)
      val += get_encoder_pos() * change_by;
    else
    {
#ifdef DEBUG
      std::cout << "!" << std::endl;
#endif
      val = val_max;
    }
  }
  else if (get_encoder_pos() < 0)
  {
#ifdef DEBUG
    std::cout << "change_val2: " << -1 * get_encoder_pos() * change_by << " <= " << (int)val << " - " << (int)val_min << std::endl;
#endif
    if (-1 * get_encoder_pos() * change_by <= val - val_min)
      val += get_encoder_pos() * change_by;
    else
    {
#ifdef DEBUG
      std::cout << "!" << std::endl;
#endif
      val = val_min;
    }
  }
  else
  {
    return 0;
  }
  set_encoder_pos(0);
  return 1;
}

enum menu_option_style
{
  NESTED_MENU = 0,
  VALUE,
  SELECTION, // all childrem are style text, nest_selected is written to value after button is pressed
  TEXT,
  ROOT_MENU,
  UNDEFINED // TODO: check for undef
};

struct menu_option
{
  std::string name;
  menu_option_style style = UNDEFINED;

  std::vector<menu_option> nested_menu_options = {}; // does this work????????
  uint8_t nest_selected = 0;                         // so that back button is possible
  uint8_t nest_scroll = 0;
  bool nest_option_active = 0;
  bool redraw = 0;

  menu_val<float> value;

  bool has_function = 0;
  void (*function)(zmq::socket_t &, menu_option &);
};

#ifdef DEBUG
std::string dbg_nests = "";
#endif

// return 1 if back out of nest (pressed back option)
bool menu_interact(lcd_t *lcd, zmq::socket_t &command_sender, menu_option &parent_menu_option, bool redraw = 0)
{
  // TODO: use menu_option.redraw instead of redraw
  std::vector<menu_option> &menu = parent_menu_option.nested_menu_options;
  uint8_t &menu_selected = parent_menu_option.nest_selected;
  uint8_t &menu_scroll = parent_menu_option.nest_scroll;

#ifdef DEBUG
  if (redraw)
  {
    std::cout << "wahoo" << std::endl;
    std::cout << "menu-size" << menu.size() << std::endl;
    std::cout << "menu_scroll" << (int)menu_scroll << std::endl;
    std::cout << "menu_select" << (int)menu_selected << std::endl;
    std::cout << "parent_menu_opti_ac" << (int)parent_menu_option.nest_option_active << std::endl;
    std::cout << "encbtnpressed" << (int)get_encoder_btn_pressed() << std::endl;
    std::cout << "isvalue" << (int)(menu[menu_selected].style == VALUE) << std::endl;
    std::cout << "wahoo" << std::endl;
  }
#endif

  // #ifdef DEBUG
  // std::cout << menu[menu_selected].name << " " << (int)menu_scroll << " " << (int)parent_menu_option.nest_option_active << std::endl;
  // #endif
  if (parent_menu_option.nest_option_active && menu[menu_selected].style != VALUE)
  {
    switch (menu[menu_selected].style)
    {
    // go down a layer of options
    case NESTED_MENU:
    case SELECTION:
#ifdef DEBUG
      dbg_nests += menu[menu_selected].name;
#endif
      if (menu_interact(lcd, command_sender, menu[menu_selected], redraw))
      {
        parent_menu_option.nest_option_active = 0; //FIXME: does this ever get called?
        menu_interact(lcd, command_sender, parent_menu_option, true); // redraw
      }
      break;

      // VALUE needs to redraw the screen even whe nselected
      // FUNCTION handled onclick

    default:
      std::cerr << "ERR: Option which cannot be active is active. dbg_nests: \"" << dbg_nests << "\", style: " << menu[menu_selected].style << std::endl;
      break;
    }
  }
  else // parentmenu active && style != value
  {
    bool selection_scrolled = 0;
    bool screen_scrolled = 0;

    if (menu[menu_selected].style == VALUE && parent_menu_option.nest_option_active)
    {
      if (get_encoder_btn_pressed())
      {
        parent_menu_option.nest_option_active = 0;
        clear_encoder_btn_pressed();
        set_encoder_pos(0);
        menu_interact(lcd, command_sender, parent_menu_option, true); // redraw
        // TODO: add return here and everywhere where this function is called
      }
      else
      {
        selection_scrolled = 0;
        menu[menu_selected].redraw = change_val<decltype(menu[menu_selected].value.num)>(menu[menu_selected].value.num, menu[menu_selected].value.min, menu[menu_selected].value.max, menu[menu_selected].value.change_by) || menu[menu_selected].redraw;
        if (menu[menu_selected].redraw)
        {
#ifdef DEBUG
          std::cout << "changeval" << std::endl;
#endif
          send_command(command_sender, "OPTION write " + std::string(menu[menu_selected].name) + " " + std::to_string(menu[menu_selected].value.num));
          }
      }
    }
#ifdef DEBUG
    // std::cout << "c" << std::endl;
#endif
    else
    {
#ifdef DEBUG
      uint8_t prev = menu_selected;
#endif
      if (parent_menu_option.style == NESTED_MENU)
        menu_selected += 1; // cancel accounting for back button in menu so that min/max values work
      selection_scrolled = change_val<uint8_t>(menu_selected, 0, (uint8_t)menu.size() - (parent_menu_option.style != NESTED_MENU));
      if (parent_menu_option.style == NESTED_MENU)
        menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
#ifdef DEBUG
      if (selection_scrolled)
      {
        std::cout << "sel" << (int)menu_selected << "prev" << (int)prev << std::endl;
        std::cout << "selScrollED" << std::endl;
      }
#endif
    }
    if (selection_scrolled || get_encoder_btn_pressed() || redraw || parent_menu_option.redraw || menu[menu_selected].redraw)
    {
#ifdef DEBUG
      std::cout << "dgb_nests: " << dbg_nests << std::endl;
      std::cout << "reason (SCR/BTN/RE/parRE)" << selection_scrolled << get_encoder_btn_pressed() << redraw << parent_menu_option.redraw << std::endl;
#endif
      if (selection_scrolled || redraw || parent_menu_option.redraw || menu[menu_selected].redraw) // FIXME: screen_scrolled flag 1 when menu_selected == 0/max
      {
#ifdef DEBUG
        std::cout << "screenscroll: " << (int)menu_selected << " >= " << (int)menu_scroll << " + " << SCREEN_HEIGHT - 2 << std::endl;
#endif
        if (parent_menu_option.style == NESTED_MENU)
          menu_selected += 1; // cancel accounting for back button in menu

        // handle scroll - prolly totally wrong :skull:
        if (menu.size() + ((parent_menu_option.style == NESTED_MENU) ? 1 : 0) > SCREEN_HEIGHT)
        {
          if (menu_selected > menu_scroll + SCREEN_HEIGHT - 2)
          {
            if (menu_selected >= menu.size() - 1)
              menu_scroll = menu.size() - (SCREEN_HEIGHT - 1);
            else
              menu_scroll = menu_selected - (SCREEN_HEIGHT - 2);
            screen_scrolled = 1;
          }
          else if (menu_selected <= menu_scroll)
          {
            if (menu_selected == 0)
              menu_scroll = 0;
            else
              menu_scroll = menu_selected - 1;
            screen_scrolled = 1;
          }
        }
        else
          menu_scroll = 0;

#ifdef DEBUG
        std::cout << "screen_scrolled:" << screen_scrolled << std::endl;
#endif

        if (parent_menu_option.style == NESTED_MENU)
          menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
      }

      if (get_encoder_btn_pressed())
      {
        clear_encoder_btn_pressed();
        // handle button
        // TODO: handle back button
        menu_selected += 1; // cancel accounting for back button in menu
        if (menu_selected == 0 && parent_menu_option.style == NESTED_MENU)
        {
          parent_menu_option.nest_option_active = 0;
          return 1;
          menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children) //FIXME: this was behind the return - wouldn't run
        }
        else if (parent_menu_option.style == SELECTION)
        {
          parent_menu_option.value.num = (int16_t)menu_selected;
          menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children) //FIXME: this was behind the return - wouldn't run
          return 1;           // FIXME: temporary solution for crash, actual problem is somewhere in scroll calculation / drawing trying to view empty slots
        }

        else
        {
          menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
          if (parent_menu_option.nest_option_active)
          {
            parent_menu_option.nest_option_active = 0;
          }
          else
          {
            switch (menu[menu_selected].style)
            {
            // go down a layer of options
            case NESTED_MENU:
            case SELECTION:
            case VALUE:

#ifdef DEBUG
              std::cout << "a" << std::endl;
#endif
              parent_menu_option.nest_option_active = 1;
              menu[menu_selected].nest_selected = 0;
#ifdef DEBUG
              std::cout << "j" << std::endl;
#endif

            default:
              if (menu[menu_selected].has_function)
              {
                // TODO: handle function menu actions
                menu[menu_selected].function(command_sender, parent_menu_option);
              }
              lcd_clear(lcd);
              menu_interact(lcd, command_sender, parent_menu_option, true); // redraw
              return 0;
              break;
            }
          }
        }
      }
      // draw
      // if (screen_scrolled || redraw)
      //   lcd_clear(lcd);
#ifdef DEBUG
      std::cout << "menu-size" << menu.size() << std::endl;
      std::cout << "menu_scroll" << (int)menu_scroll << std::endl;
      std::cout << "menu_select" << (int)menu_selected << std::endl;
      std::cout << "parent_menu_opti_ac" << (int)parent_menu_option.nest_option_active << std::endl;
#endif
#ifdef DEBUG
      std::cout << "redraw:" << (int)redraw << " parent_redraw:" << (int)parent_menu_option.redraw<< " selected_redraw:" << (int)menu[menu_selected].redraw << " screenscrollED:" << screen_scrolled << std::endl;
#endif
      if (parent_menu_option.style == NESTED_MENU)
        menu_selected += 1; // cancel accounting for back button in menu
      for (uint8_t i = menu_scroll; i < menu_scroll + SCREEN_HEIGHT; i++)
      {
        lcd_pos(lcd, i - menu_scroll, 0);
        if (i < menu.size() + ((parent_menu_option.style == NESTED_MENU) ? 1 : 0)) //+1 for back button
        {
          if (i == menu_selected && !(parent_menu_option.nest_option_active && menu[i - ((parent_menu_option.style == NESTED_MENU) ? 1 : 0)].style == VALUE))
            lcd_print(lcd, (char *)"\3");
          else
            lcd_print(lcd, (char *)" ");

          if (screen_scrolled || redraw || parent_menu_option.redraw || menu[i - ((parent_menu_option.style == NESTED_MENU) ? 1 : 0)].redraw)
          {
            parent_menu_option.redraw = 0;
            menu[i - ((parent_menu_option.style == NESTED_MENU) ? 1 : 0)].redraw = 0;
#ifdef DEBUG
            std::cout << "i" << (int)i << std::endl;
#endif
            if (i == 0 && parent_menu_option.style == NESTED_MENU)
            {
              lcd_printf(lcd, "%*s", SCREEN_WIDTH - 1, "\1back\2\2\2\2\2\2\2\2\2\2\2\2");
#ifdef DEBUG
              printf("-%*s-\n", SCREEN_WIDTH - 1, "BAAAAAAAAAAADHFASDHKCK");
#endif
            }
            else
            {
              if (parent_menu_option.style == NESTED_MENU)
                i -= 1; // account for back button in menu again (bare value can be used when pointing to children)
              switch (menu[i].style)
              {
              case SELECTION:
              case VALUE:
              {
                uint8_t num_len = 4;                      // num_digits<decltype(menu[i].value.num)>(menu[i].value.num);
                uint8_t name_len = menu[i].name.length(); // TODO: rewrite names into std::String and only convert them to cstrings here
#ifdef DEBUG
                std::cout << "i" << (int)i << std::endl;
#endif
                lcd_printf(lcd, "%.*s%*s%c%.*f", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 3 /* 2 = the ">"/" "/"." chars */, menu[i].name.c_str(), /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 3) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 3) - num_len - 3 : SCREEN_WIDTH - name_len - num_len - 3), /*fill_spaces*/ "", ((parent_menu_option.nest_option_active && (i + ((parent_menu_option.style == NESTED_MENU) ? 1 : 0) == menu_selected)) ? '\3' : ' '), num_len - 2, menu[i].value.num);
#ifdef DEBUG
                printf("\"%.*s%*s%c%.*f\"\n", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 3 /* 2 = the ">"/" "/"." chars */, menu[i].name.c_str(), /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 3) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 3) - num_len - 3 : SCREEN_WIDTH - name_len - num_len - 3), /*fill_spaces*/ "", ((parent_menu_option.nest_option_active && (i + ((parent_menu_option.style == NESTED_MENU) ? 1 : 0) == menu_selected)) ? '>' : ' '), num_len - 2, menu[i].value.num);
#endif
                break;
              }
              default:
              {
                uint8_t name_len = menu[i].name.length();
#ifdef DEBUG
                std::cout << "i" << (int)i << std::endl;
#endif
                lcd_printf(lcd, "%.*s%*s", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, menu[i].name.c_str(), /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ "");
#ifdef DEBUG
                printf("\"%.*s%*s\"\n", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, menu[i].name.c_str(), /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ "");
#endif
                break;
              }
              }

              if (parent_menu_option.style == NESTED_MENU)
                i += 1; // cancel accounting for back button in menu
            }
          }
        }
        else
        {
          if (screen_scrolled || redraw || parent_menu_option.redraw)
          {
#ifdef DEBUG
            std::cout << "i" << (int)i << std::endl;
#endif
            lcd_printf(lcd, "%*s", SCREEN_WIDTH, "");
#ifdef DEBUG
            std::cout << "-empty-" << std::endl;
#endif
          }
        }
      }
      if (parent_menu_option.style == NESTED_MENU)
        menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)

#ifdef DEBUG
      std::cout << std::endl;
#endif
    }
  }
#ifdef DEBUG
  dbg_nests = "";
#endif
  return 0;
}