/*
 * hw.c
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "hw.h"






void hwInit(void)
{
  bspInit();

  microsInit();
  swtimerInit();
  cmdifInit();
  ledInit();
  flashInit();

  uartInit();
  uartOpen(_DEF_UART1, 57600);

  clocksInit();
  #ifndef __APP_MODE
  psramInit();
  #endif
  gpioInit();
  adcInit();
  joypadInit();
  buttonInit();
  pwmInit();
  memInit();


  if (sdInit() == true)
  {
    fatfsInit();
  }

  usbdInit();
  delay(100);

  pxpInit();
  i2sInit();
  audioInit();
  batteryInit();
  speakerInit();
}

