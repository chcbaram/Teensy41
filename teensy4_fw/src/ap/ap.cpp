/*
 * ap.cpp
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "ap.h"


static void testCmdif(void);
static void threadCmdif(void const *argument);


void apInit(void)
{
  hwInit();

  cmdifOpen(_DEF_UART1, 57600);
  cmdifAdd("test", testCmdif);

  osThreadDef(threadCmdif, threadCmdif, _HW_DEF_RTOS_THREAD_PRI_CMDIF, 0, _HW_DEF_RTOS_THREAD_MEM_CMDIF);
  if (osThreadCreate(osThread(threadCmdif), NULL) != NULL)
  {
    logPrintf("threadCmdif \t\t: OK\r\n");
  }
  else
  {
    logPrintf("threadCmdif \t\t: Fail\r\n");
    while(1);
  }
}

static void threadCmdif(void const *argument)
{
  (void)argument;

  while(1)
  {
    cmdifMain();
    delay(1);
  }
}

LV_IMG_DECLARE(image_src);

void apMain(void)
{
  uint32_t pre_time;
  uint32_t pre_time_draw;
  uint32_t time_draw;
  uint16_t x = 0;
  uint16_t y = 0;

  audio_t audio;

  audioOpen(&audio);


  pre_time = micros();
  while(1)
  {
    if (micros()-pre_time >= 100*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);
    }

    if (lcdDrawAvailable())
    {
      pre_time_draw = micros();
      lcdClearBuffer(black);

      lcdPrintf(0,16*0, white, "테스트  %d fps, %d ms, %d ms", lcdGetFps(), lcdGetFpsTime(), lcdGetDrawTime());
      lcdPrintf(0,16*1, white, "드로우  %d ms", time_draw/1000);


      lcdPrintf(0,16*2, white, "X %03d Y %03d", joypadGetX(), joypadGetY());
      lcdPrintf(0,16*3, white, "밝  기  %d %%", lcdGetBackLight());

      lcdDrawFillRect(0 , 16*4, 30, 30, red);
      lcdDrawFillRect(30, 16*4, 30, 30, green);
      lcdDrawFillRect(60, 16*4, 30, 30, blue);
      lcdPrintf(7,16*4+6, white, "R");
      lcdPrintf(30+7,16*4+6, white, "G");
      lcdPrintf(60+7,16*4+6, white, "B");

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


      x += 2;

      x %= lcdGetWidth();
      y %= lcdGetHeight();

      lcdRequestDraw();
    }

    osThreadYield();
  }
}





void testCmdif(void)
{
  bool ret = true;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("info", 0) == true)
  {
    cmdifPrintf("GPR16 0x%08X\n", IOMUXC_GPR->GPR16);
    cmdifPrintf("GPR17 0x%08X\n", IOMUXC_GPR->GPR17);
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("mem", 0) == true)
  {
    uint32_t *p_buf;

    p_buf = (uint32_t *)memMalloc(1*1024*1024);

    if (p_buf)
    {
      cmdifPrintf("memMalloc OK\n");

      for (uint32_t i=0; i<1*1024*1024/4; i++)
      {
        p_buf[i] = i;
      }
      for (uint32_t i=0; i<1*1024*1024/4; i++)
      {
        if (p_buf[i] != i)
        {
          cmdifPrintf("err %d \n", i);
        }
      }
      cmdifPrintf("test finished\n");
    }
    else
    {
      cmdifPrintf("memMalloc Fail\n");
    }


  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "test info \n");
    cmdifPrintf( "test mem \n");
  }
}
