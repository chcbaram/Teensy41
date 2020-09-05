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
#include "files.h"


#ifdef _USE_HW_I2S


//#define _USE_I2S_CMDIF


#if defined(_USE_HW_CMDIF) && defined(_USE_I2S_CMDIF)
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

#if defined(_USE_HW_CMDIF) && defined(_USE_I2S_CMDIF)
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
        SCB_InvalidateDCache_by_Addr ((uint32_t *)xfer.data, xfer.dataSize);
        if (SAI_TransferSendEDMA(SAI1_PERIPHERAL, &SAI1_SAI_Tx_eDMA_Handle, &xfer) != kStatus_Success)
        {
          SAI_TransferTerminateSendEDMA(SAI1_PERIPHERAL, &SAI1_SAI_Tx_eDMA_Handle);
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

uint32_t i2sGetFrameSize(void)
{
  return I2S_MAX_FRAME_LEN;
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



#if defined(_USE_HW_CMDIF) && defined(_USE_I2S_CMDIF)

typedef struct wavfile_header_s
{
  char    ChunkID[4];     /*  4   */
  int32_t ChunkSize;      /*  4   */
  char    Format[4];      /*  4   */

  char    Subchunk1ID[4]; /*  4   */
  int32_t Subchunk1Size;  /*  4   */
  int16_t AudioFormat;    /*  2   */
  int16_t NumChannels;    /*  2   */
  int32_t SampleRate;     /*  4   */
  int32_t ByteRate;       /*  4   */
  int16_t BlockAlign;     /*  2   */
  int16_t BitsPerSample;  /*  2   */

  char    Subchunk2ID[4];
  int32_t Subchunk2Size;
} wavfile_header_t;

#include "mp3/mp3dec.h"


#define READBUF_SIZE  1940

uint8_t read_buf [READBUF_SIZE*2];
uint8_t *read_ptr;
int16_t out_buf  [2 * 1152];
int     bytes_left;

static int16_t buf_frame[I2S_MAX_BUF_LEN];


static int fillReadBuffer(uint8_t *read_buf, uint8_t *read_ptr, int buf_size, int bytes_left, FILE *infile)
{
  int nRead;

  /* move last, small chunk from end of buffer to start, then fill with new data */
  memmove(read_buf, read_ptr, bytes_left);
  nRead = fread( read_buf + bytes_left, 1, buf_size - bytes_left, infile);
  /* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
  if (nRead < buf_size - bytes_left)
  {
    memset(read_buf + bytes_left + nRead, 0, buf_size - bytes_left - nRead);
  }
  return nRead;
}


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
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("play_file", 0))
  {
    uint8_t ch;
    FILE *fp;
    uint32_t r_len;


    fp = fopen("sound.wav", "r");
    if (fp == NULL)
    {
      cmdifPrintf("open sound.wav fail\n");
      return;
    }
    fseek(fp, sizeof(wavfile_header_t), SEEK_SET);


    r_len = I2S_MAX_FRAME_LEN*4;
    ch = i2sGetEmptyChannel();
    while(cmdifRxAvailable() == 0)
    {
      int len;

      if (i2sAvailableForWrite(ch) >= r_len)
      {
        len = fread(buf_frame, r_len, 2, fp);

        if (len != r_len*2)
        {
          break;
        }
        i2sWrite(ch, (int16_t *)buf_frame, r_len);
      }
      delay(1);
    }

    fclose(fp);
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("mp3", 0))
  {
    HMP3Decoder h_dec;

    h_dec = MP3InitDecoder();

    if (h_dec != 0)
    {
      MP3FrameInfo frameInfo;
      FILE *fp;

      cmdifPrintf("init ok\n");


      fp = fopen( "mp3.mp3", "r" );

      if( fp == NULL )
      {
        printf( "File is null\n" );
        return;
      }
      else
      {
        printf( "File is not null\n" );
      }

      //int offset;
      int err;
      int n_read;

      //fread( buf, 4096, 1, fp );

      bytes_left = 0;
      read_ptr = read_buf;

      n_read = fillReadBuffer(read_buf, read_ptr, READBUF_SIZE, bytes_left, fp);
      bytes_left += n_read;
      read_ptr = read_buf;

      n_read = MP3FindSyncWord(read_ptr, READBUF_SIZE);
      printf("Offset: %d\n", n_read);

      bytes_left -= n_read;
      read_ptr += n_read;

      n_read = fillReadBuffer(read_buf, read_ptr, READBUF_SIZE, bytes_left, fp);
      bytes_left += n_read;
      read_ptr = read_buf;


      err = MP3GetNextFrameInfo(h_dec, &frameInfo, read_ptr);
      if (err != ERR_MP3_INVALID_FRAMEHEADER)
      {
        cmdifPrintf("samplerate     %d\n", frameInfo.samprate);
        cmdifPrintf("bitrate        %d\n", frameInfo.bitrate);
        cmdifPrintf("nChans         %d\n", frameInfo.nChans);
        cmdifPrintf("outputSamps    %d\n", frameInfo.outputSamps);
        cmdifPrintf("bitsPerSample  %d\n", frameInfo.bitsPerSample);

        SAI_TxSetBitClockRate(SAI1_PERIPHERAL, SAI1_TX_BCLK_SOURCE_CLOCK_HZ, frameInfo.samprate/2, SAI1_TX_WORD_WIDTH, SAI1_TX_WORDS_PER_FRAME);
      }


      while(cmdifRxAvailable() == 0)
      {
        osThreadYield();

        if (bytes_left < READBUF_SIZE)
        {
          n_read = fillReadBuffer(read_buf, read_ptr, READBUF_SIZE*2, bytes_left, fp);
          if (n_read == 0 )
          {
            break;
          }
          bytes_left += n_read;
          read_ptr = read_buf;
        }
        //uint32_t pre_time;

        //pre_time = micros();
        n_read = MP3FindSyncWord(read_ptr, bytes_left);
        if (n_read >= 0)
        {
          read_ptr += n_read;
          bytes_left -= n_read;


          //fill the inactive outbuffer
          err = MP3Decode(h_dec, &read_ptr, (int*) &bytes_left, out_buf, 0);
          //cmdifPrintf("%d us\n", micros()-pre_time);

          if (err)
          {
            // sometimes we have a bad frame, lets just nudge forward one byte
            if (err == ERR_MP3_INVALID_FRAMEHEADER)
            {
              read_ptr   += 1;
              bytes_left -= 1;
            }
          }
          else
          {
            //cmdifPrintf("%d \n", bytes_left);


            int index = 0;
            int buf_len = 1152*2 / 4 / 4;
            int16_t i2s_buf[buf_len];

            for (int i=0; i<1152*2; i+=4)
            {
              i2s_buf[index] = out_buf[i]/4;
              index++;

              if (index == buf_len)
              {
                index = 0;
                while(i2sAvailableForWrite(0) < buf_len)
                {
                  osThreadYield();
                }
                i2sWrite(0, (int16_t *)i2s_buf, buf_len);
                delay(1);
              }
            }
          }
          delay(1);
        }

      }
      fclose(fp);
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
