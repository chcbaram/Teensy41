/*
 * clocks.h
 *
 *  Created on: 2020. 2. 20.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_CLOCKS_H_
#define SRC_COMMON_HW_INCLUDE_CLOCKS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_CLOCKS


bool clocksInit(void);

#endif


#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_CLOCKS_H_ */
