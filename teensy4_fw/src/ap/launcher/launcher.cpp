/*
 * launcher.cpp
 *
 *  Created on: 2020. 9. 5.
 *      Author: Baram
 */




#include "launcher.h"
#include "app/hw_info/hw_info.h"


namespace launcher
{

#define BOX_COLOR     RGB2COLOR(171, 171, 171)
#define BG_COLOR      RGB2COLOR(10, 0, 127)



typedef struct
{
  char str[32];
  void (*func)(void);
} menu_node_t;



typedef struct
{
  int32_t list_max;
  int32_t list_max_scr;
  int32_t cursor_cur;
  int32_t cursor_scr;
  int32_t cursor_offset;


  menu_node_t str_list[32];
} menu_list_t;


menu_list_t menu;


void update(void);
bool render(void);

void drawBackground(void);
void drawMainMenu(void);
void drawBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thick, uint16_t color);
void drawBoxOut(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawBoxIn(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);


void main(void)
{
  audio_t audio;
  bool is_exit = false;

  audioOpen(&audio);



  menu.cursor_cur = 0;
  menu.cursor_scr = 0;
  menu.cursor_offset = 0;
  menu.list_max_scr = 5;
  menu.list_max = 8;

  for (uint32_t i=0; i<sizeof(menu.str_list)/sizeof(menu_node_t); i++)
  {
    menu.str_list[i].func = NULL;
  }

  strcpy(menu.str_list[0].str, "H/W 정보");
  strcpy(menu.str_list[1].str, "설정하기");
  strcpy(menu.str_list[2].str, "테스트1");
  strcpy(menu.str_list[3].str, "테스트2");
  strcpy(menu.str_list[4].str, "테스트3");
  strcpy(menu.str_list[5].str, "테스트4");
  strcpy(menu.str_list[6].str, "테스트5");
  strcpy(menu.str_list[7].str, "테스트6");

  menu.str_list[0].func = hw_info::main;


  while(is_exit == false)
  {
    if (lcdDrawAvailable())
    {
      update();

      if (render() == true)
      {
        lcdRequestDraw();
      }
    }

    osThreadYield();
  }

  audioClose(&audio);
}


void update(void)
{
  if (buttonGetRepeatEvent(_PIN_BUTTON_UP))
  {
    menu.cursor_cur--;
    if (menu.cursor_cur < 0)
    {
      menu.cursor_cur = menu.list_max - 1;
    }
    audioPlayNote(5, 1, 30);
  }
  if (buttonGetRepeatEvent(_PIN_BUTTON_DOWN))
  {
    menu.cursor_cur++;
    if (menu.cursor_cur >= menu.list_max)
    {
      menu.cursor_cur = 0;
    }
    audioPlayNote(5, 1, 30);
  }

  if (buttonGetRepeatEvent(_PIN_BUTTON_A))
  {
    if (menu.str_list[menu.cursor_cur].func != NULL)
    {
      menu.str_list[menu.cursor_cur].func();
    }
  }
}

bool render(void)
{
  drawBackground();
  drawMainMenu();

  return true;
}

void drawBackground(void)
{
  int16_t info_h = 30;
  int16_t info_y =  0;


  lcdClearBuffer(RGB2COLOR(10, 0, 127));


  //lcdDrawFillRect(0, info_y, LCD_WIDTH, info_h, BOX_COLOR);

  drawBox(2, info_y, LCD_WIDTH-4, info_h, 1, BOX_COLOR);

  lcdPrintfRect(0, info_y, LCD_WIDTH, info_h, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "OROCABOY");
  lcdPrintfRect(0, info_y, LCD_WIDTH, info_h, black, 1, LCD_ALIGN_H_RIGHT|LCD_ALIGN_V_CENTER, "BAT %d%% ", batteryGetLevel());
}

void drawMainMenu(void)
{
  int16_t box_x;
  int16_t box_y;
  int16_t box_w = 240;
  int16_t box_h = 180;

  int16_t title_box_w = 220;
  int16_t title_box_h = 20;
  int16_t title_box_x;
  int16_t title_box_y;

  int16_t menu_x;
  int16_t menu_y;
  int16_t menu_h;
  int16_t menu_w;

  uint16_t menu_color;


  box_x = (LCD_WIDTH  - box_w)/2;
  box_y = (LCD_HEIGHT - box_h + 30)/2;

  title_box_x = (LCD_WIDTH  - title_box_w)/2;
  title_box_y = box_y + 6;


  menu_x = title_box_x;
  menu_y = title_box_y + 35;
  menu_h = 24;
  menu_w = title_box_w - 10;


  drawBox(box_x, box_y, box_w, box_h, 2, gray);

  drawBoxOut(title_box_x, title_box_y, title_box_w, title_box_h, BOX_COLOR);
  drawBoxOut(title_box_x, title_box_y, title_box_w, title_box_h, BOX_COLOR);
  lcdPrintfRect(title_box_x, title_box_y, title_box_w, title_box_h, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "메인메뉴");


  drawBoxIn(title_box_x, title_box_y+26, title_box_w, box_h-40, BG_COLOR);

  menu.cursor_scr = menu.cursor_cur%menu.list_max_scr;
  menu.cursor_offset = menu.cursor_cur - menu.cursor_scr;

  for (int i=0; i<menu.list_max_scr; i++)
  {
    if ((menu.cursor_offset + i) < menu.list_max)
    {
      if (i == menu.cursor_scr)
      {
        lcdDrawFillRect(menu_x+5, menu_y + i*menu_h + 1, menu_w, menu_h-2, white);
        menu_color = black;
      }
      else
      {
        menu_color = white;
      }

      lcdPrintfRect(menu_x, menu_y + i*menu_h, menu_w, menu_h, menu_color, 1,
                    LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER,
                    menu.str_list[menu.cursor_offset+i].str);
    }
  }
}

void drawBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thick, uint16_t color)
{
  int16_t in_x;
  int16_t in_y;
  int16_t in_w;
  int16_t in_h;


  in_x = x+thick+1;
  in_w = w-thick*2-1*2;
  in_y = y+thick+1;
  in_h = h-thick*2-1*2;

  lcdDrawFillRect(x, y, w, h, BOX_COLOR);

  lcdDrawVLine(x  , y  , h, white);
  lcdDrawHLine(x  , y  , w, white);
  lcdDrawVLine(x+w, y  , h, black);
  lcdDrawHLine(x  , y+h, w, black);

  lcdDrawVLine(in_x, in_y, in_h, black);
  lcdDrawHLine(in_x, in_y, in_w, black);
  lcdDrawVLine(in_x+in_w, in_y, in_h, white);
  lcdDrawHLine(in_x, in_y+in_h, in_w, white);
}

void drawBoxOut(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawFillRect(x, y, w, h, color);

  lcdDrawVLine(x  , y  , h, white);
  lcdDrawHLine(x  , y  , w, white);
  lcdDrawVLine(x+w, y  , h, black);
  lcdDrawHLine(x  , y+h, w, black);
}

void drawBoxIn(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawFillRect(x, y, w, h, color);

  lcdDrawVLine(x  , y  , h, black);
  lcdDrawHLine(x  , y  , w, black);
  lcdDrawVLine(x+w, y  , h, white);
  lcdDrawHLine(x  , y+h, w, white);
}

}

