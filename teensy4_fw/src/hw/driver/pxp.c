/*
 * pxp.c
 *
 *  Created on: 2020. 8. 16.
 *      Author: Baram
 */




#include "pxp.h"
#include "lcd.h"

#include "fsl_pxp.h"
#include "fsl_cache.h"

typedef uint16_t pixel_t;
#define PXP_BPP         2U                          /* Use 16-bit RGB565 format. */
#define PXP_PS_FORMAT   kPXP_PsPixelFormatRGB565
#define PXP_AS_FORMAT   kPXP_AsPixelFormatRGB565
#define PXP_OUT_FORMAT  kPXP_OutputPixelFormatRGB565
#define PXP_DC_FORMAT   kVIDEO_PixelFormatRGB565


#define PXP_PS_ULC_X    0U
#define PXP_PS_ULC_Y    0U

#define PXP_PS_SIZE     (LCD_HEIGHT / 2U)


// PXP Output buffer config.
static pxp_output_buffer_config_t outputBufferConfig;
static pxp_ps_buffer_config_t     psBufferConfig;




bool pxpInit(void)
{

  PXP_Init(PXP);


  psBufferConfig.pixelFormat = PXP_PS_FORMAT;
  psBufferConfig.swapByte    = false;
  psBufferConfig.bufferAddr  = 0U;
  psBufferConfig.bufferAddrU = 0U;
  psBufferConfig.bufferAddrV = 0U;
  psBufferConfig.pitchBytes  = PXP_PS_SIZE * PXP_BPP;

  PXP_SetProcessSurfaceBackGroundColor(PXP, 0U);
  PXP_SetProcessSurfaceBufferConfig(PXP, &psBufferConfig);

  /* Disable AS. */
  PXP_SetAlphaSurfacePosition(PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);

  /* Output config. */
  outputBufferConfig.pixelFormat    = PXP_OUT_FORMAT;
  outputBufferConfig.interlacedMode = kPXP_OutputProgressive;
  outputBufferConfig.buffer0Addr    = (uint32_t)lcdGetFrameBuffer();
  outputBufferConfig.buffer1Addr    = 0U;
  outputBufferConfig.pitchBytes     = LCD_WIDTH * PXP_BPP;
  outputBufferConfig.width          = LCD_WIDTH;
  outputBufferConfig.height         = LCD_HEIGHT;

  PXP_SetOutputBufferConfig(PXP, &outputBufferConfig);

  /* Disable CSC1, it is enabled by default. */
  PXP_EnableCsc1(PXP, false);

  return true;
}

bool pxpResize(pxp_resize_t *p_src, pxp_resize_t *p_dst)
{
  uint32_t pre_time;
  bool ret = true;


  PXP_SetProcessSurfaceScaler(PXP, p_src->w, p_src->h, p_dst->w, p_dst->h);
  PXP_SetProcessSurfacePosition(PXP, 0, 0, p_dst->w - 1U, p_dst->h - 1U);


  psBufferConfig.bufferAddr = (uint32_t)&p_src->p_data[p_src->y*p_src->stride + p_src->x];
  if (p_src->stride == 0)
  {
    p_src->stride = p_src->w;
  }
  psBufferConfig.pitchBytes = p_src->stride * PXP_BPP;
  PXP_SetProcessSurfaceBufferConfig(PXP, &psBufferConfig);


  if (p_dst->stride == 0)
  {
    p_dst->stride = p_dst->w;
  }
  outputBufferConfig.buffer0Addr = (uint32_t)&p_dst->p_data[p_dst->y*p_dst->stride + p_dst->x];
  outputBufferConfig.pitchBytes  = p_dst->stride * PXP_BPP;
  outputBufferConfig.width       = p_dst->w;
  outputBufferConfig.height      = p_dst->h;
  PXP_SetOutputBufferConfig(PXP, &outputBufferConfig);

  PXP_SetProcessBlockSize(PXP, kPXP_BlockSize16);

  L1CACHE_CleanDCache();

  PXP_Start(PXP);

  pre_time = millis();
  while (!(kPXP_CompleteFlag & PXP_GetStatusFlags(PXP)))
  {
    delay(1);

    if (millis()-pre_time >= 100)
    {
      ret = false;
      break;
    }
  }
  PXP_ClearStatusFlags(PXP, kPXP_CompleteFlag);

  DCACHE_InvalidateByRange(outputBufferConfig.buffer0Addr, p_dst->w*p_dst->h*PXP_BPP);

  return ret;
}
