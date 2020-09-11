/*
 * reset.c
 *
 *  Created on: 2020. 2. 27.
 *      Author: Baram
 */




#include "reset.h"
#include "fsl_src.h"
#include "cmdif.h"


#ifdef _USE_HW_RTC
#include "rtc.h"

#ifdef _USE_HW_LED
#include "led.h"
#endif
#endif



#define RESET_BOOT_RAM_ADDR         0x20000000


typedef struct
{
  uint32_t boot_mode;
  uint32_t boot_mode_xor;

  uint32_t reset_src;
  uint32_t reset_src_xor;
} reset_boot_mode_t;



static reset_boot_mode_t *p_boot_mode = (reset_boot_mode_t *)RESET_BOOT_RAM_ADDR;


static uint8_t  reset_status = 0x00;
static uint8_t  reset_bits   = 0x00;


#ifdef _USE_HW_CMDIF
void resetCmdif(void);
#endif

void resetInit(void)
{
  uint8_t ret = 0;

#if 0
  p_boot_mode->reset_src = SRC_GetResetStatusFlags(SRC);
  p_boot_mode->reset_src_xor = p_boot_mode->reset_src ^ 0xFFFFFFFF;
  resetClearFlag();


  if (p_boot_mode->reset_src & kSRC_IppResetPinFlag)
  {
   ret |= (1<<_DEF_RESET_PIN);
  }
  if (p_boot_mode->reset_src & kSRC_WatchdogResetFlag)
  {
   ret |= (1<<_DEF_RESET_WDG);
  }
  if (p_boot_mode->reset_src & kSRC_Wdog3ResetFlag)
  {
   ret |= (1<<_DEF_RESET_WDG);
  }
  if (p_boot_mode->reset_src & kSRC_LockupSysResetFlag)
  {
   ret |= (1<<_DEF_RESET_SOFT);
  }

  if (p_boot_mode->boot_mode != (p_boot_mode->boot_mode_xor ^ 0xFFFFFFFF))
  {
    resetSetBootMode(0);
  }
  if ((ret & (1<<_DEF_RESET_SOFT)) == 0)
  {
    resetSetBootMode(0);
  }

#else

  if (p_boot_mode->reset_src & kSRC_IppResetPinFlag)
  {
   ret |= (1<<_DEF_RESET_PIN);
  }
  if (p_boot_mode->reset_src & kSRC_WatchdogResetFlag)
  {
   ret |= (1<<_DEF_RESET_WDG);
  }
  if (p_boot_mode->reset_src & kSRC_Wdog3ResetFlag)
  {
   ret |= (1<<_DEF_RESET_WDG);
  }
  if (p_boot_mode->reset_src & kSRC_LockupSysResetFlag)
  {
   ret |= (1<<_DEF_RESET_SOFT);
  }

#endif

  reset_bits = ret;


  if (ret & (1<<_DEF_RESET_WDG))
  {
    reset_status = _DEF_RESET_WDG;
  }
  else if (ret & (1<<_DEF_RESET_SOFT))
  {
    reset_status = _DEF_RESET_SOFT;
  }
  else if (ret & (1<<_DEF_RESET_POWER))
  {
    reset_status = _DEF_RESET_POWER;
  }
  else
  {
    reset_status = _DEF_RESET_PIN;
  }

#ifdef _USE_HW_CMDIF
  if (cmdifIsInit() != true)
  {
    cmdifInit();
  }
  cmdifAdd("reset", resetCmdif);
#endif
}

void resetLog(void)
{

  if (reset_bits & (1<<_DEF_RESET_POWER))
  {
    logPrintf("ResetFrom \t\t: Power\r\n");
  }
  if (reset_bits & (1<<_DEF_RESET_PIN))
  {
    logPrintf("ResetFrom \t\t: Pin\r\n");
  }
  if (reset_bits & (1<<_DEF_RESET_WDG))
  {
    logPrintf("ResetFrom \t\t: Wdg\r\n");
  }
  if (reset_bits & (1<<_DEF_RESET_SOFT))
  {
    logPrintf("ResetFrom \t\t: Soft\r\n");
  }
}

void resetClearFlag(void)
{
  SRC_ClearResetStatusFlags(SRC, 0xFFFFFFFF);
}

uint8_t resetGetStatus(void)
{
  return reset_status;
}

uint8_t resetGetBits(void)
{
  return reset_bits;
}


void resetRunSoftReset(void)
{
  NVIC_SystemReset();
}

void resetToBoot(void)
{
  resetRunSoftReset();
}

void resetSetBootMode(uint32_t mode)
{
  p_boot_mode->boot_mode = mode;
  p_boot_mode->boot_mode_xor = mode ^ 0xFFFFFFFF;
}

uint32_t resetGetBootMode(void)
{
  return p_boot_mode->boot_mode;
}





void resetCmdif(void)
{
  bool ret = true;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("mode", 0) == true)
  {
    resetLog();
    logPrintf("Boot Mode  \t\t: 0x%X\r\n", (int)resetGetBootMode());
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("reset", 0) == true)
  {
    resetRunSoftReset();
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("jump", 0) == true)
  {
    resetSetBootMode(resetGetBootMode() | (1<<0));
    resetRunSoftReset();
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "reset mode \n");
    cmdifPrintf( "reset reset \n");
    cmdifPrintf( "reset jump \n");
  }
}


