/*
 * launcher.cpp
 *
 *  Created on: 2020. 9. 5.
 *      Author: Baram
 */




#include "launcher.h"
#include "app/hw_info/hw_info.h"
#include "app/files/files.h"


namespace launcher
{

#define BOX_COLOR     RGB2COLOR(171, 171, 171)
#define BG_COLOR      RGB2COLOR(10, 0, 127)


typedef struct menu_list_ menu_list_t;



extern menu_list_t menu_main;
extern menu_list_t menu_emul;





typedef struct
{
  char str[32];
  void (*func)(void);

  menu_list_t *menu_list;
} menu_node_t;


typedef struct menu_list_
{
  char title_str[32];
  int32_t list_max;
  int32_t list_max_scr;
  int32_t cursor_cur;
  int32_t cursor_scr;
  int32_t cursor_offset;

  const menu_node_t *node_list;

  menu_list_ *menu_list_pre;
} menu_list_t;


const menu_node_t main_node[] =
    {
        {"게임 하기",   NULL, &menu_emul},
        {"파일 실행",   files::main, NULL},
        {"H/W  정보", hw_info::main, NULL},
        {"설정 하기",   NULL, NULL},
        {"테스트 1",   NULL, NULL},
        {"테스트 2",   NULL, NULL},
        {"테스트 3",   NULL, NULL},
        {"테스트 4",   NULL, NULL},
        {"테스트 5",   NULL, NULL},
    };

const menu_node_t emul_node[] =
    {
        {"오로카 갤러그", NULL, NULL},
        {"오로카 팩맨",   NULL, NULL},
    };


menu_list_t menu_main;
menu_list_t menu_emul;
menu_list_t *p_menu;



void update(void);
bool render(void);


void drawMainMenu(void);
void drawBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thick, uint16_t color);
void drawBoxOut(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawBoxIn(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void showTitle(void);


void main(void)
{
  audio_t audio;
  bool is_exit = false;

  audioOpen(&audio);


  strcpy(menu_main.title_str, "메인메뉴");
  menu_main.cursor_cur = 0;
  menu_main.cursor_scr = 0;
  menu_main.cursor_offset = 0;
  menu_main.list_max_scr = 5;
  menu_main.list_max = sizeof(main_node)/sizeof(menu_node_t);
  menu_main.menu_list_pre = NULL;
  menu_main.node_list = main_node;


  strcpy(menu_emul.title_str, "게임하기");
  menu_emul.cursor_cur = 0;
  menu_emul.cursor_scr = 0;
  menu_emul.cursor_offset = 0;
  menu_emul.list_max_scr = 5;
  menu_emul.list_max = sizeof(emul_node)/sizeof(menu_node_t);
  menu_emul.menu_list_pre = NULL;
  menu_emul.node_list = emul_node;


  p_menu = &menu_main;


  showTitle();

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
    p_menu->cursor_cur--;
    if (p_menu->cursor_cur < 0)
    {
      p_menu->cursor_cur = p_menu->list_max - 1;
    }
    audioPlayNote(5, 1, 30);
  }
  if (buttonGetRepeatEvent(_PIN_BUTTON_DOWN))
  {
    p_menu->cursor_cur++;
    if (p_menu->cursor_cur >= p_menu->list_max)
    {
      p_menu->cursor_cur = 0;
    }
    audioPlayNote(5, 1, 30);
  }

  if (buttonGetRepeatEvent(_PIN_BUTTON_A))
  {
    if (p_menu->node_list[p_menu->cursor_cur].func != NULL)
    {
      p_menu->node_list[p_menu->cursor_cur].func();
    }
    else if (p_menu->node_list[p_menu->cursor_cur].menu_list != NULL)
    {
      p_menu->node_list[p_menu->cursor_cur].menu_list->menu_list_pre = p_menu;
      p_menu = p_menu->node_list[p_menu->cursor_cur].menu_list;
    }
    else
    {
      drawMsgBox("없     음", red, 500);
    }
  }
  if (buttonGetRepeatEvent(_PIN_BUTTON_B) || buttonGetRepeatEvent(_PIN_BUTTON_HOME))
  {
    if (p_menu->menu_list_pre != NULL)
    {
      p_menu = p_menu->menu_list_pre;
    }
  }
}

bool render(void)
{
  drawBackground("OROCABOY");
  drawMainMenu();

  return true;
}

void drawBackground(const char *title_str)
{
  int16_t info_h = 30;
  int16_t info_y =  0;


  lcdClearBuffer(RGB2COLOR(10, 0, 127));


  //lcdDrawFillRect(0, info_y, LCD_WIDTH, info_h, BOX_COLOR);

  drawBox(2, info_y, LCD_WIDTH-4, info_h, 1, BOX_COLOR);

  lcdPrintfRect(0, info_y, LCD_WIDTH, info_h, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, title_str);
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
  lcdPrintfRect(title_box_x, title_box_y, title_box_w, title_box_h, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, p_menu->title_str);


  drawBoxIn(title_box_x, title_box_y+26, title_box_w, box_h-40, BG_COLOR);

  p_menu->cursor_scr = p_menu->cursor_cur%p_menu->list_max_scr;
  p_menu->cursor_offset = p_menu->cursor_cur - p_menu->cursor_scr;

  for (int i=0; i<p_menu->list_max_scr; i++)
  {
    if ((p_menu->cursor_offset + i) < p_menu->list_max)
    {
      if (i == p_menu->cursor_scr)
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
                    p_menu->node_list[p_menu->cursor_offset+i].str);
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

void drawMsgBox(const char *str, uint16_t txt_color, uint32_t timeout)
{
  int16_t box_x;
  int16_t box_y;
  int16_t box_w = 240;
  int16_t box_h = 80;
  uint32_t pre_time;

  box_x = (LCD_WIDTH  - box_w)/2;
  box_y = (LCD_HEIGHT - box_h)/2;


  drawBox(box_x, box_y, box_w, box_h, 2, BG_COLOR);
  lcdPrintfRect(box_x, box_y, box_w, box_h, txt_color, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, str);

  lcdRequestDraw();

  pre_time = millis();
  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_B))
    {
      break;
    }
    if (timeout > 0 && (millis()-pre_time) >= timeout)
    {
      break;
    }
  }
}

void showTitle(void)
{
  int16_t box_x;
  int16_t box_y;
  int16_t box_w = 240;
  int16_t box_h = 120;
  uint32_t pre_time;

  box_x = (LCD_WIDTH  - box_w)/2;
  box_y = (LCD_HEIGHT - box_h)/2;


  lcdClearBuffer(BG_COLOR);


  drawBox(box_x, box_y, box_w, box_h, 2, BG_COLOR);
  drawBoxIn(box_x+4, box_y+4, box_w-8, 20, BG_COLOR);
  lcdPrintfRect(box_x+4, box_y+4, box_w-8, 20, white, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "오픈로보틱스 커뮤니티");

  drawBoxOut(box_x+6, box_y+6, 15, 15, BOX_COLOR);
  drawBoxOut(box_x+box_w-22, box_y+6, 15, 15, BOX_COLOR);

  drawBoxOut(box_x+10, box_y+28, box_w-20, 40, BOX_COLOR);
  lcdPrintfRect(box_x, box_y+28, box_w, 40, red,   2, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "        4");
  lcdPrintfRect(box_x, box_y+28, box_w, 40, black, 2, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "OROCABOY ");

  lcdPrintfRect(box_x, box_y+15, box_w, box_h-40, white, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_BOTTOM, "OROCA.ORG");
  lcdPrintfRect(box_x, box_y+15, box_w, box_h-20, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_BOTTOM, "정식사용자 입니다");

  lcdPrintf(0, 200, white, "AT &C1 B0");
  lcdPrintf(0, 216, white, "OK");

  lcdRequestDraw();

  pre_time = millis();
  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_A))
    {
      break;
    }
    if (buttonGetRepeatEvent(_PIN_BUTTON_B))
    {
      break;
    }
    if ((millis()-pre_time) >= 3000)
    {
      //break;
    }
  }
}

bool runFile(const char *file_name)
{
  bool ret = false;
  FRESULT res;
  FIL file;
  UINT len;


  res = f_open(&file, file_name, FA_OPEN_EXISTING | FA_READ);
  if (res == FR_OK)
  {
    void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FW + 4);

    f_read(&file, (void *)FLASH_ADDR_FW, f_size(&file), &len);
    SCB_CleanInvalidateDCache();
    f_close(&file);

    if ((uint32_t)(*jump_func) != 0xFFFFFFFF)
    {
      bspDeInit();
      __set_MSP(*(uint32_t *)FLASH_ADDR_FW);
      (*jump_func)();
    }
  }

  return ret;
}



}

