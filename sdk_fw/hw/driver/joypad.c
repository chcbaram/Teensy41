/*
 * joypad.c
 *
 *  Created on: 2020. 8. 6.
 *      Author: Baram
 */




#include "joypad.h"



#include "adc.h"


#define JOYPAD_ADC_MAX_COUNT     5
#define JOYPAD_DEAD_ZONE         200
#define JOYPAD_MAX_ADC_VALUE     1700


static uint8_t  adc_ch_x = 0;
static uint8_t  adc_ch_y = 1;

static int32_t x_adc_value = 0;
static int32_t y_adc_value = 0;

static int32_t x_adc_offset = 4095/2;
static int32_t y_adc_offset = 4095/2;


static int32_t x_value = 0;
static int32_t y_value = 0;

static int32_t adc_data_x[JOYPAD_ADC_MAX_COUNT];
static int32_t adc_data_y[JOYPAD_ADC_MAX_COUNT];



bool joypadInit(void)
{
  uint32_t i;

  for (i=0; i<JOYPAD_ADC_MAX_COUNT; i++)
  {
    adc_data_x[i] = 0;
    adc_data_y[i] = 0;
  }

  return true;
}

void joypadUpdate(void)
{
  int32_t value;
  int32_t value_out;


  x_adc_value = adcRead(adc_ch_x);
  y_adc_value = adcRead(adc_ch_y);

  value = constrain(x_adc_value-x_adc_offset, -2000, 2000);
  if (value >  JOYPAD_DEAD_ZONE)      value -= JOYPAD_DEAD_ZONE;
  else if (value < -JOYPAD_DEAD_ZONE) value += JOYPAD_DEAD_ZONE;
  else                                value  = 0;

  value_out = constrain(value, -JOYPAD_MAX_ADC_VALUE, JOYPAD_MAX_ADC_VALUE);
  x_value   = map(value_out, -JOYPAD_MAX_ADC_VALUE, JOYPAD_MAX_ADC_VALUE, -100, 100);


  value = constrain(y_adc_value-y_adc_offset, -2000, 2000);
  if (value >  JOYPAD_DEAD_ZONE)      value -= JOYPAD_DEAD_ZONE;
  else if (value < -JOYPAD_DEAD_ZONE) value += JOYPAD_DEAD_ZONE;
  else                                value  = 0;

  value_out = constrain(value, -JOYPAD_MAX_ADC_VALUE, JOYPAD_MAX_ADC_VALUE);
  y_value   = map(value_out, -JOYPAD_MAX_ADC_VALUE, JOYPAD_MAX_ADC_VALUE , -100, 100);
}

int32_t joypadGetX(void)
{
  joypadUpdate();

  return x_value;
}

int32_t joypadGetY(void)
{
  joypadUpdate();

  return y_value;
}



bool joypadGetPressedButton(uint8_t ch)
{
  bool ret = false;

  joypadUpdate();

  switch(ch)
  {
    case JOYPAD_RIGHT:
      if (x_value > 50) ret = true;
      break;

    case JOYPAD_LEFT:
      if (x_value < -50) ret = true;
      break;

    case JOYPAD_UP:
      if (y_value > 50) ret = true;
      break;

    case JOYPAD_DOWN:
      if (y_value < -50) ret = true;
      break;
  }


  return ret;
}
