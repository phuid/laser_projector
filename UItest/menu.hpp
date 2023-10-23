#include <vector>
#include <stdint.h>
#include "encoder.hpp"
// #include <string.h>
// debug
#include <iostream>
#include <stdio.h>
#include <string>

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
#ifdef DEBUG
    std::cout << encoder_pos << " < " << (int)val_max << " - " << (int)*val << std::endl;
#endif
    if (encoder_pos <= val_max - *val)
      *val += encoder_pos;
    else
    {
#ifdef DEBUG
      std::cout << "!" << std::endl;
#endif
      *val = val_max;
    }
  }
  else if (encoder_pos < 0)
  {
#ifdef DEBUG
    std::cout << -1 * encoder_pos << " <= " << (int)*val << " - " << (int)val_min << std::endl;
#endif
    if (-1 * encoder_pos <= *val - val_min)
      *val += encoder_pos;
    else
    {
#ifdef DEBUG
      std::cout << "!" << std::endl;
#endif
      *val = val_min;
    }
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
  std::cout << "FUNCTION print test WAHOO WAHOO (also both rising and falling interrupts are registered)" << std::endl;
}

std::string dbg_nests = "";

// return 1 if back out of nest (pressed back option)
bool menu_interact(lcd_t *lcd, std::vector<menu_option> *menu, uint8_t *menu_selected, uint8_t *menu_scroll, bool *parent_menu_option_active, menu_option_style parent_menu_style, bool redraw = 0)
{
  // #ifdef DEBUG
  // std::cout << (*menu)[*menu_selected].name << " " << (int)*menu_scroll << " " << (int)*parent_menu_option_active << std::endl;
  // #endif
  // nest_selected + 1 so that back option being first in the list is possible
  uint8_t temp_menu_selected = *menu_selected + 1;
  if (*parent_menu_option_active && (*menu)[*menu_selected].style != VALUE)
  {
    dbg_nests += (*menu)[*menu_selected].name;
    switch ((*menu)[*menu_selected].style)
    {
    // go down a layer of options
    case NESTED_MENU:
    case SELECTION:
      if (menu_interact(lcd, &(*menu)[*menu_selected].nested_menu_options, &(*menu)[*menu_selected].nest_selected, &(*menu)[*menu_selected].nest_scroll, &(*menu)[*menu_selected].nest_option_active, (*menu)[*menu_selected].style, redraw))
      {
        *parent_menu_option_active = 0;
      }
      break;

      // VALUE handled when drawing
      // FUNCTION handled onclick

    default:
      break;
    }
  }
  else
  {
#ifdef DEBUG
    // std::cout << "c" << std::endl;
#endif
    bool selection_scrolled = 0;
    bool screen_scrolled = 0;
    if (parent_menu_style == VALUE && *parent_menu_option_active)
    {
      if (encoder_btn_pressed)
      {
        *parent_menu_option_active = 0;
        encoder_btn_pressed = 0;
        encoder_pos = 0;
      }
      else
      {
#ifdef DEBUG
        std::cout
            << "abouttochangeval" << std::endl;
#endif
        selection_scrolled = 0;
        redraw = change_val<decltype((*menu)[*menu_selected].value.num)>(&(*menu)[*menu_selected].value.num, (*menu)[*menu_selected].value.min, (*menu)[*menu_selected].value.max);
#ifdef DEBUG
        if (redraw)
          std::cout << "changeval" << std::endl;
#endif
      }
    }
    else
    {
      selection_scrolled = change_val<uint8_t>(menu_selected, 0, (uint8_t)menu->size() - 1);
#ifdef DEBUG
      if (selection_scrolled)
        std::cout << "scroll" << std::endl;
#endif
    }
    if (selection_scrolled || encoder_btn_pressed || redraw)
    {
#ifdef DEBUG
      std::cout << "nest: " << dbg_nests << std::endl;
      std::cout << "reason (SCR/BTN/RE)" << selection_scrolled << encoder_btn_pressed << redraw << std::endl;
#endif
      if (selection_scrolled) // FIXME: screen_scrolled flag 1 when menu_selected == 0/max
      {

        // TODO: overwrite leftover chars
        // * find out name string strlen()
        // * print x chars of name string and width-x chars of " "
#ifdef DEBUG
        std::cout << (int)*menu_selected << " > " << (int)*menu_scroll << " + " << SCREEN_HEIGHT - 2 << std::endl;
#endif
        // handle scroll - prolly totally wrong :skull:
        if (*menu_selected >= *menu_scroll + SCREEN_HEIGHT - 2)
        {
          if (*menu_selected == menu->size() - 1)
            *menu_scroll = *menu_selected - (SCREEN_HEIGHT - 1);
          else
            *menu_scroll = *menu_selected - (SCREEN_HEIGHT - 2);
          screen_scrolled = 1;
        }
        else if (*menu_selected <= *menu_scroll)
        {
          if (*menu_selected == 0)
            *menu_scroll = 0;
          else
            *menu_scroll = *menu_selected - 1;
          screen_scrolled = 1;
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
          if (!*parent_menu_option_active)
          {
#ifdef DEBUG
            std::cout << "a" << std::endl;
#endif
            *parent_menu_option_active = 1;
            lcd_clear(lcd);
            menu_interact(lcd, &(*menu)[*menu_selected].nested_menu_options, &(*menu)[*menu_selected].nest_selected, &(*menu)[*menu_selected].nest_scroll, &(*menu)[*menu_selected].nest_option_active, (*menu)[*menu_selected].style, 1);
#ifdef DEBUG
            std::cout << "j" << std::endl;
#endif
          }
          break;

        case FUNCTION:
          // TODO: handle function menu actions
          (*menu)[*menu_selected].function();
          break;

        default:
          break;
        }
        encoder_btn_pressed = 0;
      }
      // draw
      // if (screen_scrolled || redraw)
      //   lcd_clear(lcd);
#ifdef DEBUG
      std::cout << "menu-size" << menu->size() << std::endl;
      std::cout << "menu_scroll" << (int)*menu_scroll << std::endl;
      std::cout << "menu_select" << (int)*menu_selected << std::endl;
      std::cout << "parent_menu_opti_ac" << (int)*parent_menu_option_active << std::endl;
#endif
      for (uint8_t i = *menu_scroll; i < *menu_scroll + 4; i++)
      {
        if (i < menu->size())
        {
          lcd_pos(lcd, i - *menu_scroll, 0);
          if (i == *menu_selected)
            lcd_print(lcd, (char *)">");
          else
            lcd_print(lcd, (char *)" ");

          if (screen_scrolled || redraw)
          {
#ifdef DEBUG
            std::cout << "i" << (int)i << std::endl;
#endif
            switch ((*menu)[i].style)
            {
            case VALUE:
            {
              uint8_t num_len = num_digits<decltype((*menu)[i].value.num)>((*menu)[i].value.num);
              uint8_t name_len = strlen((*menu)[i].name);

              lcd_printf(lcd, "%.*s%*s%c%*d", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 2) - num_len - 2 : SCREEN_WIDTH - name_len - num_len - 2), /*fill_spaces*/ "", ((*parent_menu_option_active) ? '>' : ' '), num_len, (*menu)[i].value.num);
#ifdef DEBUG
              printf("\"%.*s%*s%c%.*d\"\n", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 2) - num_len - 2 : SCREEN_WIDTH - name_len - num_len - 2), /*fill_spaces*/ "", ((*parent_menu_option_active) ? '>' : ' '), num_len, (*menu)[i].value.num);
#endif
              break;
            }
            default:
              uint8_t name_len = strlen((*menu)[i].name);
              lcd_printf(lcd, "%.*s%*s", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ " ");
#ifdef DEBUG
              printf("\"%.*s%*s\"\n", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ " ");
#endif
              break;
            }
          }
        }
        else
        {
          lcd_printf(lcd, "%*s", SCREEN_WIDTH, "");
        }
      }
#ifdef DEBUG
      std::cout << std::endl;
#endif
    }
  }
  dbg_nests = "";
  return 0;
}