/*
 * ap.cpp
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "ap.h"
#include "launcher/launcher.h"


static void testCmdif(void);
static void threadCmdif(void const *argument);
static void threadUpdate(void const *argument);

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

  osThreadDef(threadUpdate, threadUpdate, _HW_DEF_RTOS_THREAD_PRI_UPDATE, 0, _HW_DEF_RTOS_THREAD_MEM_UPDATE);
  if (osThreadCreate(osThread(threadUpdate), NULL) != NULL)
  {
    logPrintf("threadUpdate \t\t: OK\r\n");
  }
  else
  {
    logPrintf("threadUpdate \t\t: Fail\r\n");
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

static void threadUpdate(void const *argument)
{
  (void)argument;

  while(1)
  {
    batteryUpdate();
    delay(1);
  }
}


void apMain(void)
{
  uint32_t pre_time;


  while(buttonGetPressed(_PIN_BUTTON_MENU));



  launcher::main();

  pre_time = micros();
  while(1)
  {
    if (micros()-pre_time >= 100*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);
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
