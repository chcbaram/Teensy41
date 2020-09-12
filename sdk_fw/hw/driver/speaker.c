/*
 * speaker.c
 *
 *  Created on: 2020. 2. 1.
 *      Author: Baram
 */




#include "speaker.h"
#include "gpio.h"
#include "audio.h"


static uint8_t volume = 100;
static audio_t audio;



bool speakerInit(void)
{

  //speakerEnable();

  return true;
}

void speakerEnable(void)
{
  //gpioPinWrite(_PIN_GPIO_SPK_EN, _DEF_HIGH);
}

void speakerDisable(void)
{
  //gpioPinWrite(_PIN_GPIO_SPK_EN, _DEF_LOW);
}

void speakerSetVolume(uint8_t volume_data)
{
  if (volume != volume_data)
  {
    volume = volume_data;
  }
}

uint8_t speakerGetVolume(void)
{
  return volume;
}


void speakerStart(uint32_t hz)
{
  audioSetSampleRate(hz);
  audioOpen(&audio);
}

void speakerStop(void)
{
  audioClose(&audio);
}

void speakerReStart(void)
{
  audioOpen(&audio);
}

uint32_t speakerAvailable(void)
{
  return audioAvailableForWrite(&audio);
}

uint32_t speakerGetBufLength(void)
{
  //return dacGetBufLength();
  return 0;
}

void speakerPutch(uint8_t data)
{
  int16_t out_data;

  //dacPut16(map(data, 0, 255, 0, volume*4095/100));

  out_data = data;
  out_data = out_data - 128;
  out_data = out_data << 8;
  audioWrite(&audio, &out_data, 1);
}

void speakerWrite(uint8_t *p_data, uint32_t length)
{
  uint32_t i;


  for (i=0; i<length; i++)
  {
    speakerPutch(p_data[i]);
  }
}
