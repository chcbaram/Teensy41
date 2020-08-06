/*
 * pwm.c
 *
 *  Created on: 2020. 8. 6.
 *      Author: Baram
 */




#include "pwm.h"
#include "cmdif.h"


#define PWM_DATA_MAX      100



#ifdef _USE_HW_CMDIF
void pwmCmdif(void);
#endif



static bool is_init = false;

typedef struct
{
  PWM_Type *base;
  uint32_t  submodule;
  uint32_t  control;
  uint32_t  channel;
  uint32_t  pwm_duty;
} pwm_tbl_t;


pwm_tbl_t  pwm_tbl[HW_PWM_MAX_CH];




bool pwmInit(void)
{

  pwm_tbl[0].base       = PWM1_PERIPHERAL;
  pwm_tbl[0].submodule  = PWM1_SM0;
  pwm_tbl[0].control    = kPWM_Control_Module_3;
  pwm_tbl[0].channel    = PWM1_SM0_A;
  pwm_tbl[0].pwm_duty   = 50;


  PWM_StartTimer(PWM1_PERIPHERAL, kPWM_Control_Module_3);


  pwmWrite(0, 0);


  is_init = true;

#ifdef _USE_HW_CMDIF
  cmdifAdd("pwm", pwmCmdif);
#endif

  return true;
}

bool pwmIsInit(void)
{
  return is_init;
}

void pwmWrite(uint8_t ch, uint16_t pwm_data)
{
  if (ch >= HW_PWM_MAX_CH) return;

  if (pwm_data > PWM_DATA_MAX)
  {
    pwm_data = PWM_DATA_MAX;
  }

  PWM_UpdatePwmDutycycle(PWM1_PERIPHERAL, pwm_tbl[ch].submodule, pwm_tbl[ch].channel, kPWM_EdgeAligned, pwm_data);
  PWM_SetPwmLdok(PWM1_PERIPHERAL, pwm_tbl[ch].control, true);
}

uint16_t pwmRead(uint8_t ch)
{
  if (ch >= HW_PWM_MAX_CH) return 0;

  return pwm_tbl[ch].pwm_duty;
}




#ifdef _USE_HW_CMDIF
void pwmCmdif(void)
{
  bool ret = true;
  uint8_t  ch;
  uint32_t pwm;


  if (cmdifGetParamCnt() == 3)
  {
    ch = (uint8_t) cmdifGetParam(1);
    pwm = (uint8_t) cmdifGetParam(2);

    ch = constrain(ch, 0, PWM_MAX_CH);

    if(cmdifHasString("set", 0) == true)
    {
      pwmWrite(ch, pwm);
      cmdifPrintf("pwm CH%d %d\n", ch, pwm);
    }
    else
    {
      ret = false;
    }
  }
  else if (cmdifGetParamCnt() == 2)
  {
    ch = (uint8_t) cmdifGetParam(1);

    if(cmdifHasString("get", 0) == true)
    {
      cmdifPrintf("pwm CH%d %d\n", ch, pwmRead(ch));
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
    cmdifPrintf( "pwm set 0~%d 0~255 \n", PWM_MAX_CH-1);
    cmdifPrintf( "pwm get 0~%d \n", PWM_MAX_CH-1);
  }

}
#endif
