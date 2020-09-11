/*
 * files.cpp
 *
 *  Created on: 2020. 8. 21.
 *      Author: Baram
 */




#include "files.h"
#include "launcher/launcher.h"
namespace files
{



#define FILES_DIR         "files"
#define FILES_LIST_MAX    32

typedef struct
{
  char     file_name[32];
  uint32_t file_size;
} file_node_t;


typedef struct
{
  int32_t list_max;
  int32_t list_max_scr;
  int32_t cursor_cur;
  int32_t cursor_scr;
  int32_t cursor_offset;

  file_node_t file_node[FILES_LIST_MAX];
} file_list_t;



void drawMsgBox(const char *str);
bool updateFileList(file_list_t *p_list);
void drawFileList(file_list_t *p_menu);

void main(void)
{
  audio_t audio;
  file_list_t file_list;
  char file_str[64];


  audioOpen(&audio);
  lcdSetResizeMode(LCD_RESIZE_BILINEAR);

  file_list.cursor_cur = 0;
  file_list.cursor_offset = 0;
  file_list.cursor_scr = 0;
  file_list.list_max_scr = 8;

  updateFileList(&file_list);


  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_HOME) || buttonGetRepeatEvent(_PIN_BUTTON_B))
    {
      while(lcdDrawAvailable() != true);
      lcdClearBuffer(black);
      break;
    }


    if (lcdDrawAvailable())
    {
      launcher::drawBackground("파일 실행");


      if (buttonGetRepeatEvent(_PIN_BUTTON_UP))
      {
        file_list.cursor_cur--;
        if (file_list.cursor_cur < 0)
        {
          file_list.cursor_cur = file_list.list_max - 1;
        }
      }
      if (buttonGetRepeatEvent(_PIN_BUTTON_DOWN))
      {
        file_list.cursor_cur++;
        if (file_list.cursor_cur >= file_list.list_max)
        {
          file_list.cursor_cur = 0;
        }
      }

      if (buttonGetRepeatEvent(_PIN_BUTTON_A))
      {
        sprintf(file_str, "/%s/%s", FILES_DIR, file_list.file_node[file_list.cursor_cur].file_name);
        if (launcher::runFile(file_str) != true)
        {
          drawMsgBox("잘못된 파일");
        }
      }

      drawFileList(&file_list);

      lcdRequestDraw();
    }

    osThreadYield();
  }

  audioStopFile(&audio);
}


void drawMsgBox(const char *str)
{
  int16_t box_x;
  int16_t box_y;
  int16_t box_w = 240;
  int16_t box_h = 120;

  box_x = (LCD_WIDTH  - box_w)/2;
  box_y = (LCD_HEIGHT - box_h)/2;


  lcdDrawFillRect(box_x, box_y, box_w, box_h, gray);
  lcdDrawFillRect(box_x, box_y, box_w, 30, lightblue);
  lcdDrawRect(box_x, box_y, box_w, box_h, white);

  lcdPrintfRect(box_x, box_y+ 0, box_w,       30, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "Message");
  lcdPrintfRect(box_x, box_y+30, box_w, box_h-30, white, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, str);


  lcdRequestDraw();

  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_B))
    {
      break;
    }
  }
}

void drawFileList(file_list_t *p_menu)
{
  int16_t box_x;
  int16_t box_y;
  int16_t box_w = 280;
  int16_t box_h = 200;

  int16_t menu_x;
  int16_t menu_y;
  int16_t menu_h;
  int16_t menu_w;

  uint16_t menu_color;
  char file_name[32];


  box_x = (LCD_WIDTH  - box_w)/2;
  box_y = (LCD_HEIGHT - box_h + 35)/2;


  menu_x = box_x;
  menu_y = box_y;
  menu_h = 24;
  menu_w = box_w;



  p_menu->cursor_scr = p_menu->cursor_cur%p_menu->list_max_scr;
  p_menu->cursor_offset = p_menu->cursor_cur - p_menu->cursor_scr;

  for (int i=0; i<p_menu->list_max_scr; i++)
  {
    if ((p_menu->cursor_offset + i) < p_menu->list_max)
    {
      if (i == p_menu->cursor_scr)
      {
        lcdDrawFillRect(menu_x, menu_y + i*menu_h + 1, menu_w, menu_h-2, white);
        menu_color = black;
      }
      else
      {
        menu_color = white;
      }

      strncpy(file_name, p_menu->file_node[p_menu->cursor_offset+i].file_name, 32);
      if (strlen(file_name) > 20)
      {
        file_name[20] = '.';
        file_name[21] = '.';
        file_name[22] = '.';
        file_name[23] = 0;
      }

      lcdPrintfRect(menu_x, menu_y + i*menu_h, menu_w, menu_h, menu_color, 1,
                    LCD_ALIGN_H_LEFT|LCD_ALIGN_V_CENTER,
                    file_name);

      lcdPrintfRect(menu_x, menu_y + i*menu_h, menu_w, menu_h, menu_color, 1,
                    LCD_ALIGN_H_RIGHT|LCD_ALIGN_V_CENTER,
                    "%dKB", p_menu->file_node[p_menu->cursor_offset+i].file_size/1024);
    }
  }
}

bool updateFileList(file_list_t *p_list)
{
  bool ret = false;
  FRESULT res;
  DIR dir;
  FILINFO fno;
  FIL file;
  //UINT len;
  char file_str[64];


  p_list->list_max = 0;


  res = f_opendir(&dir, FILES_DIR);
  if (res == FR_OK)
  {
    while(1)
    {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR)
      {

      }
      else
      {
        sprintf(p_list->file_node[p_list->list_max].file_name, "%s", fno.fname);
        sprintf(file_str, "/%s/%s", FILES_DIR, fno.fname);

        res = f_open(&file, file_str, FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK)
        {
          //f_read(&file, (void *)&slot_fw_tag, sizeof(slot_fw_tag), &len);

          p_list->file_node[p_list->list_max].file_size = f_size(&file);

          p_list->list_max++;

          if (p_list->list_max >= FILES_LIST_MAX)
          {
            break;
          }
        }
        f_close(&file);
      }
    }
    f_closedir(&dir);
  }

  if (p_list->list_max > 0)
  {
    ret = true;
  }
  return ret;
}

}
