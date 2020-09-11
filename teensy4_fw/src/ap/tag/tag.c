/*
 * tag.c
 *
 *  Created on: 2020. 9. 12.
 *      Author: Baram
 */


#include "hw_def.h"


extern uint32_t _image_start;
extern uint32_t _image_end;
extern uint32_t _image_size;



__attribute__((aligned(2048))) __attribute__((used, section(".tag"))) const boot_tag_t boot_tag =
    {
        "OROCABOY4_LAUNCHER",
        "V200912R1",
        0x5555AAAA,
        0x60000000,
        (uint32_t)&_image_start,
        (uint32_t)&_image_end,
        (uint32_t)&_image_size,
    };
