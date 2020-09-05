/*
 * files.cpp
 *
 *  Created on: 2020. 8. 21.
 *      Author: Baram
 */




#include "files.h"



void filesMsgBox(const char *str)
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


void filesMain(void)
{
  audio_t audio;
  int16_t title_h = 30;
  int16_t cursor_y;
  int16_t cursor_h = 30;

  audioOpen(&audio);

  //audioPlayFile(&audio, "sound.wav", false);
  lcdSetResizeMode(LCD_RESIZE_BILINEAR);

  cursor_y = 0;
  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_HOME))
    {
      break;
    }


    if (lcdDrawAvailable())
    {
      lcdClearBuffer(black);


      if (buttonGetRepeatEvent(_PIN_BUTTON_UP))
      {
        cursor_y--;
        if (cursor_y < 0)
        {
          cursor_y = 4;
        }
        audioPlayNote(5, 1, 30);
      }
      if (buttonGetRepeatEvent(_PIN_BUTTON_DOWN))
      {
        cursor_y++;
        if (cursor_y > 4)
        {
          cursor_y = 0;
        }
        audioPlayNote(5, 1, 30);
      }

      lcdDrawFillRect(0 , cursor_y*cursor_h+title_h, 320, cursor_h, blue);

      lcdPrintfRect(3,cursor_h*0+title_h, LCD_WIDTH-6, cursor_h, white, 0.9, LCD_ALIGN_V_CENTER, "OROCABOY4");
      lcdPrintfRect(3,cursor_h*1+title_h, LCD_WIDTH-6, cursor_h, white, 1.0, LCD_ALIGN_V_CENTER, "OROCABOY4");
      lcdPrintfRect(3,cursor_h*2+title_h, LCD_WIDTH-6, cursor_h, white, 1.2, LCD_ALIGN_V_CENTER, "OROCABOY4");
      lcdPrintfRect(3,cursor_h*3+title_h, LCD_WIDTH-6, cursor_h, white, 1.5, LCD_ALIGN_V_CENTER, "OROCABOY4");
      lcdPrintfRect(3,cursor_h*4+title_h, LCD_WIDTH-6, cursor_h, white, 1.8, LCD_ALIGN_V_CENTER, "OROCABOY4");

      lcdPrintfRect(3,cursor_h*0+title_h, LCD_WIDTH-6, cursor_h, white, 0.9, LCD_ALIGN_V_CENTER|LCD_ALIGN_H_RIGHT, "오로카보이4");
      lcdPrintfRect(3,cursor_h*1+title_h, LCD_WIDTH-6, cursor_h, white, 1.0, LCD_ALIGN_V_CENTER|LCD_ALIGN_H_RIGHT, "오로카보이4");
      lcdPrintfRect(3,cursor_h*2+title_h, LCD_WIDTH-6, cursor_h, white, 1.2, LCD_ALIGN_V_CENTER|LCD_ALIGN_H_RIGHT, "오로카보이4");
      lcdPrintfRect(3,cursor_h*3+title_h, LCD_WIDTH-6, cursor_h, white, 1.5, LCD_ALIGN_V_CENTER|LCD_ALIGN_H_RIGHT, "오로카보이4");
      lcdPrintfRect(3,cursor_h*4+title_h, LCD_WIDTH-6, cursor_h, white, 1.8, LCD_ALIGN_V_CENTER|LCD_ALIGN_H_RIGHT, "오로카보이4");


      lcdDrawFillRect(0, 0, LCD_WIDTH, title_h, lightgreen);
      lcdDrawRect    (0, 0, LCD_WIDTH, title_h, white);
      lcdDrawRect    (0, 0, LCD_WIDTH, LCD_HEIGHT, white);

      lcdPrintfRect(0, 0, LCD_WIDTH, title_h, black, 1, LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, "Files");


      if (buttonGetRepeatEvent(_PIN_BUTTON_A))
      {
        filesMsgBox("테스트 메세지");
      }

      lcdRequestDraw();
    }

    osThreadYield();
  }

  audioStopFile(&audio);
}

