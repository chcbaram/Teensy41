/*
 * usb_device.c
 *
 *  Created on: 2020. 2. 25.
 *      Author: Baram
 */




#include "usbd.h"
#include "vcp.h"


extern void usbCompositeInit(void);
extern bool USB_DeviceIsMSC(void);


bool usbdInit(void)
{
  vcpInit();
  usbCompositeInit();

  return true;
}

bool usbdIsMSC(void)
{
  return USB_DeviceIsMSC();
}
