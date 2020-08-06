/*
 * lcd.c
 *
 *  Created on: 2020. 8. 5.
 *      Author: Baram
 */




#include "lcd.h"
#include "cmdif.h"
#include "button.h"
#include "pwm.h"


#ifdef _USE_HW_LCD
#include "gpio.h"
#include "hangul/PHan_Lib.h"
#ifdef _USE_HW_ILI9341
#include "lcd/ili9341.h"
#endif


#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif



#define LCD_OPT_DEF   __attribute__((optimize("O2")))
#define _PIN_DEF_BL_CTL       1



static lcd_driver_t lcd;


static bool is_init = false;
static uint8_t backlight_value = 100;
static volatile bool is_double_buffer = true;
static uint8_t frame_index = 0;

static bool lcd_request_draw = false;

static volatile uint32_t fps_pre_time;
static volatile uint32_t fps_time;
static volatile uint32_t fps_count = 0;

static volatile uint32_t draw_fps = 30;
static volatile uint32_t draw_frame_time = 0;


static uint16_t *p_draw_frame_buf = NULL;
static __attribute__((section(".lcd_buf"))) uint16_t frame_buffer[2][HW_LCD_WIDTH * HW_LCD_HEIGHT];

static uint16_t _win_w  = HW_LCD_WIDTH;
static uint16_t _win_h  = HW_LCD_HEIGHT;
static uint16_t _win_x  = 0;
static uint16_t _win_y  = 0;


static volatile bool requested_from_thread = false;
static volatile osMessageQId cmd_q;






#ifdef _USE_HW_CMDIF
static void lcdCmdif(void);
#endif


void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor);
void lcdSwapFrameBuffer(void);
static void lcdDrawProcess(void const *argument);






bool lcdInit(void)
{
  backlight_value = 100;


#ifdef _USE_HW_ILI9341
  ili9341Init();
  ili9341InitDriver(&lcd);
#endif


  for (int i=0; i<LCD_WIDTH*LCD_HEIGHT; i++)
  {
    frame_buffer[0][i] = black;
    frame_buffer[1][i] = black;
  }
  memset(frame_buffer, 0x00, sizeof(frame_buffer));

  if (is_double_buffer == true)
  {
    p_draw_frame_buf = frame_buffer[frame_index ^ 1];
  }
  else
  {
    p_draw_frame_buf = frame_buffer[frame_index];
  }


  uint16_t line_buf[HW_LCD_WIDTH];
  memset(line_buf, 0x00, HW_LCD_WIDTH*2);
  lcd.setWindow(_win_x, _win_y, (_win_x+_win_w)-1, (_win_y+_win_h)-1);

  for (int i=0; i<HW_LCD_HEIGHT; i++)
  {
    lcd.sendBuffer((uint8_t *)line_buf, HW_LCD_WIDTH*2, 10);
  }

  lcdSetBackLight(100);


  osThreadId ret;
  osThreadDef(lcdDrawProcess, lcdDrawProcess, osPriorityNormal, 0, 1*1024/4);
  ret = osThreadCreate(osThread(lcdDrawProcess), NULL);
  if (ret == NULL)
  {
    logPrintf("threadCtableCmd Create fail\n");
  }


  is_init = true;

#ifdef _USE_HW_CMDIF
  cmdifAdd("lcd", lcdCmdif);
#endif

  return true;
}

static void lcdDrawProcess(void const *argument)
{
  (void)argument;
  uint32_t pre_time;
  uint32_t delay_time;
  uint32_t pre_draw_time;
  uint16_t *p_frame_buffer;

  pre_time = osKernelSysTick();

  delay_time = 1000 / draw_fps;
  while(1)
  {
    osDelayUntil(&pre_time, delay_time);

    if (lcd_request_draw == true)
    {
      p_frame_buffer = lcdGetFrameBuffer();

      lcdSwapFrameBuffer();
      lcd_request_draw = false;

      pre_draw_time = micros();
      lcd.setWindow(_win_x, _win_y, (_win_x+_win_w)-1, (_win_y+_win_h)-1);
      lcd.sendBuffer((uint8_t *)p_frame_buffer, _win_w*_win_h*2, 0);
      draw_frame_time = (micros() - pre_draw_time)/1000;
    }

    fps_time = millis()-fps_pre_time;
    if (fps_time > 0)
    {
      fps_count = 1000 / fps_time;
    }
    fps_pre_time = millis();

    delay_time = 1000 / draw_fps;
  }
}

uint32_t lcdGetDrawTime(void)
{
  return draw_frame_time;
}

bool lcdIsInit(void)
{
  return is_init;
}

void lcdReset(void)
{
  lcd.reset();
}

uint8_t lcdGetBackLight(void)
{
  return backlight_value;
}

void lcdSetBackLight(uint8_t value)
{
  value = constrain(value, 0, 100);

  if (value != backlight_value)
  {
    backlight_value = value;
  }

  pwmWrite(0, backlight_value * 50 / 100);
}

LCD_OPT_DEF uint32_t lcdReadPixel(uint16_t x_pos, uint16_t y_pos)
{
  return p_draw_frame_buf[y_pos * LCD_WIDTH + x_pos];
}

LCD_OPT_DEF void lcdDrawPixel(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code)
{
  p_draw_frame_buf[y_pos * LCD_WIDTH + x_pos] = rgb_code;
}

LCD_OPT_DEF void lcdClear(uint32_t rgb_code)
{
  lcdClearBuffer(rgb_code);

  lcdUpdateDraw();
}

LCD_OPT_DEF void lcdClearBuffer(uint32_t rgb_code)
{
  uint16_t *p_buf = lcdGetFrameBuffer();

  for (int i=0; i<LCD_WIDTH * LCD_HEIGHT; i++)
  {
    p_buf[i] = rgb_code;
  }
}

void lcdSetDoubleBuffer(bool enable)
{
  is_double_buffer = enable;

  if (enable == true)
  {
    p_draw_frame_buf = frame_buffer[frame_index^1];
  }
  else
  {
    p_draw_frame_buf = frame_buffer[frame_index];
  }
}

bool lcdGetDoubleBuffer(void)
{
  return is_double_buffer;
}

bool lcdDrawAvailable(void)
{
  return !lcd_request_draw;
}

bool lcdRequestDraw(void)
{
  if (lcd_request_draw != true)
  {
    lcd_request_draw = true;
  }
  return true;
}

void lcdUpdateDraw(void)
{
  lcdRequestDraw();
  while(lcdDrawAvailable() != true);
}

void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  lcd.setWindow(x, y, w, h);
}

void lcdSwapFrameBuffer(void)
{
  if (is_double_buffer == true)
  {
    frame_index ^= 1;
    p_draw_frame_buf = frame_buffer[frame_index ^ 1];
  }
  else
  {
    p_draw_frame_buf = frame_buffer[frame_index];
  }
}

uint16_t *lcdGetFrameBuffer(void)
{
  return (uint16_t *)p_draw_frame_buf;
}

uint16_t *lcdGetCurrentFrameBuffer(void)
{
  return (uint16_t *)frame_buffer[frame_index];
}

void lcdDisplayOff(void)
{
}

void lcdDisplayOn(void)
{
  lcdSetBackLight(lcdGetBackLight());
}

int32_t lcdGetWidth(void)
{
  return LCD_WIDTH;
}

int32_t lcdGetHeight(void)
{
  return LCD_HEIGHT;
}


void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      lcdDrawPixel(y0, x0, color);
    } else
    {
      lcdDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  lcdDrawLine(x, y, x, y+h-1, color);
}

void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  lcdDrawLine(x, y, x+w-1, y, color);
}

void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  for (int16_t i=x; i<x+w; i++)
  {
    lcdDrawVLine(i, y, h, color);
  }
}

void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawHLine(x, y, w, color);
  lcdDrawHLine(x, y+h-1, w, color);
  lcdDrawVLine(x, y, h, color);
  lcdDrawVLine(x+w-1, y, h, color);
}

void lcdDrawFillScreen(uint16_t color)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color);
}

void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i, x_Pre = x;
  PHAN_FONT_OBJ FontBuf;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );


    disHanFont( x, y, &FontBuf, color);

    Size_Char = FontBuf.Size_Char;
    if (Size_Char >= 2)
    {
        x += 2*8;
    }
    else
    {
        x += 1*8;
    }

    if( HW_LCD_WIDTH < x )
    {
        x  = x_Pre;
        y += 16;
    }

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }
}

uint32_t lcdGetStrWidth(const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  PHAN_FONT_OBJ FontBuf;
  uint32_t str_len;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  str_len = 0;

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );

    Size_Char = FontBuf.Size_Char;

    str_len += (Size_Char * 8);

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  return str_len;
}

void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      for( Loop=0; Loop<8; Loop++ )
      {
        if( FontPtr->FontBuffer[i*FontSize +j] & (0x80>>Loop))
        {
          lcdDrawPixel(x + index_x, y + i, textcolor);
        }
        index_x++;
      }
    }
  }
}





#ifdef _USE_HW_CMDIF


void lcdCmdif(void)
{
  bool ret = true;
  uint32_t pre_time;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("test", 0) == true)
  {
    uint16_t line_buf[HW_LCD_WIDTH];

    for (int i=0; i<HW_LCD_WIDTH; i++)
    {
      line_buf[i] = red;
    }

    pre_time = millis();
    lcd.setWindow(_win_x, _win_y, (_win_x+_win_w)-1, (_win_y+_win_h)-1);

    for (int i=0; i<HW_LCD_HEIGHT; i++)
    {
      lcd.sendBuffer((uint8_t *)line_buf, HW_LCD_WIDTH*2, 10);
    }
    cmdifPrintf("%d ms\n", millis()-pre_time);
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("info", 0) == true)
  {
    uint8_t info[4] = {0, };

    FLEXIO_MCULCD_StartTransfer(&FLEXIO3_peripheralConfig);
    FLEXIO_MCULCD_WriteCommandBlocking(&FLEXIO3_peripheralConfig, 0x04);
    FLEXIO_MCULCD_StopTransfer(&FLEXIO3_peripheralConfig);

    FLEXIO_MCULCD_StartTransfer(&FLEXIO3_peripheralConfig);
    FLEXIO_MCULCD_ReadDataArrayBlocking(&FLEXIO3_peripheralConfig, &info[0], 4);
    FLEXIO_MCULCD_StopTransfer(&FLEXIO3_peripheralConfig);

    cmdifPrintf("%X %X %X %X\n", info[0], info[1], info[2], info[3]);
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "lcd test \n");
  }
}
#endif



#endif /* _USE_HW_LCD */
