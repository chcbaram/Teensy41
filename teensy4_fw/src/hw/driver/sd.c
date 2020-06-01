/*
 * sd.c
 *
 *  Created on: 2020. 2. 21.
 *      Author: Baram
 */




#include "sd.h"
#include "fsl_sd.h"


#ifdef _USE_HW_SD
#include "cmdif.h"


//-- Internal Variables
//
static bool is_init = false;
sd_card_t g_sd;



//-- External Variables
//


//-- Internal Functions
//
#ifdef _USE_HW_CMDIF
void sdCmdifInit(void);
void sdCmdif(void);
#endif

//static void sdInitHw(void);


//-- External Functions
//




bool sdInit(void)
{
  sd_card_t *card = &g_sd;
  status_t status;

  card->host.base           = USDHC1;
  card->host.sourceClock_Hz = (CLOCK_GetSysPfdFreq(kCLOCK_Pfd0) / (CLOCK_GetDiv(kCLOCK_Usdhc1Div) + 1U));
  card->usrParam.cd = NULL;
  card->usrParam.pwr = NULL;



  if (sdIsDetected() != true)
  {
    logPrintf("sdCard     \t\t: not connected\r\n");
    return false;
  }
  else
  {
    logPrintf("sdCard     \t\t: connected\r\n");
  }

  status = SD_HostInit(card);
  if (status != kStatus_Success)
  {
    logPrintf("sdCard     \t\t: fail, %d\r\n", (int)status);
    return false;
  }
  else
  {
    SDMMCHOST_CARD_DETECT_DATA3_ENABLE(card->host.base, true);

    status = SD_CardInit(card);
    if (status != kStatus_Success)
    {
      logPrintf("sdCardInit  \t\t: fail, %d\r\n", (int)status);
      return false;
    }

    logPrintf("sdCard     \t\t: OK\r\n");
  }

  is_init = true;


#ifdef _USE_HW_CMDIF
  static bool is_cmd_init = false;

  if (is_cmd_init == false)
  {
    sdCmdifInit();
    is_cmd_init = true;
  }
#endif

  return is_init;
}

bool sdDeInit(void)
{
  bool ret = true;
  sd_card_t *card = &g_sd;

  if (is_init != true)
  {
    return false;
  }

  SD_CardDeinit(card);

  is_init = false;

  return ret;
}


bool sdReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  sd_card_t *card = &g_sd;

  if (is_init != true)
  {
    return false;
  }

  if (SD_ReadBlocks(card, p_data, block_addr, num_of_blocks) == kStatus_Success )
  {
    ret = true;
  }

  return ret;
}

bool sdWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  sd_card_t *card = &g_sd;


  if (is_init != true)
  {
    return false;
  }

  if (SD_WriteBlocks(card, p_data, block_addr, num_of_blocks) == kStatus_Success)
  {
    ret = true;
  }

  return ret;
}

bool sdEraseBlocks(uint32_t start_addr, uint32_t end_addr)
{
  bool ret = false;
  sd_card_t *card = &g_sd;


  if (SD_EraseBlocks(card, start_addr, (end_addr - start_addr) + 1) == kStatus_Success)
  {
    ret = true;
  }

  return ret;
}

bool sdIsBusy(void)
{
  bool is_busy = false;

  return is_busy;
}

bool sdIsDetected(void)
{
  bool ret = true;

  return ret;
}

bool sdGetInfo(sd_info_t *p_info)
{
  bool ret = false;
  sd_card_t *card = &g_sd;


  if (is_init == true)
  {
    p_info->card_type          = card->operationVoltage;
    p_info->freq               = card->busClock_Hz;
    p_info->card_version       = card->version;
    p_info->card_class         = card->currentTiming;
    p_info->rel_card_Add       = card->relativeAddress;
    p_info->block_numbers      = card->blockCount;
    p_info->block_size         = card->blockSize;
    p_info->card_size          =  (uint32_t)((uint64_t)card->blockCount * (uint64_t)card->blockSize / (uint64_t)1024 / (uint64_t)1024);

    ret = true;
  }

  return ret;
}


#ifdef _USE_HW_CMDIF
void sdCmdifInit(void)
{
  if (cmdifIsInit() == false)
  {
    cmdifInit();
  }
  cmdifAdd("sd", sdCmdif);
}

void sdCmdif(void)
{
  bool ret = true;
  sd_info_t sd_info;
  sd_card_t *card = &g_sd;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("info", 0) == true)
  {
    cmdifPrintf("sd init      : %d\n", is_init);
    cmdifPrintf("sd connected : %d\n", sdIsDetected());

    if (is_init == true)
    {
      if (sdGetInfo(&sd_info) == true)
      {
        if (sd_info.card_type == kCARD_OperationVoltage330V)
        {
          cmdifPrintf("  card_voltage         : 3.3V\n");
        }
        else
        {
          cmdifPrintf("  card_voltage         : 1.8V\n");
        }
        cmdifPrintf(  "  card_freq            : %d\n", sd_info.freq);
        cmdifPrintf(  "  card_version         : %d\n", sd_info.card_version);

        if (sd_info.card_class == kSD_TimingSDR12DefaultMode)
        {
          if (card->operationVoltage == kCARD_OperationVoltage330V)
          {
            cmdifPrintf("  card_timinig         : default mode\n");
          }
          else if (card->operationVoltage == kCARD_OperationVoltage180V)
          {
            cmdifPrintf("  card_timinig         : SDR12 mode\n");
          }
        }
        else if (card->currentTiming == kSD_TimingSDR25HighSpeedMode)
        {
          if (card->operationVoltage == kCARD_OperationVoltage180V)
          {
            cmdifPrintf("  card_timinig         : SDR25 mode\n");
          }
          else
          {
            cmdifPrintf("  card_timinig         : High Speed mode\n");
          }
        }
        else if (card->currentTiming == kSD_TimingSDR50Mode)
        {
          cmdifPrintf("  card_timinig         : SDR50 mode\n");
        }
        else if (card->currentTiming == kSD_TimingSDR104Mode)
        {
          cmdifPrintf("  card_timinig         : SDR104 mode\n");
        }
        else if (card->currentTiming == kSD_TimingDDR50Mode)
        {
          cmdifPrintf("  card_timinig         : DDR50 mode\n");
        }

        cmdifPrintf("  rel_card_Add         : %d\n", sd_info.rel_card_Add);
        cmdifPrintf("  block_numbers        : %d\n", sd_info.block_numbers);
        cmdifPrintf("  block_size           : %d\n", sd_info.block_size);
        cmdifPrintf("  card_size            : %d MB, %d.%d GB\n", sd_info.card_size, sd_info.card_size/1024, ((sd_info.card_size * 10)/1024) % 10);
      }
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "sd info \n");
  }
}
#endif /* _USE_HW_CMDIF_SD */

#endif /* _USE_HW_SD */
