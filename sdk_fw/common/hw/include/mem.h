/*
 * mem.h
 *
 *  Created on: 2020. 8. 8.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_MEM_H_
#define SRC_COMMON_HW_INCLUDE_MEM_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_MEM


void  memInit(void);
void *memMalloc(uint32_t size);
void  memFree(void *ptr);
void *memCalloc(size_t nmemb, size_t size);
void *memRealloc(void *ptr, size_t size);


#endif


#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_MEM_H_ */
