/*
 * pxp.h
 *
 *  Created on: 2020. 8. 16.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_PXP_H_
#define SRC_COMMON_HW_INCLUDE_PXP_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_PXP


typedef struct
{
  int32_t  w;
  int32_t  h;
  int32_t  x;
  int32_t  y;
  int32_t  stride;
  uint16_t *p_data;
} pxp_resize_t;


bool pxpInit(void);
bool pxpResize(pxp_resize_t *p_src, pxp_resize_t *p_dst);

#endif


#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_PXP_H_ */
