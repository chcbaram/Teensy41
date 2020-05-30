/*
 * psram.c
 *
 *  Created on: 2020. 5. 30.
 *      Author: Baram
 */




#include "psram.h"
#include "cmdif.h"
#include "fsl_cache.h"
#include "fsl_flexspi.h"



#define PSRAM_MAX_CH              2


#define PSRAM_ADDR_OFFSET         0x70000000
#define PSRAM_MAX_SIZE            (8*1024*1024)
#define PSRAM_SECTOR_SIZE         (4*1024)
#define PSRAM_PAGE_SIZE           (1024)
#define PSRAM_MAX_SECTOR          (FLASH_MAX_SIZE / FLASH_SECTOR_SIZE)



static FLEXSPI_Type *p_flexspi = FLEXSPI2;
static uint32_t psram_addr   = PSRAM_ADDR_OFFSET;
static uint32_t psram_length = 0;

typedef struct
{
  bool     is_init;
  uint32_t id;
  uint32_t length;

} psram_tbl_t;


static psram_tbl_t psram_tbl[PSRAM_MAX_CH];



static bool psramSetup(uint8_t ch);
static status_t flexspiGetVendorID(uint8_t ch, uint8_t *vendorId);
static void flexspiInit(uint8_t ch);
static status_t flexspiEnterQPI(uint8_t ch);
static status_t flexspiExitQPI(uint8_t ch);


#ifdef _USE_HW_CMDIF
void psramCmdif(void);
#endif

bool psramInit(void)
{
  bool ret = true;
  uint32_t i;



  for (i=0; i<PSRAM_MAX_CH; i++)
  {
    psram_tbl[i].is_init = false;
    psram_tbl[i].length = 0;


    if (psramSetup(i) == true)
    {
      psram_length += PSRAM_MAX_SIZE;
    }
  }

#ifdef _USE_HW_CMDIF
  cmdifAdd("psram", psramCmdif);
#endif

  return ret;
}


bool psramSetup(uint8_t ch)
{
  bool ret = true;
  status_t status;


  flexspiInit(ch);
  flexspiExitQPI(ch);

  psram_tbl[ch].is_init = false;
  psram_tbl[ch].id = 0;
  status = flexspiGetVendorID(ch, (uint8_t *)&psram_tbl[ch].id);

  if (status == 0 && psram_tbl[ch].id == 0x5D0D)
  {
    psram_tbl[ch].is_init = true;
    psram_tbl[ch].length = PSRAM_MAX_SIZE;

    flexspiEnterQPI(ch);
  }


  ret = psram_tbl[ch].is_init;

  return ret;
}

uint32_t psramGetAddr(void)
{
  return psram_addr;
}

uint32_t psramGetLength(void)
{
  return psram_length;
}




#define PSRAM_CMD_LUT_SEQ_IDX_ENTERQPI          0
#define PSRAM_CMD_LUT_SEQ_IDX_EXITQPI           1
#define PSRAM_CMD_LUT_SEQ_IDX_READID            2
#define PSRAM_CMD_LUT_SEQ_IDX_READ_QPI          4
#define PSRAM_CMD_LUT_SEQ_IDX_WRITE_QPI         6


#define CUSTOM_LUT_LENGTH                       (60)



flexspi_device_config_t deviceconfig =
{
    .flexspiRootClk       = 99000000,
    .flashSize            = PSRAM_MAX_SIZE/1024,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 3,
    .CSSetupTime          = 3,
    .dataValidTime        = 0,
    .columnspace          = 0,
    .enableWordAddress    = 0,

    .AWRSeqIndex          = PSRAM_CMD_LUT_SEQ_IDX_WRITE_QPI,
    .AWRSeqNumber         = 1,

    .ARDSeqIndex          = PSRAM_CMD_LUT_SEQ_IDX_READ_QPI,
    .ARDSeqNumber         = 1,

    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};



const uint32_t customLUT[CUSTOM_LUT_LENGTH] =
{
    /* Enter QPI mode */
    [4 * PSRAM_CMD_LUT_SEQ_IDX_ENTERQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x35, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Exit QPI mode */
    [4 * PSRAM_CMD_LUT_SEQ_IDX_EXITQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xF5, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read ID */
    [4 * PSRAM_CMD_LUT_SEQ_IDX_READID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x9F, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 24),
    [4 * PSRAM_CMD_LUT_SEQ_IDX_READID + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),


    /* Read QPI */
    [4 * PSRAM_CMD_LUT_SEQ_IDX_READ_QPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xEB, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 24),
    [4 * PSRAM_CMD_LUT_SEQ_IDX_READ_QPI + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x06, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),


    /* Write QPI */
    [4 * PSRAM_CMD_LUT_SEQ_IDX_WRITE_QPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x38, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 24),
    [4 * PSRAM_CMD_LUT_SEQ_IDX_WRITE_QPI + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_4PAD, 0),
};



static void flexspiInit(uint8_t ch)
{
  static bool init = false;

  flexspi_config_t config;

  /*Get FLEXSPI default settings and configure the flexspi. */
  FLEXSPI_GetDefaultConfig(&config);

  if (init != true)
  {
    /*Set AHB buffer size for reading data through AHB bus. */
    config.ahbConfig.enableAHBPrefetch    = true;
    config.ahbConfig.enableAHBBufferable  = true;
    config.ahbConfig.enableReadAddressOpt = true;
    config.ahbConfig.enableAHBCachable    = true;
    config.rxSampleClock                  = kFLEXSPI_ReadSampleClkLoopbackFromDqsPad;
    FLEXSPI_Init(p_flexspi, &config);

    init = true;
  }

  if (ch == 0)
  {
    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(p_flexspi, &deviceconfig, kFLEXSPI_PortA1);
  }
  else
  {
    FLEXSPI_SetFlashConfig(p_flexspi, &deviceconfig, kFLEXSPI_PortA2);
  }

  /* Update LUT table. */
  FLEXSPI_UpdateLUT(p_flexspi, 0, customLUT, CUSTOM_LUT_LENGTH);

  /* Do software reset. */
  FLEXSPI_SoftwareReset(p_flexspi);
}

status_t flexspiGetVendorID(uint8_t ch, uint8_t *vendorId)
{
  uint32_t temp;
  flexspi_transfer_t flashXfer;

  if (ch == 0)
  {
    flashXfer.port        = kFLEXSPI_PortA1;
    flashXfer.deviceAddress = 0;
  }
  else
  {
    flashXfer.port        = kFLEXSPI_PortA2;
    flashXfer.deviceAddress = 0;
  }
  flashXfer.cmdType       = kFLEXSPI_Read;
  flashXfer.SeqNumber     = 1;
  flashXfer.seqIndex      = PSRAM_CMD_LUT_SEQ_IDX_READID;
  flashXfer.data          = &temp;
  flashXfer.dataSize      = 2;

  status_t status = FLEXSPI_TransferBlocking(p_flexspi, &flashXfer);

  vendorId[0] = temp;
  vendorId[1] = temp>>8;

  /* Do software reset. */
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT)
  base->AHBCR |= FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK;
  base->AHBCR &= ~(FLEXSPI_AHBCR_CLRAHBRXBUF_MASK | FLEXSPI_AHBCR_CLRAHBTXBUF_MASK);
#else
  FLEXSPI_SoftwareReset(p_flexspi);
#endif

  return status;
}


status_t flexspiEnterQPI(uint8_t ch)
{
  flexspi_transfer_t flashXfer;
  if (ch == 0)
  {
    flashXfer.port        = kFLEXSPI_PortA1;
    flashXfer.deviceAddress = 0;
  }
  else
  {
    flashXfer.port        = kFLEXSPI_PortA2;
    flashXfer.deviceAddress = 0;;
  }
  flashXfer.cmdType       = kFLEXSPI_Command;
  flashXfer.SeqNumber     = 1;
  flashXfer.seqIndex      = PSRAM_CMD_LUT_SEQ_IDX_ENTERQPI;

  status_t status = FLEXSPI_TransferBlocking(p_flexspi, &flashXfer);

  return status;
}

status_t flexspiExitQPI(uint8_t ch)
{
  flexspi_transfer_t flashXfer;

  if (ch == 0)
  {
    flashXfer.port        = kFLEXSPI_PortA1;
    flashXfer.deviceAddress = 0;
  }
  else
  {
    flashXfer.port        = kFLEXSPI_PortA2;
    flashXfer.deviceAddress = 0;;
  }
  flashXfer.cmdType       = kFLEXSPI_Command;
  flashXfer.SeqNumber     = 1;
  flashXfer.seqIndex      = PSRAM_CMD_LUT_SEQ_IDX_EXITQPI;

  status_t status = FLEXSPI_TransferBlocking(p_flexspi, &flashXfer);

  return status;
}

#ifdef _USE_HW_CMDIF
void psramCmdif(void)
{
  bool ret = true;
  uint8_t number;
  uint32_t i;
  uint32_t pre_time;


  if (cmdifGetParamCnt() == 1)
  {
    if(cmdifHasString("info", 0) == true)
    {
      cmdifPrintf("PSRAM Add : 0x%X\n", psram_addr);
      cmdifPrintf("PSRAM Len : %dKB\n", psram_length/1024);

      for (int i=0; i<PSRAM_MAX_CH; i++)
      {
        cmdifPrintf("PSRAM CH%d, Init: %d, 0x%X, Len : %dKB\n", i+1,
                    psram_tbl[i].is_init,
                    psram_tbl[i].id,
                    psram_tbl[i].length/1024);
      }
    }
    else
    {
      ret = false;
    }
  }
  else if (cmdifGetParamCnt() == 2)
  {
    if(cmdifHasString("test", 0) == true)
    {
      uint32_t *p_data = (uint32_t *)psram_addr;

      number = (uint8_t)cmdifGetParam(1);

      while(number > 0)
      {
        pre_time = millis();
        for (i=0; i<psram_length/4; i++)
        {
          p_data[i] = i;
        }
        cmdifPrintf( "Write : %d MB/s\n", psram_length / 1000 / (millis()-pre_time) );

        volatile uint32_t data_sum = 0;
        pre_time = millis();
        for (i=0; i<psram_length/4; i++)
        {
          data_sum += p_data[i];
        }
        cmdifPrintf( "Read : %d MB/s\n", psram_length / 1000 / (millis()-pre_time) );


        for (i=0; i<psram_length/4; i++)
        {
          if (p_data[i] != i)
          {
            cmdifPrintf( "%d : 0x%X fail\n", i, p_data[i]);
            break;
          }
        }

        if (i == psram_length/4)
        {
          cmdifPrintf( "Count %d\n", number);
          cmdifPrintf( "PSRAM %d MB OK\n\n", psram_length/1024/1024);
          for (i=0; i<psram_length/4; i++)
          {
            p_data[i] = 0x5555AAAA;
          }
        }

        number--;

        if (cmdifRxAvailable() > 0)
        {
          cmdifPrintf( "Stop test...\n");
          break;
        }
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
    cmdifPrintf( "psram info \n");
    cmdifPrintf( "psram test 1~\n");
  }
}
#endif





