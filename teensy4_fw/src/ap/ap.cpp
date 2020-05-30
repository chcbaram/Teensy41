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


  pre_time = micros();
  while(1)
  {
    cmdifMain();

    if (micros()-pre_time >= 500*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);
    }
  }
}

