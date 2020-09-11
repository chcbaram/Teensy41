/*
 * boot.h
 *
 *  Created on: 2020. 1. 27.
 *      Author: Baram
 */

#ifndef SRC_AP_BOOT_BOOT_H_
#define SRC_AP_BOOT_BOOT_H_


#include "hw.h"





void bootInit(void);
void bootProcessCmd(cmd_t *p_cmd);
void bootJumpToFw(uint32_t tag_addr);
bool bootVerifyFw(uint32_t tag_addr);
bool bootVerifyCrc(uint32_t tag_addr);



#endif /* SRC_AP_BOOT_BOOT_H_ */
