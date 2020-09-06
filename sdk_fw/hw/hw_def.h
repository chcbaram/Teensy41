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



#define _HW_DEF_RTOS_MEM_SIZE(x)              ((x)/4)

#define _HW_DEF_RTOS_THREAD_PRI_MAIN          osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_CMDIF         osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_LCD           osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_I2S           osPriorityAboveNormal
#define _HW_DEF_RTOS_THREAD_PRI_AUDIO         osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_UPDATE        osPriorityNormal


#define _HW_DEF_RTOS_THREAD_MEM_MAIN          _HW_DEF_RTOS_MEM_SIZE(12*1024)
#define _HW_DEF_RTOS_THREAD_MEM_CMDIF         _HW_DEF_RTOS_MEM_SIZE( 6*1024)
#define _HW_DEF_RTOS_THREAD_MEM_LCD           _HW_DEF_RTOS_MEM_SIZE( 1*1024)
#define _HW_DEF_RTOS_THREAD_MEM_I2S           _HW_DEF_RTOS_MEM_SIZE( 1*1024)
#define _HW_DEF_RTOS_THREAD_MEM_AUDIO         _HW_DEF_RTOS_MEM_SIZE( 1*1024)
#define _HW_DEF_RTOS_THREAD_MEM_UPDATE        _HW_DEF_RTOS_MEM_SIZE( 1*1024)



#define _USE_HW_MICROS
#define _USE_HW_VCP
#define _USE_HW_CLOCKS
#define _USE_HW_JOYPAD
#define _USE_HW_RTOS
#define _USE_HW_I2S
#define _USE_HW_FLASH
#define _USE_HW_AUDIO
#define _USE_HW_PXP
#define _USE_HW_BATTERY


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
#define      HW_GPIO_MAX_CH         4

#define _USE_HW_PSRAM
#define      HW_PSRAM_ADDR           0x70000000
#define      HW_PSRAM_LENGTH         (16*1024*1024)

#define _USE_HW_FILES
#define _USE_HW_FATFS
#define _USE_HW_SD
#define      HW_SD_PIN_DETECTED     0
#define      HW_SD_PIN_PWREN       -1

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       12

#define _USE_HW_LCD
#define _USE_HW_ILI9341
#define      HW_LCD_WIDTH           320
#define      HW_LCD_HEIGHT          240

#define _USE_HW_ADC
#define      HW_ADC_MAX_CH          3

#define _USE_HW_PWM
#define      HW_PWM_MAX_CH          1

#define _USE_HW_MIXER
#define      HW_MIXER_MAX_CH        8
#define      HW_MIXER_MAX_BUF_LEN   (16*4*8)

#define _USE_HW_MEM
#define      HW_MEM_ADDR            0x70800000
#define      HW_MEM_LENGTH          (8*1024*1024)




#define FLASH_ADDR_TAG                0x70400800
#define FLASH_ADDR_FW                 0x70400000

#define FLASH_ADDR_START              0x70400000
#define FLASH_ADDR_END                (FLASH_ADDR_START + 2*1-24*1024)



#define _PIN_BUTTON_A               0
#define _PIN_BUTTON_B               1
#define _PIN_BUTTON_X               2
#define _PIN_BUTTON_Y               3
#define _PIN_BUTTON_START           4
#define _PIN_BUTTON_SELECT          5
#define _PIN_BUTTON_HOME            6
#define _PIN_BUTTON_MENU            7
#define _PIN_BUTTON_LEFT            8
#define _PIN_BUTTON_RIGHT           9
#define _PIN_BUTTON_UP              10
#define _PIN_BUTTON_DOWN            11


#define _PIN_GPIO_LCD_BKT_EN        1
#define _PIN_GPIO_LCD_RST           2
#define _PIN_GPIO_BAT_CHG           3



#endif /* SRC_HW_HW_DEF_H_ */
