/*
 * audio.c
 *
 *  Created on: 2020. 8. 9.
 *      Author: Baram
 */




#include "audio.h"
#include "i2s.h"
#include "files.h"



#define AUDIO_CMD_MAX_CH        4



static bool is_init = false;
static osMutexId mutex_update;
static uint8_t note_volume = 50;

static osMessageQId  msg_cmd_0;
static osMessageQId  msg_cmd_1;
static osMessageQId  msg_cmd_2;
static osMessageQId  msg_cmd_3;
static osMessageQId  msg_cmd_q[AUDIO_CMD_MAX_CH];


typedef enum
{
  AUDIO_PLAY_FILE,
  AUDIO_PLAY_NOTE,
  AUDIO_NONE,
} AudioCmtTable;

typedef struct
{
  bool is_used;
  bool is_busy;
  bool request_stop;

  uint16_t  cmd;
  audio_t   *audio;
} audio_cmd_t;


static audio_t     audio_note;
static audio_cmd_t audio_cmd[AUDIO_CMD_MAX_CH];


static int8_t audioGetCmdChannel(void);
static void audioProcessPlayFile(audio_cmd_t *p_cmd);
static void audioProcessPlayNote(audio_cmd_t *p_cmd);


static void threadAudio(void const *argument)
{
  uint32_t ch = (uint32_t)argument;
  osEvent evt;
  audio_cmd_t *p_cmd;

  while(1)
  {
    evt = osMessageGet(msg_cmd_q[ch], osWaitForever);
    if (evt.status == osEventMessage)
    {
      p_cmd = (audio_cmd_t *)evt.value.p;

      if (p_cmd->is_used == true)
      {
        switch(p_cmd->cmd)
        {
          case AUDIO_PLAY_FILE:
            audioProcessPlayFile(p_cmd);
            break;

          case AUDIO_PLAY_NOTE:
            audioProcessPlayNote(p_cmd);
            break;
        }
      }
    }
  }
}


bool audioInit(void)
{
  bool ret = true;


  for (int i=0; i<AUDIO_CMD_MAX_CH; i++)
  {
    audio_cmd[i].is_used = false;
    audio_cmd[i].is_busy = false;
    audio_cmd[i].request_stop = false;
  }

  audioOpen(&audio_note);

  osMessageQDef(msg_cmd_0, 2, uint32_t);
  msg_cmd_0 = osMessageCreate(osMessageQ(msg_cmd_0), NULL);
  msg_cmd_q[0] = msg_cmd_0;

  osMessageQDef(msg_cmd_1, 2, uint32_t);
  msg_cmd_1 = osMessageCreate(osMessageQ(msg_cmd_1), NULL);
  msg_cmd_q[1] = msg_cmd_1;

  osMessageQDef(msg_cmd_2, 2, uint32_t);
  msg_cmd_2 = osMessageCreate(osMessageQ(msg_cmd_2), NULL);
  msg_cmd_q[2] = msg_cmd_2;

  osMessageQDef(msg_cmd_3, 2, uint32_t);
  msg_cmd_3 = osMessageCreate(osMessageQ(msg_cmd_3), NULL);
  msg_cmd_q[3] = msg_cmd_3;



  osMutexDef(mutex_update);
  mutex_update = osMutexCreate (osMutex(mutex_update));


  osThreadDef(threadAudio0, threadAudio, _HW_DEF_RTOS_THREAD_PRI_AUDIO, 0, _HW_DEF_RTOS_THREAD_MEM_AUDIO);
  if (osThreadCreate(osThread(threadAudio0), (void *)0) == NULL) ret = false;

  osThreadDef(threadAudio1, threadAudio, _HW_DEF_RTOS_THREAD_PRI_AUDIO, 0, _HW_DEF_RTOS_THREAD_MEM_AUDIO);
  if (osThreadCreate(osThread(threadAudio1), (void *)1) == NULL) ret = false;

  osThreadDef(threadAudio2, threadAudio, _HW_DEF_RTOS_THREAD_PRI_AUDIO, 0, _HW_DEF_RTOS_THREAD_MEM_AUDIO);
  if (osThreadCreate(osThread(threadAudio2), (void *)2) == NULL) ret = false;

  osThreadDef(threadAudio3, threadAudio, _HW_DEF_RTOS_THREAD_PRI_AUDIO, 0, _HW_DEF_RTOS_THREAD_MEM_AUDIO);
  if (osThreadCreate(osThread(threadAudio3), (void *)3) == NULL) ret = false;

  is_init = ret;
  return true;
}


bool audioIsInit(void)
{
  return is_init;
}

int8_t audioGetCmdChannel(void)
{
  int8_t ret = -1;


  for (int i=0; i<AUDIO_CMD_MAX_CH; i++)
  {
    if (audio_cmd[i].is_used != true)
    {
      ret = i;
      break;
    }
  }

  return ret;
}

bool audioOpen(audio_t *p_audio)
{
  int8_t ch;
  bool ret = false;


  p_audio->is_open = false;

  ch = audioGetCmdChannel();
  if (ch >= 0)
  {
    p_audio->ch = ch;
    p_audio->is_open = true;
    audio_cmd[ch].is_used = true;
    ret = true;
  }

  return ret;
}

bool audioClose(audio_t *p_audio)
{
  bool ret = true;

  if (p_audio->is_open == true)
  {
    audioStopFile(p_audio);
    p_audio->is_open = false;
    audio_cmd[p_audio->ch].is_used = false;
  }

  return ret;
}

void audioSetNoteVolume(uint8_t volume)
{
  note_volume = constrain(volume, 0, 100);
}

uint8_t audioGetNoteVolume(void)
{
  return note_volume;
}

bool audioPlayFile(audio_t *p_audio, const char *p_name, bool wait)
{
  bool ret = true;
  uint8_t ch;


  if (p_audio == NULL)
  {
    return false;
  }
  if (p_audio->is_open != true)
  {
    return false;
  }

  ch = p_audio->ch;

  if (audio_cmd[ch].is_used)
  {
    if (audio_cmd[ch].is_busy)
    {
      audioStopFile(p_audio);
    }

    audio_cmd_t *p_cmd;

    p_cmd = &audio_cmd[ch];

    p_cmd->cmd = AUDIO_PLAY_FILE;
    p_cmd->audio = p_audio;
    p_cmd->is_busy = true;
    p_cmd->request_stop = false;
    p_cmd->audio->p_file_name = p_name;

    osMessagePut(msg_cmd_q[ch], (uint32_t)p_cmd, 0);

    if (wait == true)
    {
      while(1)
      {
        if (p_cmd->is_busy != true)
        {
          break;
        }
        delay(5);
      }
    }
  }

  return ret;
}

bool audioStopFile(audio_t *p_audio)
{
  bool ret = true;

  if (p_audio == NULL)
  {
    return false;
  }
  if (p_audio->is_open != true)
  {
    return false;
  }

  if (audio_cmd[p_audio->ch].is_used == true)
  {
    audio_cmd[p_audio->ch].request_stop = true;

    while(1)
    {
      if (audio_cmd[p_audio->ch].is_busy == false)
      {
        break;
      }
      delay(1);
    }
    audio_cmd[p_audio->ch].request_stop = false;
  }

  return ret;
}

bool audioIsPlaying(audio_t *p_audio)
{
  return true;
}

uint32_t audioAvailableForWrite(audio_t *p_audio)
{
  return 0;
}

bool audioWrite(audio_t *p_audio, int16_t *p_wav_data, uint32_t wav_len)
{
  return true;
}

bool audioPlayNote(int8_t octave, int8_t note, uint32_t time_ms)
{
  uint8_t ch;
  audio_t *p_audio;


  p_audio = &audio_note;


  ch = p_audio->ch;

  if (audio_cmd[ch].is_used)
  {
    if (audio_cmd[ch].is_busy)
    {
      audioStopFile(p_audio);
    }

    audio_cmd_t *p_cmd;

    p_cmd = &audio_cmd[ch];

    p_cmd->cmd = AUDIO_PLAY_NOTE;
    p_cmd->audio = p_audio;
    p_cmd->is_busy = true;
    p_cmd->request_stop = false;
    p_cmd->audio->p_file_name = NULL;
    p_cmd->audio->octave = octave;
    p_cmd->audio->note = note;
    p_cmd->audio->note_time = time_ms;

    osMessagePut(msg_cmd_q[ch], (uint32_t)p_cmd, 0);
  }

  return true;
}



static int16_t buf_frame[32*8];


void audioProcessPlayFile(audio_cmd_t *p_cmd)
{
  uint8_t ch;
  FILE *fp;
  uint32_t r_len;



  while(1)
  {
    fp = fopen(p_cmd->audio->p_file_name, "r");
    if (fp == NULL)
    {
      break;
    }
    fseek(fp, 44, SEEK_SET);


    r_len = 32*8;
    ch = p_cmd->audio->ch;
    while(p_cmd->request_stop == false)
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
    memset(buf_frame, 0, r_len*2);
    i2sWrite(ch, (int16_t *)buf_frame, r_len);


    fclose(fp);
    break;
  }

  p_cmd->request_stop = false;
  p_cmd->is_busy = false;
}

void audioProcessPlayNote(audio_cmd_t *p_cmd)
{
  i2sPlayNote(p_cmd->audio->octave, p_cmd->audio->note, note_volume, p_cmd->audio->note_time);
  p_cmd->request_stop = false;
  p_cmd->is_busy = false;
}
