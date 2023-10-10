#include <vector>
#include <stdint.h>
#include <iostream>

char parent_char[] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00111,
    0b00000,
};
#define PARENT_CHAR_NUM 1

template <typename T>
struct menu_val
{
  T num;
  T min;
  T max;
};

template <typename T>
bool change_val(T *val, T val_min = 0, T val_max = 100) // return 1 if changed
{
  if (encoder_pos > 0)
  {
    if (encoder_pos < val_max - *val)
      *val += encoder_pos;
    else
      *val = val_max;
  }
  else if (encoder_pos < 0)
  {
    std::cout << -1*encoder_pos << ">" << (int)*val << "-" << (int)val_min << std::endl;
    if (-1 * encoder_pos <= *val - val_min)
      *val += encoder_pos;
    else
      *val = val_min;
  }
  else
  {
    return 0;
  }
  encoder_pos = 0;
  return 1;
}

enum menu_option_style
{
  NESTED_MENU = 0,
  VALUE,
  SELECTION, // all childrem are style text, nest_selected is written to value after button is pressed
  FUNCTION,
  TEXT,
  ROOT_MENU,
};

struct menu_option
{
  char *name;
  menu_option_style style;

  std::vector<menu_option> nested_menu_options; // does this work????????
  uint8_t nest_selected = 0;
  uint8_t nest_scroll = 0;
  bool nest_option_active = 0;

  menu_val<int16_t> value;

  void (*function)(void);
};

void print_test() // TODO: remove print_test
{
  std::cout << "print test" << std::endl;
}

void menu_interact(lcd_t *lcd, std::vector<menu_option> *menu, uint8_t *menu_selected, uint8_t *menu_scroll, bool *parent_menu_option_active, menu_option_style parent_menu_style, bool redraw = 0)
{
  // std::cout << (*menu)[*menu_selected].name << " " << (int)*menu_scroll << " " << (int)*parent_menu_option_active << std::endl;
  // nest_selected + 1 so that back option being first in the list is possible
  uint8_t temp_menu_selected = *menu_selected + 1;
  if (*parent_menu_option_active && (*menu)[*menu_selected].style != VALUE)
  {
    switch ((*menu)[*menu_selected].style)
    {
    // go down a layer of options
    case NESTED_MENU:
    case SELECTION:
      menu_interact(lcd, &(*menu)[*menu_selected].nested_menu_options, &(*menu)[*menu_selected].nest_selected, &(*menu)[*menu_selected].nest_scroll, &(*menu)[*menu_selected].nest_option_active, (*menu)[*menu_selected].style, redraw);
      break;

    default:
      break;
    }
  }
  else
  {
    bool scrolled;
    if (*parent_menu_option_active && (*menu)[*menu_selected].style == VALUE)
    {
      scrolled = 0;
      redraw = change_val<decltype((*menu)[*menu_selected].value.num)>(&(*menu)[*menu_selected].value.num, (*menu)[*menu_selected].value.min, (*menu)[*menu_selected].value.max);
      if (redraw)
      std::cout << "changeval" << std::endl;
    }
    else
    {
      scrolled = change_val<uint8_t>(menu_selected, 0, (uint8_t)menu->size());
      if (scrolled)
      std::cout << "scroll" << std::endl;
    }
    if (scrolled || encoder_btn_pressed || redraw)
    {
      if (scrolled && 0) //FIXME: SCROLL
      {
        // handle scroll - prolly totally wrong :skull:
        if (*menu_selected > (*menu_scroll + 1) && *menu_selected - (*menu_scroll + 1) > SCREEN_HEIGHT / 2)
        {
          *menu_scroll += *menu_selected - (*menu_scroll + 1);
        }
        else if (*menu_selected < (*menu_scroll + 1) && (*menu_scroll + 1) - *menu_selected > SCREEN_HEIGHT / 2)
        {
          *menu_scroll -= (*menu_scroll + 1) - *menu_selected;
        }
      }

      if (encoder_btn_pressed)
      {
        // handle button
        // TODO: handle back button
        switch ((*menu)[*menu_selected].style)
        {
        // go down a layer of options
        case NESTED_MENU:
        case SELECTION:
        case VALUE:
          *parent_menu_option_active = 1;
          menu_interact(lcd, &(*menu)[*menu_selected].nested_menu_options, &(*menu)[*menu_selected].nest_selected, &(*menu)[*menu_selected].nest_scroll, &(*menu)[*menu_selected].nest_option_active, (*menu)[*menu_selected].style, 1);
          return;

        case FUNCTION:
          // TODO: handle function menu actions
          return;

        default:
          return;
        }
      }
      // draw
      lcd_clear(lcd);
      std::cout << "menu-size" << menu->size() << std::endl;
      std::cout << "menu_scroll" << (int)*menu_scroll << std::endl;
      std::cout << "menu_select" << (int)*menu_selected << std::endl;
      std::cout << "parent_menu_opti_ac" << (int)*parent_menu_option_active << std::endl;
      for (uint8_t i = *menu_scroll; i < *menu_scroll + 4; i++)
      {
        if (i == *menu_selected)
          lcd_print(lcd, (char *)">");
        else
          lcd_print(lcd, (char *)" ");

        std::cout << "i" << (int)i << std::endl;
        switch ((*menu)[i].style)
        {
        case VALUE:
        {
          if (parent_menu_option_active && (*menu)[i].style == VALUE)
          {
            lcd_printf(lcd, "%*s>%4d", SCREEN_WIDTH - 5 - 1, (*menu)[i].name, (*menu)[i].value);
          }
          else
          {
            lcd_printf(lcd, "%*s %4d", SCREEN_WIDTH - 5 - 1, (*menu)[i].name, (*menu)[i].value);
          }
          break;
        }
        default:
          lcd_printf(lcd, "%*s", SCREEN_WIDTH - 1, (*menu)[i].name);
          break;
        }
        lcd_print(lcd, (char *)"\n");
      }
    }
  }
}