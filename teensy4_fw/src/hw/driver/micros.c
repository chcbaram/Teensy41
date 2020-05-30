/*
 * micros.c
 *
 *  Created on: 2020. 2. 22.
 *      Author: Baram
 */




#include "micros.h"
#include "fsl_gpt.h"




bool microsInit(void)
{
  gpt_config_t gpt1_config;


  gpt1_config.clockSource     = kGPT_ClockSource_Periph;
  gpt1_config.divider         = 75;
  gpt1_config.enableFreeRun   = true;
  gpt1_config.enableRunInWait = true;
  gpt1_config.enableRunInStop = true;
  gpt1_config.enableRunInDoze = false;
  gpt1_config.enableRunInDbg  = false;
  gpt1_config.enableMode      = true;


  GPT_Init(GPT1, &gpt1_config);
  GPT_SetOscClockDivider(GPT1, 1);
  GPT_StartTimer(GPT1);


  return true;
}

uint32_t micros(void)
{
  return GPT1->CNT;
}
