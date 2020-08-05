/*
 * hw_def.h
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"



#define _USE_HW_MICROS
#define _USE_HW_VCP
#define _USE_HW_CLOCKS
#define _USE_HW_PSRAM


#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         1

#define _USE_HW_SWTIMER
#define      HW_SWTIMER_MAX_CH      8

#define _USE_HW_CMDIF
#define      HW_CMDIF_LIST_MAX              32
#define      HW_CMDIF_CMD_STR_MAX           16
#define      HW_CMDIF_CMD_BUF_LENGTH        128

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         3

#define _USE_HW_SD
#define      HW_SD_PIN_DETECTED     0
#define      HW_SD_PIN_PWREN       -1

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       8

#define _USE_HW_LCD
#define _USE_HW_ILI9341
#define      HW_LCD_WIDTH           320
#define      HW_LCD_HEIGHT          240


#define _PIN_BUTTON_A               0
#define _PIN_BUTTON_B               1
#define _PIN_BUTTON_X               2
#define _PIN_BUTTON_Y               3
#define _PIN_BUTTON_START           4
#define _PIN_BUTTON_SELECT          5
#define _PIN_BUTTON_HOME            6
#define _PIN_BUTTON_MENU            7


#define _PIN_GPIO_LCD_BKT_EN        1
#define _PIN_GPIO_LCD_RST           2



#endif /* SRC_HW_HW_DEF_H_ */
