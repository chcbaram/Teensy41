/*
 * launcher.h
 *
 *  Created on: 2020. 9. 5.
 *      Author: Baram
 */

#ifndef SRC_AP_LAUNCHER_LAUNCHER_H_
#define SRC_AP_LAUNCHER_LAUNCHER_H_


#include "hw.h"


namespace launcher
{

void main(void);
void drawBackground(const char *title_str);
void drawMsgBox(const char *str, uint16_t txt_color, uint32_t timeout);
bool runFile(const char *file_name);
}

#endif /* SRC_AP_LAUNCHER_LAUNCHER_H_ */
