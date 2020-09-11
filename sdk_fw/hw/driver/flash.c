/*
 * flash.c
 *
 *  Created on: 2020. 2. 20.
 *      Author: HanCheol Cho
 */




#include "flash.h"
#include "cmdif.h"

#define FLASH_ADDR_OFFSET         0x60000000
#define FLASH_MAX_SIZE            (8*1024*1024)
#define FLASH_SECTOR_SIZE         (4*1024)
#define FLASH_PAGE_SIZE           (256)
#define FLASH_MAX_SECTOR          (FLASH_MAX_SIZE / FLASH_SECTOR_SIZE)





static bool _flashInit(void);
static bool _flashEraseSector(uint32_t start_sector,  uint32_t sector_cnt);
static bool _flashWritePage(uint32_t addr, uint32_t buf_addr);



#ifdef _USE_HW_CMDIF
void flashCmdifInit(void);
void flashCmdif(void);
#endif




bool flashInit(void)
{

  _flashInit();


#ifdef _USE_HW_CMDIF
  flashCmdifInit();
#endif

  return true;
}

bool flashIsRange(uint32_t addr_begin, uint32_t length)
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

bool flashErase(uint32_t addr, uint32_t length)
{
  bool ret = false;

  int32_t start_sector = -1;
  int32_t end_sector = -1;


#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiErase(addr - qspiGetAddr(), length);
    return ret;
  }
#endif

  if (flashIsRange(addr, length) == true)
  {
    return true;
  }

  if (addr < FLASH_ADDR_OFFSET) return false;
  if (addr >= (FLASH_ADDR_OFFSET + FLASH_MAX_SIZE)) return false;
  if ((addr+length) > (FLASH_ADDR_OFFSET + FLASH_MAX_SIZE)) return false;

  start_sector = -1;
  end_sector = -1;


  for (int i=0; i<FLASH_MAX_SECTOR; i++)
  {
    bool update = false;
    uint32_t start_addr;
    uint32_t end_addr;


    start_addr = FLASH_ADDR_OFFSET + i * FLASH_SECTOR_SIZE;
    end_addr   = start_addr + FLASH_SECTOR_SIZE - 1;

    if (start_addr >= addr && start_addr < (addr+length))
    {
      update = true;
    }
    if (end_addr >= addr && end_addr < (addr+length))
    {
      update = true;
    }

    if (addr >= start_addr && addr <= end_addr)
    {
      update = true;
    }
    if ((addr+length-1) >= start_addr && (addr+length-1) <= end_addr)
    {
      update = true;
    }


    if (update == true)
    {
      if (start_sector < 0)
      {
        start_sector = i;
      }
      end_sector = i;
    }
  }

  if (start_sector >= 0)
  {
    ret = _flashEraseSector(start_sector,  (end_sector - start_sector) + 1);
  }


  return ret;
}

bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint32_t index;
  uint32_t write_length;
  uint32_t write_addr;
  uint8_t buf[FLASH_PAGE_SIZE];
  uint32_t offset;



#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiWrite(addr - qspiGetAddr(), p_data, length);
    return ret;
  }
#endif


  if (flashIsRange(addr, length) == true)
  {
    memcpy((void *)addr, (void *)p_data, length);
    return true;
  }

  index = 0;
  offset = addr%FLASH_PAGE_SIZE;

  if (offset != 0 || length < FLASH_PAGE_SIZE)
  {
    write_addr = addr - offset;
    memcpy(&buf[0], (void *)write_addr, FLASH_PAGE_SIZE);
    memcpy(&buf[offset], &p_data[0], constrain(FLASH_PAGE_SIZE-offset, 0, length));

    ret = _flashWritePage(write_addr, (uint32_t)buf);
    if (ret != true)
    {
      return false;
    }

    if (length < FLASH_PAGE_SIZE)
    {
      index += length;
    }
    else
    {
      index += (FLASH_PAGE_SIZE - offset);
    }
  }


  while(index < length)
  {
    write_length = constrain(length - index, 0, FLASH_PAGE_SIZE);

    ret = _flashWritePage(addr + index, (uint32_t)&p_data[index]);
    if (ret != true)
    {
      ret = false;
      break;
    }

    index += write_length;

    if ((length - index) > 0 && (length - index) < FLASH_PAGE_SIZE)
    {
      offset = length - index;
      write_addr = addr + index;
      memcpy(&buf[0], (void *)write_addr, FLASH_PAGE_SIZE);
      memcpy(&buf[0], &p_data[index], offset);

      ret = _flashWritePage(write_addr, (uint32_t)buf);
      if (ret != true)
      {
        return false;
      }
      break;
    }
  }

  return ret;
}

bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t *p_byte = (uint8_t *)addr;


#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiRead(addr - qspiGetAddr(), p_data, length);
    return ret;
  }
#endif

  if (flashIsRange(addr, length) == true)
  {
    memcpy((void *)p_data, (void *)addr, length);
    return true;
  }

  if (addr < FLASH_ADDR_OFFSET) return false;
  if (addr >= (FLASH_ADDR_OFFSET + FLASH_MAX_SIZE)) return false;
  if ((addr+length) > (FLASH_ADDR_OFFSET + FLASH_MAX_SIZE)) return false;


  for (int i=0; i<length; i++)
  {
    p_data[i] = p_byte[i];
  }

  return ret;
}







#ifdef _USE_HW_CMDIF
void flashCmdifInit(void)
{
  cmdifAdd("flash", flashCmdif);
}

void flashCmdif(void)
{
  bool ret = true;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint8_t  data;
  uint32_t pre_time;
  bool flash_ret;


  if (cmdifGetParamCnt() == 1)
  {
    if(cmdifHasString("info", 0) == true)
    {
      cmdifPrintf("flash addr  : 0x%X\n", FLASH_ADDR_OFFSET);
    }
    else
    {
      ret = false;
    }
  }
  else if (cmdifGetParamCnt() == 3)
  {
    if(cmdifHasString("read", 0) == true)
    {
      addr   = (uint32_t)cmdifGetParam(1);
      length = (uint32_t)cmdifGetParam(2);

      for (i=0; i<length; i++)
      {
        flash_ret = flashRead(addr+i, &data, 1);

        if (flash_ret == true)
        {
          cmdifPrintf( "addr : 0x%X\t 0x%02X\n", addr+i, data);
        }
        else
        {
          cmdifPrintf( "addr : 0x%X\t Fail\n", addr+i);
        }
      }
    }
    else if(cmdifHasString("erase", 0) == true)
    {
      addr   = (uint32_t)cmdifGetParam(1);
      length = (uint32_t)cmdifGetParam(2);

      pre_time = millis();
      flash_ret = flashErase(addr, length);

      cmdifPrintf( "addr : 0x%X\t len : %d %d ms\n", addr, length, (millis()-pre_time));
      if (flash_ret)
      {
        cmdifPrintf("OK\n");
      }
      else
      {
        cmdifPrintf("FAIL\n");
      }
    }
    else if(cmdifHasString("write", 0) == true)
    {
      addr = (uint32_t)cmdifGetParam(1);
      data = (uint8_t )cmdifGetParam(2);

      pre_time = millis();
      flash_ret = flashWrite(addr, &data, 1);

      cmdifPrintf( "addr : 0x%X\t 0x%02X %dus\n", addr, data, millis()-pre_time);
      if (flash_ret)
      {
        cmdifPrintf("OK\n");
      }
      else
      {
        cmdifPrintf("FAIL\n");
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }


  if (ret == false)
  {
    cmdifPrintf( "flash info\n");
    cmdifPrintf( "flash read  [addr] [length]\n");
    cmdifPrintf( "flash erase [addr] [length]\n");
    cmdifPrintf( "flash write [addr] [data]\n");
  }

}
#endif







#include "fsl_flexspi.h"
#include "fsl_rtwdog.h"
#include "fsl_wdog.h"
#include "xip_boot/xip_flexspi_nor_config.h"

typedef struct {
     void (*RTWDOG_GetDefaultConfig)(rtwdog_config_t *config);
     void (*RTWDOG_Init)(RTWDOG_Type *base, const rtwdog_config_t *config);
     void (*RTWDOG_Deinit)(RTWDOG_Type *base);
     void (*RTWDOG_Enable)(RTWDOG_Type *base);
     void (*RTWDOG_Disable)(RTWDOG_Type *base);
     void (*RTWDOG_EnableInterrupts)(RTWDOG_Type *base, uint32_t mask);
     void (*RTWDOG_DisableInterrupts)(RTWDOG_Type *base, uint32_t mask);
     uint32_t (*RTWDOG_GetStatusFlags)(RTWDOG_Type *base);
     void (*RTWDOG_ClearStatusFlags)(RTWDOG_Type *base, uint32_t mask);
     void (*RTWDOG_SetTimeoutValue)(RTWDOG_Type *base, uint16_t timeoutCount);
     void (*RTWDOG_SetWindowValue)(RTWDOG_Type *base, uint16_t windowValue);
     void (*RTWDOG_Unlock)(RTWDOG_Type *base);
     void (*RTWDOG_Refresh)(RTWDOG_Type *base);
     uint16_t (*RTWDOG_GetCounterValue)(RTWDOG_Type *base);
} rtwdog_driver_interface_t;

typedef struct {
     void (*WDOG_GetDefaultConfig)(wdog_config_t *config);
     void (*WDOG_Init)(WDOG_Type *base, const wdog_config_t *config);
     void (*WDOG_Deinit)(WDOG_Type *base);
     void (*WDOG_Enable)(WDOG_Type *base);
     void (*WDOG_Disable)(WDOG_Type *base);
     void (*WDOG_EnableInterrupts)(WDOG_Type *base, uint16_t mask);
     uint16_t (*WDOG_GetStatusFlags)(WDOG_Type *base);
     void (*WDOG_ClearInterruptStatus)(WDOG_Type *base, uint16_t mask);
     void (*WDOG_SetTimeoutValue)(WDOG_Type *base, uint16_t timeoutCount);
     void (*WDOG_SetInterrputTimeoutValue)(WDOG_Type *base, uint16_t timeoutCount);
     void (*WDOG_DisablePowerDownEnable)(WDOG_Type *base);
     void (*WDOG_Refresh)(WDOG_Type *base);
} wdog_driver_interface_t;

typedef struct _serial_nor_config_option {
     union {
          struct {
               uint32_t max_freq : 4; //!< Maximum supported Frequency
               uint32_t misc_mode : 4; //!< miscellaneous mode
               uint32_t quad_mode_setting : 4; //!< Quad mode setting
               uint32_t cmd_pads : 4; //!< Command pads
               uint32_t query_pads : 4; //!< SFDP read pads
               uint32_t device_type : 4; //!< Device type
               uint32_t option_size : 4; //!< Option size, in terms of uint32_t, size = (option_size + 1) * 4
               uint32_t tag : 4; //!< Tag, must be 0x0E
          } B;
          uint32_t U;
     } option0;
     union {
          struct {
               uint32_t dummy_cycles : 8; //!< Dummy cycles before read
               uint32_t reserved0 : 8; //!< Reserved for future use
               uint32_t pinmux_group : 4; //!< The pinmux group selection
               uint32_t reserved1 : 8; //!< Reserved for future use
               uint32_t flash_connection : 4; //!< Flash connection option: 0 - Single Flash connected to port A
          } B;
          uint32_t U;
     } option1;
} serial_nor_config_option_t;


typedef enum _FlexSPIOperationType {
     kFlexSpiOperation_Command, //!< FlexSPI operation: Only command, both TX and RX buffer are ignored.
     kFlexSpiOperation_Config, //!< FlexSPI operation: Configure device mode, the TX FIFO size is fixed in LUT.
     kFlexSpiOperation_Write, //!< FlexSPI operation: Write, only TX buffer is effective
     kFlexSpiOperation_Read, //!< FlexSPI operation: Read, only Rx Buffer is effective.
     kFlexSpiOperation_End = kFlexSpiOperation_Read,
} flexspi_operation_t;

typedef struct _FlexSpiXfer {
     flexspi_operation_t operation; //!< FlexSPI operation
     uint32_t baseAddress; //!< FlexSPI operation base address
     uint32_t seqId; //!< Sequence Id
     uint32_t seqNum; //!< Sequence Number
     bool isParallelModeEnable; //!< Is a parallel transfer
     uint32_t *txBuffer; //!< Tx buffer
     uint32_t txSize; //!< Tx size in bytes
     uint32_t *rxBuffer; //!< Rx buffer
     uint32_t rxSize; //!< Rx size in bytes
} flexspi_xfer_t;


typedef struct {
     uint32_t version;
     status_t (*init)(uint32_t instance, flexspi_nor_config_t *config);
     status_t (*program)(uint32_t instance, flexspi_nor_config_t *config, uint32_t dst_addr, const uint32_t *src);
     status_t (*erase_all)(uint32_t instance, flexspi_nor_config_t *config);
     status_t (*erase)(uint32_t instance, flexspi_nor_config_t *config, uint32_t start, uint32_t lengthInBytes);
     status_t (*read)(uint32_t instance, flexspi_nor_config_t *config, uint32_t *dst, uint32_t addr, uint32_t lengthInBytes);
     void (*clear_cache)(uint32_t instance);
     status_t (*xfer)(uint32_t instance, flexspi_xfer_t *xfer);
     status_t (*update_lut)(uint32_t instance, uint32_t seqIndex, const uint32_t *lutBase, uint32_t seqNumber);
     status_t (*get_config)(uint32_t instance, flexspi_nor_config_t *config, serial_nor_config_option_t *option);
} flexspi_nor_driver_interface_t;

typedef struct {
     const uint32_t version; //!< Bootloader version number
     const char *copyright; //!< Bootloader Copyright
     void (*runBootloader)(void *arg); //!< Function to start the bootloader executing
     const uint32_t *reserved0; //!< Reserved
     const flexspi_nor_driver_interface_t *flexSpiNorDriver; //!< FlexSPI NOR Flash API
     const uint32_t *reserved1[2]; //!< Reserved
     const rtwdog_driver_interface_t *rtwdogDriver;
     const wdog_driver_interface_t *wdogDriver;
     const uint32_t *reserved2;
} bootloader_api_entry_t;
#define g_bootloaderTree (*(bootloader_api_entry_t**)(0x0020001c))








//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_flash_init(uint32_t instance, flexspi_nor_config_t *config) {
     return g_bootloaderTree->flexSpiNorDriver->init(instance, config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_flash_page_program(uint32_t instance, flexspi_nor_config_t*config, uint32_t dstAddr, const uint32_t *src) {
     return g_bootloaderTree->flexSpiNorDriver->program(instance, config, dstAddr, src);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_flash_erase_all(uint32_t instance, flexspi_nor_config_t *config) {
     return g_bootloaderTree->flexSpiNorDriver->erase_all(instance, config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_get_config(uint32_t instance, flexspi_nor_config_t *config, serial_nor_config_option_t *option) {
     return g_bootloaderTree->flexSpiNorDriver->get_config(instance, config, option);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_flash_erase(uint32_t instance, flexspi_nor_config_t *config, uint32_t start, uint32_t length) {
     return g_bootloaderTree->flexSpiNorDriver->erase(instance, config, start, length);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
status_t flexspi_nor_flash_read(uint32_t instance, flexspi_nor_config_t *config, uint32_t *dst, uint32_t start, uint32_t bytes) {
     return g_bootloaderTree->flexSpiNorDriver->read(instance, config, dst, start, bytes);
}




static flexspi_nor_config_t config;
static serial_nor_config_option_t option;
static status_t status;
static uint32_t instance = 0; // Should identify NOR memory instance



bool _flashInit(void)
{
  //option.option0.U = 0xC0000008; // QuadSPI NOR, Frequency: 133MHz
  option.option0.U = 0xC0100003; // QuadSPI NOR, Frequency: 133MHz



  // Need to run with interrupts disabled as all our code is running out of FlexSPI2 (internal NOT flash in RT1064)
  __disable_irq();


  status = flexspi_nor_get_config(instance, &config, &option);
  if (status != kStatus_Success)
  {
    __enable_irq();
    return false;
  }
#if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
  status = flexspi_nor_flash_init(instance, &config);
  if (status != kStatus_Success)
  {
    __enable_irq();
    return false;
  }
#endif
  __enable_irq();
  return true;
}

bool _flashEraseSector(uint32_t start_sector,  uint32_t sector_cnt)
{
  uint32_t start_addr;

  __disable_irq();

  start_addr = start_sector * FLASH_SECTOR_SIZE;


  status = flexspi_nor_flash_erase(instance, &config, start_addr, sector_cnt * FLASH_SECTOR_SIZE);
  if (status != kStatus_Success)
  {
    __enable_irq();
    return false;
  }

  __enable_irq();

  SCB_InvalidateDCache_by_Addr((void *)(start_addr + FLASH_ADDR_OFFSET), sector_cnt * FLASH_SECTOR_SIZE);

  return true;
}

bool _flashWritePage(uint32_t addr, uint32_t buf_addr)
{
  uint32_t buf[FLASH_PAGE_SIZE/4];
  uint8_t *p_dst;
  uint8_t *p_src;

  p_dst = (uint8_t *)buf;
  p_src = (uint8_t *)buf_addr;

  for (int i=0; i<FLASH_PAGE_SIZE; i++)
  {
    p_dst[i] = p_src[i];
  }

  __disable_irq();
  status = flexspi_nor_flash_page_program(instance, &config, addr-FLASH_ADDR_OFFSET, buf);
  if (status != kStatus_Success)
  {
    __enable_irq();
    return status;
  }

  __enable_irq();

  SCB_InvalidateDCache_by_Addr((void *)addr, FLASH_PAGE_SIZE);

  return true;
}

#if 0
bool _flashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  __disable_irq();
  status = flexspi_nor_flash_read(instance, &config, (uint32_t *)p_data, addr-FLASH_ADDR_OFFSET, length);

  if (status != kStatus_Success)
  {
    __enable_irq();
    return status;
  }

  __enable_irq();

  return true;
}
#endif
