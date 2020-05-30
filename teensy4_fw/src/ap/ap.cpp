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
}

void apMain(void)
{
  uint32_t pre_time;


  pre_time = micros();
  while(1)
  {
    if (micros()-pre_time >= 500*1000)
    {
      pre_time = micros();

      ledToggle(_DEF_LED1);
    }
  }
}

