/*
 * ap.cpp
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "ap.h"
#include "boot/boot.h"
#include "launcher/launcher.h"


static bool is_cmd = true;
static cmd_t cmd_boot;


static void bootCmdif(void);
static void threadCmdif(void const *argument);
static void threadUpdate(void const *argument);

void apInit(void)
{
  hwInit();

  cmdifOpen(_DEF_UART1, 57600);
  cmdifAdd("boot", bootCmdif);

  cmdInit(&cmd_boot);
  cmdBegin(&cmd_boot, _DEF_UART1, 57600);


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

void apMain(void)
{
  //i2sInit();
  //audioInit();
  //batteryInit();
  lcdInit();


  launcher::main();

  while(1)
  {
    osThreadYield();
  }
}

static void threadCmdif(void const *argument)
{
  (void)argument;
  uint32_t pre_time;
  uint32_t led_period;


  if (buttonGetPressed(_PIN_BUTTON_HOME) == true)
  {
    is_cmd = false;
  }

  while(1)
  {
    if (buttonGetRepeatEvent(_PIN_BUTTON_X) == true)
    {
      is_cmd ^= 1;
    }

    if (is_cmd == true)
    {
      if (cmdReceivePacket(&cmd_boot) == true)
      {
        bootProcessCmd(&cmd_boot);
      }
    }
    else
    {
      cmdifMain();
    }
    delay(1);

    if (is_cmd == true)
    {
      led_period = 100;
    }
    else
    {
      led_period = 500;
    }
    if (millis()-pre_time >= led_period)
    {
      pre_time = millis();

      ledToggle(_DEF_LED1);
    }
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





void bootCmdif(void)
{
  bool ret = true;
  FRESULT res;
  FIL file;
  UINT len;
  uint32_t pre_time;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("jump", 0) == true)
  {
    //void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FW + 4);
    void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FW + 4);

    if ((uint32_t)(*jump_func) != 0xFFFFFFFF)
    {
      cmdifPrintf("jump 0x%X \n", (int)(*jump_func));
      delay(100);
      bspDeInit();

      __set_MSP(*(uint32_t *)FLASH_ADDR_FW);
      (*jump_func)();
    }
    else
    {
      cmdifPrintf("firmware empty \n");
    }
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("load", 0) == true)
  {
    res = f_open(&file, "fw.bin", FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK)
    {
      pre_time = millis();
      f_read(&file, (void *)FLASH_ADDR_FW, f_size(&file), &len);
      SCB_CleanInvalidateDCache();

      cmdifPrintf("copy_fw   \t\t: %dms, %dKB\n", (int)(millis()-pre_time), (int)f_size(&file)/1024);

      cmdifPrintf("size %d \n", f_size(&file));
      f_close(&file);
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "boot jump \n");
  }
}


