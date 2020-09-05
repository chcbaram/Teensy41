/*
 * hw_info.cpp
 *
 *  Created on: 2020. 9. 5.
 *      Author: Baram
 */




#include "hw_info.h"

namespace hw_info
{








void main(void)
{
  audio_t audio;
  uint32_t pre_time;
  uint32_t pre_time_draw;
  uint32_t time_draw;


  audioOpen(&audio);

  lcdSetResizeMode(LCD_RESIZE_BILINEAR);

  pre_time = micros();
  while(1)
  {
    if (micros()-pre_time >= 100*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);
    }

    if (buttonGetRepeatEvent(_PIN_BUTTON_HOME))
    {
      break;
    }


    if (lcdDrawAvailable())
    {
      pre_time_draw = micros();
      lcdClearBuffer(black);

      lcdPrintf(0,16*0, white, "테스트  %d fps, %d ms, %d ms", lcdGetFps(), lcdGetFpsTime(), lcdGetDrawTime());
      lcdPrintf(0,16*1, white, "드로우  %d ms", time_draw/1000);


      lcdPrintf(0,16*2, white, "X %03d Y %03d", joypadGetX(), joypadGetY());
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        if (buttonGetPressed(i) == true)
        {
          lcdPrintf(120 + 8*i,16*2, white, "1");
        }
        else
        {
          lcdPrintf(120 + 8*i,16*2, white, "0");
        }
      }
      lcdPrintf(0,16*3, white, "밝  기  %d %%", lcdGetBackLight());

      lcdDrawFillRect(0 , 16*4, 30, 30, red);
      lcdDrawFillRect(30, 16*4, 30, 30, green);
      lcdDrawFillRect(60, 16*4, 30, 30, blue);
      lcdPrintf(7,16*4+6, white, "R");
      lcdPrintf(30+7,16*4+6, white, "G");
      lcdPrintf(60+7,16*4+6, white, "B");

      lcdPrintf(0,16*5+12, white, "배터리  %d %%, %d.%02d V", batteryGetLevel(), batteryGetVoltage()/100, batteryGetVoltage()%100);
      lcdPrintf(0,16*6+12, white, "충  전  %d", batteryIsCharging());


      if (buttonGetRepeatEvent(_PIN_BUTTON_A))
      {
        audioPlayNote(5, 1, 30);
        lcdSetBackLight(lcdGetBackLight() + 10);
      }
      if (buttonGetRepeatEvent(_PIN_BUTTON_SELECT))
      {
        audioStopFile(&audio);
      }
      if (buttonGetRepeatEvent(_PIN_BUTTON_START))
      {
        audioPlayFile(&audio, "sound.wav", false);
      }
      if (buttonGetRepeatEvent(_PIN_BUTTON_B))
      {
        audioPlayNote(5, 1, 30);

        if (lcdGetBackLight() >= 10)
        {
          lcdSetBackLight(lcdGetBackLight() - 10);
        }
        else
        {
          lcdSetBackLight(0);
        }
      }

      time_draw = micros()-pre_time_draw;



      lcdRequestDraw();
    }

    osThreadYield();
  }

  audioClose(&audio);
}


void update(void)
{
}

bool render(void)
{
  return true;
}


}

