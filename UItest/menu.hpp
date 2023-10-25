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
  uint8_t nest_selected = 1;                    // so that back button is possible
  uint8_t nest_scroll = 0;
  bool nest_option_active = 0;

  menu_val<int16_t> value;

  void (*function)(void);
};

void print_test() // TODO: remove print_test
{
  std::cout << "FUNCTION print test WAHOO WAHOO (also both rising and falling interrupts are registered)" << std::endl;
}

#ifdef DEBUG
std::string dbg_nests = "";
#endif

// return 1 if back out of nest (pressed back option)
bool menu_interact(lcd_t *lcd, std::vector<menu_option> *menu, uint8_t *menu_selected, uint8_t *menu_scroll, bool *parent_menu_option_active, menu_option_style parent_menu_style, bool redraw = 0)
{
#ifdef DEBUG
  if (redraw)
  {
    std::cout << "wahoo" << std::endl;
    std::cout << "menu-size" << menu->size() << std::endl;
    std::cout << "menu_scroll" << (int)*menu_scroll << std::endl;
    std::cout << "menu_select" << (int)*menu_selected << std::endl;
    std::cout << "parent_menu_opti_ac" << (int)*parent_menu_option_active << std::endl;
    std::cout << "encbtnpressed" << (int)encoder_btn_pressed << std::endl;
    std::cout << "isvalue" << (int)((*menu)[*menu_selected].style == VALUE) << std::endl;
    std::cout << "wahoo" << std::endl;
  }
#endif

  // #ifdef DEBUG
  // std::cout << (*menu)[*menu_selected].name << " " << (int)*menu_scroll << " " << (int)*parent_menu_option_active << std::endl;
  // #endif
  if (parent_menu_style != ROOT_MENU)
    *menu_selected -= 1; // so that back option being first in the list is possible (bare value can be used when pointing to children)
  if (*parent_menu_option_active && (*menu)[*menu_selected].style != VALUE)
  {
    #ifdef DEBUG
    dbg_nests += (*menu)[*menu_selected].name;
    #endif
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
    if ((*menu)[*menu_selected].style == VALUE && *parent_menu_option_active)
    {
      if (encoder_btn_pressed)
      {
        *parent_menu_option_active = 0;
        encoder_btn_pressed = 0;
        encoder_pos = 0;
        menu_interact(lcd, menu, menu_selected, menu_scroll, parent_menu_option_active, parent_menu_style, true);
      }
      else
      {
        selection_scrolled = 0;
        redraw = (redraw || change_val<decltype((*menu)[*menu_selected].value.num)>(&(*menu)[*menu_selected].value.num, (*menu)[*menu_selected].value.min, (*menu)[*menu_selected].value.max));
#ifdef DEBUG
        if (redraw)
          std::cout << "changeval" << std::endl;
#endif
      }
    }
    else
    {
      if (parent_menu_style != ROOT_MENU)
        *menu_selected += 1; // cancel accounting for back button in menu so that min/max values work
      selection_scrolled = change_val<uint8_t>(menu_selected, 0, (uint8_t)menu->size() - 1);
      if (parent_menu_style != ROOT_MENU)
        *menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
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
      if (selection_scrolled || redraw) // FIXME: screen_scrolled flag 1 when menu_selected == 0/max
      {
#ifdef DEBUG
        std::cout << (int)*menu_selected << " > " << (int)*menu_scroll << " + " << SCREEN_HEIGHT - 2 << std::endl;
#endif
        if (parent_menu_style != ROOT_MENU)
          *menu_selected += 1; // cancel accounting for back button in menu
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
        if (parent_menu_style != ROOT_MENU)
          *menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
      }

      if (encoder_btn_pressed)
      {
        encoder_btn_pressed = 0;
        // handle button
        // TODO: handle back button
        *menu_selected += 1; // cancel accounting for back button in menu
        if (*menu_selected == 0 && parent_menu_style != ROOT_MENU)
        {
          *parent_menu_option_active = 0;
          return 1;
          *menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)
        }
        else
        {
          *menu_selected -= 1; // account for back button in menu again (bare value can be used when pointing to children)

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
              menu_interact(lcd, menu, menu_selected, menu_scroll, parent_menu_option_active, parent_menu_style, true);
#ifdef DEBUG
              std::cout << "j" << std::endl;
#endif
            }
            break;

          case FUNCTION:
            // TODO: handle function menu actions
            (*menu)[*menu_selected].function();
            menu_interact(lcd, menu, menu_selected, menu_scroll, parent_menu_option_active, parent_menu_style, true);
            break;

          default:
            break;
          }
        }
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
#ifdef DEBUG
      std::cout << "redraw:" << (int)redraw << std::endl;
#endif
      if (parent_menu_style != ROOT_MENU)
        *menu_selected += 1; // cancel accounting for back button in menu
      for (uint8_t i = *menu_scroll; i < *menu_scroll + SCREEN_HEIGHT; i++)
      {
        if (i < menu->size() + 1) //+1 for back button
        {
          lcd_pos(lcd, i - *menu_scroll, 0);
          if (i == *menu_selected && !*parent_menu_option_active)
            lcd_print(lcd, (char *)"\3");
          else
            lcd_print(lcd, (char *)" ");

          if (screen_scrolled || redraw)
          {
#ifdef DEBUG
            std::cout << "i" << (int)i << std::endl;
#endif
            if (i == 0 && parent_menu_style != ROOT_MENU)
            {
              lcd_printf(lcd, "%*s", SCREEN_WIDTH - 1, "\1back\6\6\6\6\6\6\6\6\6\6\6\6\6\6");
#ifdef DEBUG
              printf("\"%*s\"\n", SCREEN_WIDTH - 1, "BAAAAAAAAAAADHFASDHKCK");
#endif
            }
            else
            {
              switch ((*menu)[i].style)
              {
              case VALUE:
              {
                if (parent_menu_style != ROOT_MENU)
                  i -= 1; // account for back button in menu again (bare value can be used when pointing to children)
                uint8_t num_len = num_digits<decltype((*menu)[i].value.num)>((*menu)[i].value.num);
                uint8_t name_len = strlen((*menu)[i].name);
#ifdef DEBUG
                std::cout << "i" << (int)i << std::endl;
#endif
                lcd_printf(lcd, "%.*s%*s%c%*d", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 2) - num_len - 2 : SCREEN_WIDTH - name_len - num_len - 2), /*fill_spaces*/ "", ((*parent_menu_option_active && (i == *menu_selected)) ? '\3' : ' '), num_len, (*menu)[i].value.num);
#ifdef DEBUG
                printf("\"%.*s%*s%c%.*d\"\n", /*name_str_max_len*/ SCREEN_WIDTH - num_len - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ ((name_len > SCREEN_WIDTH - num_len - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - num_len - 2) - num_len - 2 : SCREEN_WIDTH - name_len - num_len - 2), /*fill_spaces*/ "", ((*parent_menu_option_active && (i == *menu_selected)) ? '>' : ' '), num_len, (*menu)[i].value.num);
#endif
                if (parent_menu_style != ROOT_MENU)
                  i += 1; // cancel accounting for back button in menu
                break;
              }
              default:
              {
                if (parent_menu_style != ROOT_MENU)
                  i -= 1; // account for back button in menu again (bare value can be used when pointing to children)
                uint8_t name_len = strlen((*menu)[i].name);
#ifdef DEBUG
                std::cout << "i" << (int)i << std::endl;
#endif
                lcd_printf(lcd, "%.*s%*s", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ "");
#ifdef DEBUG
                printf("\"%.*s%*s\"\n", /*name_str_max_len*/ SCREEN_WIDTH - 2 /* 2 = the ">"/" " chars */, (*menu)[i].name, /*fill_spaces_len*/ (name_len > SCREEN_WIDTH - 2) ? SCREEN_WIDTH - (SCREEN_WIDTH - 2) : SCREEN_WIDTH - name_len - 1, /*fill_spaces*/ "");
#endif
                if (parent_menu_style != ROOT_MENU)
                  i += 1; // cancel accounting for back button in menu
                break;
              }
              }
            }
          }
        }
        else
        {
          lcd_printf(lcd, "%*s", SCREEN_WIDTH, "");
        }
      }
      // accounting for back button is done at the beginning of this function, it would execute twice in a row if the same was done at the end
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