/*
 * ap.cpp
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "ap.h"



void apInit(void)
{
  hwInit();

  cmdifOpen(_DEF_UART1, 57600);
}

void apMain(void)
{
  uint32_t pre_time;
  uint32_t pre_time_fps;
  uint32_t pre_time_draw;
  uint32_t time_draw;
  uint32_t fps = 0;
  uint32_t fps_show = 0;
  uint16_t x = 0;
  uint16_t y = 0;


  pre_time = micros();
  while(1)
  {
    cmdifMain();

    if (micros()-pre_time >= 500*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);

      fps_show = fps;
    }

    if (lcdDrawAvailable())
    {
      pre_time_draw = micros();
      lcdClearBuffer(black);

      lcdPrintf(0,16*0, white, "테스트  %d fps, %d ms", fps_show, (millis()-pre_time_fps));
      lcdPrintf(0,16*1, white, "드로우  %d ms", time_draw/1000);

      fps = 1000/(millis()-pre_time_fps);
      pre_time_fps = millis();

      lcdPrintf(0,16*2, white, "X %03d Y %03d", joypadGetX(), joypadGetY());


      uint16_t y_offset = 60;

      lcdDrawFillRect(x, y_offset+32, 30, 30, red);
      lcdDrawFillRect(lcdGetWidth()-x, y_offset+62, 30, 30, green);
      lcdDrawFillRect(x + 30, y_offset+92, 30, 30, blue);

      if (buttonGetPressed(_PIN_BUTTON_A))
      {
        lcdDrawFillRect(150, 200, 30, 30, blue);
      }
      if (buttonGetPressed(_PIN_BUTTON_B))
      {
        lcdDrawFillRect(150-60, 200, 30, 30, green);
      }


      time_draw = micros()-pre_time_draw;

      x += 2;

      x %= lcdGetWidth();
      y %= lcdGetHeight();

      lcdRequestDraw();
    }
  }
}

