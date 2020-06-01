/*
 * bsp.h
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"

#include "config/board/clock_config.h"
#include "config/board/pin_mux.h"
#include "config/board/peripherals.h"


#include "fsl_gpio.h"


#define BOARD_FLASH_SIZE        (0x800000U)
#define BOARD_SD_SUPPORT_180V   0


#define logPrintf(...)    printf(__VA_ARGS__)


void bspInit(void);
void bspDeInit(void);

extern void delay(uint32_t delay_ms);
extern uint32_t millis(void);
extern uint32_t micros(void);


#ifdef __cplusplus
}
#endif


#endif /* SRC_BSP_BSP_H_ */
