/*
 * hw.h
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */

#ifndef SRC_HW_HW_H_
#define SRC_HW_HW_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"



#include "led.h"
#include "micros.h"
#include "cmdif.h"
#include "swtimer.h"
#include "vcp.h"
#include "uart.h"
#include "clocks.h"
#include "psram.h"
#include "gpio.h"
#include "sd.h"
#include "button.h"
#include "lcd.h"
#include "adc.h"
#include "joypad.h"
#include "pwm.h"
#include "i2s.h"
#include "mixer.h"

#include "fatfs.h"

void hwInit(void);


#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_HW_H_ */
