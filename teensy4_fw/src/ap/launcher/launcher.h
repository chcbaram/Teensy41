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
bool runFile(const char *file_name);
}

#endif /* SRC_AP_LAUNCHER_LAUNCHER_H_ */
