/*
 * tag.c
 *
 *  Created on: 2020. 9. 12.
 *      Author: Baram
 */




#include "hw_def.h"


extern uint32_t __vectors_start__;
extern uint32_t _image_start;
extern uint32_t _image_size;

#if 0
typedef struct
{
  uint32_t magic_number;

  //-- fw info
  //
  uint8_t  version_str[32];
  uint8_t  board_str  [32];
  uint8_t  name_str   [32];
  uint8_t  date_str   [32];
  uint8_t  time_str   [32];
  uint32_t addr_tag;
  uint32_t addr_fw;
  uint32_t load_start;
  uint32_t load_size;
  uint32_t reserved   [30];

  //-- tag info
  //
  uint32_t tag_flash_type;
  uint32_t tag_flash_start;
  uint32_t tag_flash_end;
  uint32_t tag_flash_length;
  uint32_t tag_flash_crc;
  uint32_t tag_length;
  uint8_t  tag_date_str[32];
  uint8_t  tag_time_str[32];
} flash_tag_t;
#endif

__attribute__((section(".tag"))) const flash_tag_t fw_tag =
   {
    // fw info
    //
    0xAAAA5555,         // magic_number
    "V200229R1",        // version_str
    "OROCABOY4",        // board_str
    "GNUBOY",           // name_str
    __DATE__,           // date_str
    __TIME__,           // time_str
    (uint32_t)&fw_tag,  // addr_tag
    (uint32_t)&__vectors_start__, // addr_fw

    (uint32_t)&_image_start,  // load_addr
    (uint32_t)&_image_size,   // load_size
    {0,},

    // tag info
    //
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    {0,},
    {0,}
   };
