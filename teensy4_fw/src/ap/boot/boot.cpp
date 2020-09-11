/*
 * boot.cpp
 *
 *  Created on: 2020. 1. 27.
 *      Author: Baram
 */





#include "boot.h"


#define FLASH_TAG_SIZE      0x400


extern boot_tag_t boot_tag;



#define BOOT_CMD_READ_BOOT_VERSION      0x00
#define BOOT_CMD_READ_BOOT_NAME         0x01
#define BOOT_CMD_READ_FIRM_VERSION      0x02
#define BOOT_CMD_READ_FIRM_NAME         0x03
#define BOOT_CMD_FLASH_ERASE            0x04
#define BOOT_CMD_FLASH_WRITE            0x05
#define BOOT_CMD_FLASH_VERIFY           0x06
#define BOOT_CMD_FLASH_READ             0x07
#define BOOT_CMD_JUMP_TO_FW             0x08



static void bootCmdReadBootVersion(cmd_t *p_cmd);
static void bootCmdReadBootName(cmd_t *p_cmd);
static void bootCmdReadFirmVersion(cmd_t *p_cmd);
static void bootCmdReadFirmName(cmd_t *p_cmd);
static void bootCmdFlashErase(cmd_t *p_cmd);
static void bootCmdFlashWrite(cmd_t *p_cmd);
static void bootCmdFlashRead(cmd_t *p_cmd);
static void bootCmdJumpToFw(cmd_t *p_cmd);
static bool bootIsFlashRange(uint32_t addr_begin, uint32_t length);


bool bootCheckFw(void);



static flash_tag_t  *p_fw_tag = (flash_tag_t *)FLASH_ADDR_TAG;


void bootInit(void)
{
}

void bootJumpToFw(uint32_t tag_addr)
{
  flash_tag_t *p_tag;
  void (**jump_func)(void);


  p_tag = (flash_tag_t *)tag_addr;

  jump_func = (void (**)(void))(p_tag->addr_fw + 4);


  if (p_tag->addr_fw != p_tag->load_start)
  {
    memcpy((void *)p_tag->addr_fw, (const void *)p_tag->load_start, p_tag->load_size);
    SCB_InvalidateDCache_by_Addr((void *)(p_tag->addr_fw), p_tag->load_size);
  }

  delay(100);
  bspDeInit();

  __set_MSP(*(uint32_t *)p_tag->addr_fw);
  (*jump_func)();
}

bool bootVerifyFw(uint32_t tag_addr)
{
  flash_tag_t *p_tag;
  void (**jump_func)(void);

  p_tag = (flash_tag_t *)tag_addr;
  jump_func = (void (**)(void))(p_tag->addr_fw + 4);

  if ((uint32_t)(*jump_func) != 0xFFFFFFFF)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool bootVerifyMagicNumber(uint32_t tag_addr)
{
  flash_tag_t *p_tag;

  p_tag = (flash_tag_t *)tag_addr;


  if (p_tag->magic_number == 0xAAAA5555)
  {
    return true;
  }

  if (p_tag->magic_number != FLASH_MAGIC_NUMBER)
  {
    return false;
  }

  return true;
}

bool bootVerifyCrc(uint32_t tag_addr)
{
  uint32_t i;
  uint8_t *p_data;
  uint16_t fw_crc;
  flash_tag_t *p_tag;


  p_tag = (flash_tag_t *)tag_addr;

  if (p_tag->magic_number == 0xAAAA5555)
  {
    //logPrintf("empty crc\n");
    return true;
  }



  if (p_tag->addr_fw == p_tag->load_start)
  {
    p_data = (uint8_t *)p_fw_tag->tag_flash_start;
  }
  else
  {
    p_data = (uint8_t *)(p_fw_tag->addr_tag + FLASH_TAG_SIZE);
  }


  fw_crc = 0;

  for (i=0; i<p_fw_tag->tag_flash_length; i++)
  {
    utilUpdateCrc(&fw_crc, p_data[i]);
  }

  if (fw_crc == p_fw_tag->tag_flash_crc)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void bootCmdReadBootVersion(cmd_t *p_cmd)
{
  cmdSendResp(p_cmd, OK, (uint8_t *)boot_tag.boot_ver, 32);
}

void bootCmdReadBootName(cmd_t *p_cmd)
{
  cmdSendResp(p_cmd, OK, (uint8_t *)boot_tag.boot_name, 32);
}

void bootCmdReadFirmVersion(cmd_t *p_cmd)
{
  cmdSendResp(p_cmd, OK, p_fw_tag->version_str, 32);
}

void bootCmdReadFirmName(cmd_t *p_cmd)
{
  cmdSendResp(p_cmd, OK, p_fw_tag->board_str, 32);
}

void bootCmdFlashErase(cmd_t *p_cmd)
{
  uint8_t err_code = OK;
  uint32_t addr_begin;
  uint32_t length;


  addr_begin  = p_cmd->rx_packet.data[0]<<0;
  addr_begin |= p_cmd->rx_packet.data[1]<<8;
  addr_begin |= p_cmd->rx_packet.data[2]<<16;
  addr_begin |= p_cmd->rx_packet.data[3]<<24;

  length      = p_cmd->rx_packet.data[4]<<0;
  length     |= p_cmd->rx_packet.data[5]<<8;
  length     |= p_cmd->rx_packet.data[6]<<16;
  length     |= p_cmd->rx_packet.data[7]<<24;

  if (bootIsFlashRange(addr_begin, length) == true)
  {
    if (flashErase(addr_begin, length) == false)
    {
      err_code = ERR_FLASH_ERASE;
    }
  }
  else
  {
    err_code = ERR_FLASH_INVALID_ADDR;
  }

  cmdSendResp(p_cmd, err_code, NULL, 0);

}

void bootCmdFlashWrite(cmd_t *p_cmd)
{
  uint8_t err_code = OK;
  uint32_t addr_begin;
  uint32_t length;


  addr_begin  = p_cmd->rx_packet.data[0]<<0;
  addr_begin |= p_cmd->rx_packet.data[1]<<8;
  addr_begin |= p_cmd->rx_packet.data[2]<<16;
  addr_begin |= p_cmd->rx_packet.data[3]<<24;

  length      = p_cmd->rx_packet.data[4]<<0;
  length     |= p_cmd->rx_packet.data[5]<<8;
  length     |= p_cmd->rx_packet.data[6]<<16;
  length     |= p_cmd->rx_packet.data[7]<<24;


  if (bootIsFlashRange(addr_begin, length) == true)
  {
    if (flashWrite(addr_begin, &p_cmd->rx_packet.data[8], length) == false)
    {
      err_code = ERR_FLASH_WRITE;
    }
  }
  else
  {
    err_code = ERR_FLASH_INVALID_ADDR;
  }

  cmdSendResp(p_cmd, err_code, NULL, 0);
}

void bootCmdFlashRead(cmd_t *p_cmd)
{
  uint8_t err_code = OK;
  uint32_t addr_begin;
  uint32_t length;


  addr_begin  = p_cmd->rx_packet.data[0]<<0;
  addr_begin |= p_cmd->rx_packet.data[1]<<8;
  addr_begin |= p_cmd->rx_packet.data[2]<<16;
  addr_begin |= p_cmd->rx_packet.data[3]<<24;

  length      = p_cmd->rx_packet.data[4]<<0;
  length     |= p_cmd->rx_packet.data[5]<<8;
  length     |= p_cmd->rx_packet.data[6]<<16;
  length     |= p_cmd->rx_packet.data[7]<<24;


  if (length < (CMD_MAX_DATA_LENGTH - 5))
  {
    cmdSendResp(p_cmd, err_code, (uint8_t *)addr_begin, length);
  }
  else
  {
    cmdSendResp(p_cmd, ERR_INVALID_LENGTH, NULL, 0);
  }
}

void bootCmdJumpToFw(cmd_t *p_cmd)
{
  if (bootVerifyFw(FLASH_ADDR_TAG) != true)
  {
    cmdSendResp(p_cmd, ERR_INVALID_FW, NULL, 0);
  }
  else if (bootVerifyMagicNumber(FLASH_ADDR_TAG) != true)
  {
    cmdSendResp(p_cmd, ERR_INVALID_MAGIC_NUMBER, NULL, 0);
  }
  else if (bootVerifyCrc(FLASH_ADDR_TAG) != true)
  {
    cmdSendResp(p_cmd, ERR_FLASH_CRC, NULL, 0);
  }
  else
  {
    cmdSendResp(p_cmd, OK, NULL, 0);
    delay(100);
    bootJumpToFw(FLASH_ADDR_TAG);
  }
}

void bootProcessCmd(cmd_t *p_cmd)
{
  switch(p_cmd->rx_packet.cmd)
  {
    case BOOT_CMD_READ_BOOT_VERSION:
      bootCmdReadBootVersion(p_cmd);
      break;

    case BOOT_CMD_READ_BOOT_NAME:
      bootCmdReadBootName(p_cmd);
      break;

    case BOOT_CMD_READ_FIRM_VERSION:
      bootCmdReadFirmVersion(p_cmd);
      break;

    case BOOT_CMD_READ_FIRM_NAME:
      bootCmdReadFirmName(p_cmd);
      break;

    case BOOT_CMD_FLASH_ERASE:
      bootCmdFlashErase(p_cmd);
      break;

    case BOOT_CMD_FLASH_WRITE:
      bootCmdFlashWrite(p_cmd);
      break;

    case BOOT_CMD_FLASH_READ:
      bootCmdFlashRead(p_cmd);
      break;

    case BOOT_CMD_JUMP_TO_FW:
      bootCmdJumpToFw(p_cmd);
      break;


    default:
      cmdSendResp(p_cmd, ERR_INVALID_CMD, NULL, 0);
      break;
  }
}

bool bootIsFlashRange(uint32_t addr_begin, uint32_t length)
{
  bool ret = false;
  uint32_t addr_end;
  uint32_t flash_start;
  uint32_t flash_end;


  addr_end = addr_begin + length - 1;


  flash_start = FLASH_ADDR_START;
  flash_end   = FLASH_ADDR_END;
  if ((addr_begin >= flash_start) && (addr_begin < flash_end) &&
      (addr_end   >= flash_start) && (addr_end   < flash_end))
  {
    ret = true;
  }

  return ret;
}
