/*
 * button.c
 *
 *  Created on: 2020. 8. 5.
 *      Author: Baram
 */




#include "button.h"
#include "swtimer.h"
#include "cmdif.h"
#include "joypad.h"


typedef struct
{
  GPIO_Type    *port;
  uint32_t      pin;
  uint8_t       on_state;
  uint8_t       off_state;
} button_tbl_t;




button_tbl_t button_port_tbl[BUTTON_MAX_CH] =
{
  {GPIO4, 5, _DEF_LOW, _DEF_HIGH}, // 0. BTN_A
  {GPIO4, 6, _DEF_LOW, _DEF_HIGH}, // 1. BTN_B
  {GPIO2,10, _DEF_LOW, _DEF_HIGH}, // 2. BTN_X
  {GPIO2,11, _DEF_LOW, _DEF_HIGH}, // 3. BTN_Y
  {GPIO4, 7, _DEF_LOW, _DEF_HIGH}, // 4. BTN_START
  {GPIO4, 8, _DEF_LOW, _DEF_HIGH}, // 5. BTN_SELECT
  {GPIO4,31, _DEF_LOW, _DEF_HIGH}, // 6. BTN_HOME
  {GPIO3,18, _DEF_LOW, _DEF_HIGH}, // 7. BTN_MENU

  {NULL, },                        // 8.  BTN_LEFT
  {NULL, },                        // 9.  BTN_RIGHT
  {NULL, },                        // 10. BTN_UP
  {NULL, },                        // 11. BTN_DOWN
};



typedef struct
{
  bool        pressed;
  bool        pressed_event;
  uint16_t    pressed_cnt;
  uint32_t    pressed_start_time;
  uint32_t    pressed_end_time;

  bool        released;
  bool        released_event;
  uint32_t    released_start_time;
  uint32_t    released_end_time;

  bool        repeat_update;
  uint32_t    repeat_cnt;
  uint32_t    repeat_time_detect;
  uint32_t    repeat_time_delay;
  uint32_t    repeat_time;
} button_t;


static button_t button_tbl[BUTTON_MAX_CH];


#ifdef _USE_HW_CMDIF
void buttonCmdifInit(void);
void buttonCmdif(void);
#endif

static bool is_enable = true;
static bool buttonGetPin(uint8_t ch);


void button_isr(void *arg)
{
  uint8_t i;
  uint32_t repeat_time;


  for (i=0; i<BUTTON_MAX_CH; i++)
  {

    if (buttonGetPin(i))
    {
      if (button_tbl[i].pressed == false)
      {
        button_tbl[i].pressed_event = true;
        button_tbl[i].pressed_start_time = millis();
      }

      button_tbl[i].pressed = true;
      button_tbl[i].pressed_cnt++;

      if (button_tbl[i].repeat_cnt == 0)
      {
        repeat_time = button_tbl[i].repeat_time_detect;
      }
      else if (button_tbl[i].repeat_cnt == 1)
      {
        repeat_time = button_tbl[i].repeat_time_delay;
      }
      else
      {
        repeat_time = button_tbl[i].repeat_time;
      }
      if (button_tbl[i].pressed_cnt >= repeat_time)
      {
        button_tbl[i].pressed_cnt = 0;
        button_tbl[i].repeat_cnt++;
        button_tbl[i].repeat_update = true;
      }

      button_tbl[i].pressed_end_time = millis();

      button_tbl[i].released = false;
    }
    else
    {
      if (button_tbl[i].pressed == true)
      {
        button_tbl[i].released_event = true;
        button_tbl[i].released_start_time = millis();
      }

      button_tbl[i].pressed  = false;
      button_tbl[i].released = true;
      button_tbl[i].repeat_cnt = 0;
      button_tbl[i].repeat_update = false;

      button_tbl[i].released_end_time = millis();
    }
  }
}



bool buttonInit(void)
{
  uint32_t i;
  swtimer_handle_t h_button_timer;


  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    if (button_port_tbl[i].port != NULL)
    {
    }
  }

  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    button_tbl[i].pressed_cnt    = 0;
    button_tbl[i].pressed        = 0;
    button_tbl[i].released       = 0;
    button_tbl[i].released_event = 0;

    button_tbl[i].repeat_cnt     = 0;
    button_tbl[i].repeat_time_detect = 50;
    button_tbl[i].repeat_time_delay  = 150;
    button_tbl[i].repeat_time        = 200;

    button_tbl[i].repeat_update = false;
  }

  h_button_timer = swtimerGetHandle();
  swtimerSet(h_button_timer, 1, LOOP_TIME, button_isr, NULL );
  swtimerStart(h_button_timer);

#ifdef _USE_HW_CMDIF
  buttonCmdifInit();
#endif


  return true;
}

void buttonResetTime(uint8_t ch)
{
  button_tbl[ch].pressed_start_time    = 0;
  button_tbl[ch].pressed_end_time      = 0;
  button_tbl[ch].released_start_time   = 0;
  button_tbl[ch].released_end_time     = 0;

  button_tbl[ch].pressed_event = false;
  button_tbl[ch].released_event = false;
}

bool buttonGetPin(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  if (button_port_tbl[ch].port != NULL)
  {
    if (GPIO_PinRead(button_port_tbl[ch].port, button_port_tbl[ch].pin) == button_port_tbl[ch].on_state)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    bool ret = false;

    if (ch == _DEF_HW_BTN_LEFT  && joypadGetPressedButton(JOYPAD_LEFT))   ret = true;
    if (ch == _DEF_HW_BTN_RIGHT && joypadGetPressedButton(JOYPAD_RIGHT))  ret = true;
    if (ch == _DEF_HW_BTN_UP    && joypadGetPressedButton(JOYPAD_UP))     ret = true;
    if (ch == _DEF_HW_BTN_DOWN  && joypadGetPressedButton(JOYPAD_DOWN))   ret = true;

    return ret;
  }
}

void buttonEnable(bool enable)
{
  is_enable = enable;
}

bool buttonGetPressed(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH || is_enable == false)
  {
    return false;
  }

  return button_tbl[ch].pressed;
}

uint8_t  buttonGetPressedCount(void)
{
  uint32_t i;
  uint8_t ret = 0;

  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    if (buttonGetPressed(i) == true)
    {
      ret++;
    }
  }

  return ret;
}

void buttonSetRepeatTime(uint8_t ch, uint32_t detect_ms, uint32_t repeat_delay_ms, uint32_t repeat_ms)
{
  if (ch >= BUTTON_MAX_CH || is_enable == false) return;

  button_tbl[ch].repeat_update = false;
  button_tbl[ch].repeat_cnt = 0;
  button_tbl[ch].pressed_cnt = 0;

  button_tbl[ch].repeat_time_detect = detect_ms;
  button_tbl[ch].repeat_time_delay  = repeat_delay_ms;
  button_tbl[ch].repeat_time        = repeat_ms;
}

uint32_t buttonGetRepeatEvent(uint8_t ch)
{
  volatile uint32_t ret = 0;

  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;

  if (button_tbl[ch].repeat_update)
  {
    button_tbl[ch].repeat_update = false;
    ret = button_tbl[ch].repeat_cnt;
  }

  return ret;
}

uint32_t buttonGetRepeatCount(uint8_t ch)
{
  volatile uint32_t ret = 0;

  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;

  ret = button_tbl[ch].repeat_cnt;

  return ret;
}

bool buttonGetPressedEvent(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].pressed_event;

  button_tbl[ch].pressed_event = 0;

  return ret;
}

uint32_t buttonGetPressedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;


  ret = button_tbl[ch].pressed_end_time - button_tbl[ch].pressed_start_time;

  return ret;
}


bool buttonGetReleased(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].released;

  return ret;
}

bool buttonGetReleasedEvent(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].released_event;

  button_tbl[ch].released_event = 0;

  return ret;
}

uint32_t buttonGetReleasedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;


  ret = button_tbl[ch].released_end_time - button_tbl[ch].released_start_time;

  return ret;
}



#ifdef _USE_HW_CMDIF
void buttonCmdifInit(void)
{
  cmdifAdd("button", buttonCmdif);
}

void buttonCmdif(void)
{
  bool ret = true;
  uint8_t ch;
  uint32_t i;


  if (cmdifGetParamCnt() == 1)
  {
    if(cmdifHasString("show", 0) == true)
    {
      while(cmdifRxAvailable() == 0)
      {
        for (i=0; i<BUTTON_MAX_CH; i++)
        {
          cmdifPrintf("%d", buttonGetPressed(i));
        }
        cmdifPrintf("\r");
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
    ch = (uint8_t)cmdifGetParam(1);

    if (ch > 0)
    {
      ch--;
    }

    if (cmdifHasString("time", 0) == true)
    {
      while(cmdifRxAvailable() == 0)
      {
        if(buttonGetPressed(ch))
        {
          cmdifPrintf("BUTTON%d, Time :  %d ms\n", ch+1, buttonGetPressedTime(ch));
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
    cmdifPrintf( "button [show/time] channel(1~%d) ...\n", BUTTON_MAX_CH);
  }
}
#endif
