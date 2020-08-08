/*
 * i2s.c
 *
 *  Created on: 2020. 8. 7.
 *      Author: Baram
 */



#include "i2s.h"
#include "mixer.h"
#include "cmdif.h"
#include "wav/wav.h"
#include <math.h>


#ifdef _USE_HW_I2S

#ifdef _USE_HW_CMDIF
static void i2sCmdif(void);
#endif

#define I2S_SAMPLERATE_HZ       16000
#define I2S_MAX_BUF_LEN         (16*4*8) // 32ms
#define I2S_MAX_FRAME_LEN       (16*4)   // 4ms




static bool is_init = false;
volatile static bool is_started = false;
static int16_t send_frame[I2S_MAX_BUF_LEN] = {0, };


static bool i2sAvailableTxDMA(void);
static void i2sThreadProcess(void const *argument);


void i2sCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
  if (kStatus_SAI_RxError == status)
  {
  }
  else
  {
  }
}


bool i2sInit(void)
{

  mixerInit();


  osThreadId ret;
  osThreadDef(i2sThreadProcess, i2sThreadProcess, _HW_DEF_RTOS_THREAD_PRI_I2S, 0, _HW_DEF_RTOS_THREAD_MEM_I2S);
  ret = osThreadCreate(osThread(i2sThreadProcess), NULL);
  if (ret == NULL)
  {
    logPrintf("i2sThreadProcess Create fail\n");
  }


  is_init = true;

#ifdef _USE_HW_CMDIF
  cmdifAdd("i2s", i2sCmdif);
#endif

  return true;
}

bool i2sAvailableTxDMA(void)
{
  if (SAI1_SAI_Tx_eDMA_Handle.saiQueue[SAI1_SAI_Tx_eDMA_Handle.queueUser].data != NULL)
  {
    return false;
  }

  return true;
}

static void i2sThreadProcess(void const *argument)
{
  (void)argument;
  sai_transfer_t xfer;
  uint32_t len;


  while(1)
  {
    if (i2sAvailableTxDMA())
    {
      if (mixerAvailable() >= I2S_MAX_FRAME_LEN)
      {
        len = I2S_MAX_FRAME_LEN;
        mixerRead(send_frame, len);
        xfer.data     = (uint8_t *)send_frame;
        xfer.dataSize = len*2;
        //SCB_InvalidateDCache_by_Addr ((uint32_t *)xfer.data, xfer.dataSize);
        if (SAI_TransferSendEDMA(SAI1_PERIPHERAL, &SAI1_SAI_Tx_eDMA_Handle, &xfer) != kStatus_Success)
        {
        }
      }
    }
    delay(1);
  }
}

bool i2sIsInit(void)
{
  return is_init;
}

bool i2sStart(void)
{
  is_started = true;
  return true;
}

bool i2sStop(void)
{
  is_started = false;
  return true;
}

uint32_t i2sAvailableForWrite(uint8_t ch)
{
  return mixerAvailableForWrite(ch);
}

bool i2sWrite(uint8_t ch, int16_t *p_data, uint32_t length)
{
  return mixerWrite(ch, p_data, length);
}

int8_t i2sGetEmptyChannel(void)
{
  return mixerGetEmptyChannel();
}

// https://m.blog.naver.com/PostView.nhn?blogId=hojoon108&logNo=80145019745&proxyReferer=https:%2F%2Fwww.google.com%2F
//
float i2sGetNoteHz(int8_t octave, int8_t note)
{
  float hz;
  float f_note;

  if (octave < 1) octave = 1;
  if (octave > 8) octave = 8;

  if (note <  1) note = 1;
  if (note > 12) note = 12;

  f_note = (float)(note-10)/12.0f;

  hz = pow(2, (octave-1)) * 55 * pow(2, f_note);

  return hz;
}

// https://gamedev.stackexchange.com/questions/4779/is-there-a-faster-sine-function
//
float i2sSin(float x)
{
  const float B = 4 / M_PI;
  const float C = -4 / (M_PI * M_PI);

  return -(B * x + C * x * ((x < 0) ? -x : x));
}

bool i2sPlayNote(int8_t octave, int8_t note, uint16_t volume, uint32_t time_ms)
{
  uint32_t pre_time;
  int32_t sample_rate = I2S_SAMPLERATE_HZ;
  int32_t num_samples = 4 * 16000 / 1000;
  float sample_point;
  int16_t sample[num_samples];
  int16_t sample_index = 0;
  float div_freq;
  int8_t mix_ch;
  int32_t volume_out;


  volume = constrain(volume, 0, 100);
  volume_out = (INT16_MAX/40) * volume / 100;

  mix_ch = i2sGetEmptyChannel();

  if (mix_ch < 0)
  {
    return false;
  }

  div_freq = (float)sample_rate/(float)i2sGetNoteHz(octave, note);

  pre_time = millis();
  while(millis()-pre_time <= time_ms)
  {
    if (i2sAvailableForWrite(mix_ch) >= num_samples)
    {
      for (int i=0; i<num_samples; i++)
      {
        sample_point = i2sSin(2 * M_PI * (float)(sample_index) / ((float)div_freq));
        sample[i] = (int16_t)(sample_point * volume_out);
        sample_index = (sample_index + 1) % (int)div_freq;
      }
      i2sWrite(mix_ch, sample, num_samples);
    }
    delay(2);
  }

  return true;
}



#ifdef _USE_HW_CMDIF


void i2sCmdif(void)
{
  bool ret = true;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("play", 0))
  {
    uint32_t index;
    uint8_t ch;

    ch = i2sGetEmptyChannel();
    index = 0;
    while(cmdifRxAvailable() == 0)
    {
      if (i2sAvailableForWrite(ch) >= I2S_MAX_FRAME_LEN)
      {

        i2sWrite(ch, (int16_t *)&data[index], I2S_MAX_FRAME_LEN);

        index += I2S_MAX_FRAME_LEN;

        if ((index+I2S_MAX_FRAME_LEN) >= NUM_ELEMENTS)
        {
          break;
        }
      }
      delay(1);
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "i2s play\n");
  }
}

#endif

#endif