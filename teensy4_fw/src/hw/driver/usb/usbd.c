/*
 * usb_device.c
 *
 *  Created on: 2020. 2. 25.
 *      Author: Baram
 */




#include "usbd.h"
#include "vcp.h"


extern void usbCompositeInit(void);



bool usbdInit(void)
{
  vcpInit();
  usbCompositeInit();

  return true;
}
