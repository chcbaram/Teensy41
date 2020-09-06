/*
 * hw_info.cpp
 *
 *  Created on: 2020. 9. 5.
 *      Author: Baram
 */




#include "hw_info.h"
#include "launcher/launcher.h"
namespace hw_info
{








void main(void)
{
  audio_t audio;
  uint32_t pre_time;
  uint32_t pre_time_draw;
  uint32_t time_draw;
  int16_t offset_x = 10;
  int16_t offset_y = 50;


  audioOpen(&audio);


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
      while(lcdDrawAvailable() != true);
      lcdClearBuffer(black);
      break;
    }


    if (lcdDrawAvailable())
    {
      pre_time_draw = micros();
      launcher::drawBackground("H/W 정보");

      lcdPrintf(offset_x, offset_y + 16*0, white, "테스트  %d fps, %d ms, %d ms", lcdGetFps(), lcdGetFpsTime(), lcdGetDrawTime());
      lcdPrintf(offset_x, offset_y + 16*1, white, "드로우  %d ms", time_draw/1000);


      lcdPrintf(offset_x, offset_y + 16*2, white, "X %03d Y %03d", joypadGetX(), joypadGetY());
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        if (buttonGetPressed(i) == true)
        {
          lcdPrintf(offset_x + 120 + 8*i, offset_y + 16*2, white, "1");
        }
        else
        {
          lcdPrintf(offset_x + 120 + 8*i, offset_y + 16*2, white, "0");
        }
      }
      lcdPrintf(offset_x, offset_y + 16*3, white, "밝  기  %d %%", lcdGetBackLight());

      lcdDrawFillRect(offset_x + 0 , offset_y + 16*4, 30, 30, red);
      lcdDrawFillRect(offset_x + 30, offset_y + 16*4, 30, 30, green);
      lcdDrawFillRect(offset_x + 60, offset_y + 16*4, 30, 30, blue);
      lcdPrintf(offset_x +    7, offset_y + 16*4+6, white, "R");
      lcdPrintf(offset_x + 30+7, offset_y + 16*4+6, white, "G");
      lcdPrintf(offset_x + 60+7, offset_y + 16*4+6, white, "B");

      lcdPrintf(offset_x + 0, offset_y + 16*5+12, white, "배터리  %d %%, %d.%02d V", batteryGetLevel(), batteryGetVoltage()/100, batteryGetVoltage()%100);
      lcdPrintf(offset_x + 0, offset_y + 16*6+12, white, "충  전  %d", batteryIsCharging());


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

