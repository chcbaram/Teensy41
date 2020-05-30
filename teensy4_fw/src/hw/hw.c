/*
 * hw.c
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "hw.h"


extern uint32_t _image_start;
extern uint32_t _image_end;
extern uint32_t _image_size;


__attribute__((aligned(2048))) __attribute__((used, section(".tag"))) const boot_tag_t boot_tag =
    {
        .boot_name    = "RT1052_FSBL",
        .boot_ver     = "B2000312R1",
        .magic_number = 0x5555AAAA,
        .image_start  = (uint32_t)&_image_start,
        .image_end    = (uint32_t)&_image_end,
        .image_size   = (uint32_t)&_image_size,
    };


void hwInit(void)
{
  bspInit();

  microsInit();
  ledInit();
}
