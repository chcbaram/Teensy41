/*
 * adc.c
 *
 *  Created on: 2020. 8. 6.
 *      Author: Baram
 */




#include "adc.h"
#include "cmdif.h"





typedef struct
{
  bool                    is_init;
} adc_tbl_t;



static adc_tbl_t adc_tbl[ADC_MAX_CH];
static uint16_t adc_data[ADC_MAX_CH];


#ifdef _USE_HW_CMDIF
static void adcCmdif(void);
#endif


bool adcInit(void)
{
  uint32_t i;
  uint32_t ch;



  for (i=0; i<ADC_MAX_CH; i++)
  {
    adc_tbl[i].is_init = false;
  }



  // JOY_X
  ch = 0;
  adc_tbl[ch].is_init     = true;


  // JOY_Y
  ch = 1;
  adc_tbl[ch].is_init     = true;


#ifdef _USE_HW_CMDIF
  cmdifAdd("adc", adcCmdif);
#endif

  return true;
}

uint32_t adcRead(uint8_t ch)
{
  uint32_t adc_value;

  if (adc_tbl[ch].is_init != true)
  {
    return 0;
  }


  ADC_SetChannelConfig(ADC2_PERIPHERAL, 0, &ADC2_channels_config[ch]);
  while (ADC_GetChannelStatusFlags(ADC2_PERIPHERAL, 0) == 0)
  {
  }

  switch(ch)
  {
    case 0:
      adc_data[ch] = 4095 - ADC_GetChannelConversionValue(ADC2_PERIPHERAL, 0);
      break;

    case 1:
      adc_data[ch] = ADC_GetChannelConversionValue(ADC2_PERIPHERAL, 0);
      break;
  }


  adc_value = adc_data[ch];

  return adc_value;
}

uint32_t adcRead8(uint8_t ch)
{
  return adcRead(ch)>>4;
}

uint32_t adcRead10(uint8_t ch)
{
  return adcRead(ch)>>2;
}

uint32_t adcRead12(uint8_t ch)
{
  return adcRead(ch);
}

uint32_t adcRead16(uint8_t ch)
{
  return adcRead(ch)<<4;
}

uint32_t adcReadVoltage(uint8_t ch)
{
  return adcConvVoltage(ch, adcRead(ch));
}

uint32_t adcReadCurrent(uint8_t ch)
{

  return adcConvCurrent(ch, adcRead(ch));
}

uint32_t adcConvVoltage(uint8_t ch, uint32_t adc_value)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case 0:
    case 1:
      ret  = (uint32_t)((adc_value * 3300 * 10) / (4095*10));
      ret += 5;
      ret /= 10;
      break;

    case 2:
      ret  = (uint32_t)((adc_value * 3445 * 26) / (4095*10));
      ret += 5;
      ret /= 10;
      break;

  }

  return ret;
}

uint32_t adcConvCurrent(uint8_t ch, uint32_t adc_value)
{
  return 0;
}

uint8_t  adcGetRes(uint8_t ch)
{
  return 0;
}







#ifdef _USE_HW_CMDIF
//-- adcCmdif
//
void adcCmdif(void)
{
  bool ret = true;


  if (cmdifGetParamCnt() == 1)
  {
    if (cmdifHasString("show", 0) == true)
    {
      while(cmdifRxAvailable() == 0)
      {
        for (int i=0; i<ADC_MAX_CH; i++)
        {
          cmdifPrintf("%04d ", adcRead(i));
        }
        cmdifPrintf("\r\n");
        delay(50);
      }
    }
    else
    {
      ret = false;
    }
  }
  else if (cmdifGetParamCnt() == 2)
  {
    if (cmdifHasString("show", 0) == true && cmdifHasString("voltage", 1) == true)
    {
      while(cmdifRxAvailable() == 0)
      {
        for (int i=0; i<ADC_MAX_CH; i++)
        {
          uint32_t adc_data;

          adc_data = adcReadVoltage(i);

          cmdifPrintf("%d.%d ", adc_data/10, adc_data%10);
        }
        cmdifPrintf("\r\n");
        delay(50);
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
    cmdifPrintf( "adc show\n");
    cmdifPrintf( "adc show voltage\n");
  }
}
#endif
