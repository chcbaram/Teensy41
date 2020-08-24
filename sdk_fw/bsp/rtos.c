/*
 * rtos.c
 *
 *  Created on: 2020. 1. 29.
 *      Author: Baram
 */




#include "rtos.h"





void rtosInit(void)
{

}


bool rtosIsStarted(void)
{
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    return true;
  }
  else
  {
    return false;
  }
}


void vApplicationStackOverflowHook(xTaskHandle xTask,
                                   signed portCHAR* pcTaskName)
{
  logPrintf("StackOverflow : %s\r\n", pcTaskName);
  while (1);
}

void vApplicationMallocFailedHook(xTaskHandle xTask,
                                  signed portCHAR* pcTaskName)
{
  logPrintf("MallocFailed : %s\r\n", pcTaskName);
  while (1);
}



