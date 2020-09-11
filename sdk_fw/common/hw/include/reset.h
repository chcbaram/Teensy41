/*
 * reset.h
 *
 *  Created on: 2020. 2. 27.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_RESET_H_
#define SRC_COMMON_HW_INCLUDE_RESET_H_


#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_RESET



#define RESET_MODE_TO_BOOT    1
#define RESET_MODE_TO_JUMP    0


void resetInit(void);
void resetLog(void);
void resetClearFlag(void);
void resetRunSoftReset(void);
uint8_t resetGetStatus(void);
uint8_t resetGetBits(void);
void resetToBoot(void);
void resetSetBootMode(uint32_t mode);
uint32_t resetGetBootMode(void);


#endif

#ifdef __cplusplus
 }
#endif


#endif /* SRC_COMMON_HW_INCLUDE_RESET_H_ */
