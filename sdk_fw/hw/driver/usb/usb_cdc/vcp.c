/*
 * vcp.c
 *
 *  Created on: 2017. 2. 13.
 *      Author: baram
 */



#include <stdarg.h>
#include <stdbool.h>

#include "vcp.h"
#include "hw.h"



//-- Internal Variables
//
static volatile uint32_t tx_err_cnt = 0;
//static volatile uint32_t rx_err_cnt = 0;

static volatile uint32_t tx_retry_cnt = 0;

static volatile uint32_t tx_cnt = 0;
//static volatile uint32_t rx_cnt = 0;

static bool is_init = false;

//-- External Variables
//


//-- Internal Functions
//



//-- External Functions
//
extern uint32_t CDC_Itf_GetBaud(void);
extern uint32_t CDC_Itf_TxAvailable( void );
extern uint32_t CDC_Itf_RxAvailable( void );
extern int32_t  CDC_Itf_Write( uint8_t *p_buf, uint32_t length );
extern uint8_t  CDC_Itf_Getch( void );
extern uint8_t  CDC_Itf_Read( void );
extern uint32_t CDC_Itf_TxBufLengh( void );
extern bool CDC_Itf_IsConnected(void);
extern void CDC_Itf_Flush( void );
extern void CDC_Itf_Init(void);



bool vcpInit(void)
{
  is_init = true;


  CDC_Itf_Init();

  return true;
}

uint32_t vcpGetBaud(void)
{
  return CDC_Itf_GetBaud();
}


bool vcpFlush(void)
{
  if (is_init != true) return false;

  CDC_Itf_Flush();

  return true;
}

uint32_t vcpAvailable(void)
{
  if (is_init != true) return 0;

  return CDC_Itf_RxAvailable();
}

void vcpPutch(uint8_t ch)
{
  if (is_init != true) return;

  vcpWrite( &ch, 1 );
}


uint8_t vcpGetch(void)
{
  if (is_init != true) return 0;

  return CDC_Itf_Getch();
}

int32_t vcpWrite(uint8_t *p_data, uint32_t length)
{

  int32_t  ret;
  uint32_t t_time;


  if (is_init != true) return 0;


  t_time = millis();
  while(1)
  {
    ret = CDC_Itf_Write( p_data, length );

    if(ret < 0)
    {
      break;
    }
    if(ret == length)
    {
      tx_cnt += length;
      break;
    }
    if(millis()-t_time > 100)
    {
      ret = 0;
      break;
    }
  }

  return ret;
}

int32_t vcpWriteTimeout(uint8_t *p_data, uint32_t length, uint32_t timeout)
{

  int32_t  ret;
  uint32_t t_time;

  if (is_init != true) return 0;

  t_time = millis();
  while(1)
  {
    ret = CDC_Itf_Write( p_data, length );

    if(ret < 0)
    {
      ret = 0;
      break;
    }
    if(ret == length)
    {
      tx_cnt += length;
      break;
    }
    if(millis()-t_time > timeout)
    {
      ret = 0;
      break;
    }
  }

  return ret;
}

uint8_t vcpRead(void)
{
  if (is_init != true) return 0;

  return CDC_Itf_Read();
}

int32_t vcpPrintf( const char *fmt, ...)
{
  int32_t ret = 0;
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[255];

  if (is_init != true) return 0;

  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  ret = vcpWrite( (uint8_t *)print_buffer, len);

  if (ret != len)
  {
    tx_retry_cnt++;
  }

  return ret;
}

bool vcpIsConnected(void)
{
  if (is_init != true) return false;

  return CDC_Itf_IsConnected();
}

uint32_t vcpGetTxErrCount(void)
{
  return tx_err_cnt;
}

uint32_t vcpGetTxCount(void)
{
  return tx_cnt;
}

uint32_t vcpGetTxRetryCount(void)
{
  return tx_retry_cnt;
}



