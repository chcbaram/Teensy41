/*
 * clocks.c
 *
 *  Created on: 2020. 2. 20.
 *      Author: Baram
 */




#include "clocks.h"
#include "cmdif.h"


typedef struct
{
  uint32_t hz_OSC;
  uint32_t hz_CPU;
  uint32_t hz_PERIPH_CLK;


  uint32_t hz_SystemPLL;
  uint32_t hz_PLL2_PFD0_CLK;
  uint32_t hz_PLL2_PFD1_CLK;
  uint32_t hz_PLL2_PFD2_CLK;
  uint32_t hz_PLL2_PFD3_CLK;


  uint32_t hz_PLL3_MAIN_CLK;
  uint32_t hz_PLL3_PFD0_CLK;
  uint32_t hz_PLL3_PFD1_CLK;
  uint32_t hz_PLL3_PFD2_CLK;
  uint32_t hz_PLL3_PFD3_CLK;

  uint32_t hz_SEMC;
  uint32_t hz_flexSPI;
  uint32_t hz_flexSPI2;
  uint32_t hz_CSI;
  uint32_t hz_UART;
  uint32_t hz_USDHC1;
  uint32_t hz_USDHC2;
  uint32_t hz_LCDIF;

} clocks_tbl_t;




#ifdef _USE_HW_CMDIF
void clocksCmdifInit(void);
void clocksCmdif(void);
#endif

bool clocksInit(void)
{
#if 0
  // For PLL2
  //
  CLOCK_InitSysPll(&sysPllConfig_BOARD_BootClockRUN);
  /* Init System pfd0. */
  CLOCK_InitSysPfd(kCLOCK_Pfd0, 24);
  /* Init System pfd1. */
  CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
  /* Init System pfd2. */
  CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);
  /* Init System pfd3. */
  CLOCK_InitSysPfd(kCLOCK_Pfd3, 16);


  // For PLL3
  //
  /* Init Usb1 PLL. */
  CLOCK_InitUsb1Pll(&usb1PllConfig_BOARD_BootClockRUN);
  /* Init Usb1 pfd0. */
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 24);
  /* Init Usb1 pfd1. */
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
  /* Init Usb1 pfd2. */
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
  /* Init Usb1 pfd3. */
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);

#endif

#ifdef _USE_HW_CMDIF
  clocksCmdifInit();
#endif
  return true;
}



void clocksGetInfo(clocks_tbl_t *p_info)
{
  p_info->hz_OSC = CLOCK_GetFreq(kCLOCK_OscClk);
  p_info->hz_CPU = CLOCK_GetFreq(kCLOCK_CpuClk);
  p_info->hz_PERIPH_CLK = CLOCK_GetFreq(kCLOCK_PerClk);


  p_info->hz_SystemPLL = CLOCK_GetFreq(kCLOCK_SysPllClk);
  p_info->hz_PLL2_PFD0_CLK = CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk);
  p_info->hz_PLL2_PFD1_CLK = CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk);
  p_info->hz_PLL2_PFD2_CLK = CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk);
  p_info->hz_PLL2_PFD3_CLK = CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk);


  p_info->hz_PLL3_MAIN_CLK = CLOCK_GetFreq(kCLOCK_Usb1PllClk);
  p_info->hz_PLL3_PFD0_CLK = CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk);
  p_info->hz_PLL3_PFD1_CLK = CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk);
  p_info->hz_PLL3_PFD2_CLK = CLOCK_GetFreq(kCLOCK_Usb1PllPfd2Clk);
  p_info->hz_PLL3_PFD3_CLK = CLOCK_GetFreq(kCLOCK_Usb1PllPfd3Clk);

  p_info->hz_SEMC = CLOCK_GetFreq(kCLOCK_SemcClk);


  switch(CLOCK_GetMux(kCLOCK_FlexspiMux))
  {
    case 1:
      p_info->hz_flexSPI = p_info->hz_PLL2_PFD2_CLK / (CLOCK_GetDiv(kCLOCK_FlexspiDiv) + 1);
      break;
    case 3:
      p_info->hz_flexSPI = p_info->hz_PLL3_PFD0_CLK / (CLOCK_GetDiv(kCLOCK_FlexspiDiv) + 1);
      break;
    default:
      p_info->hz_flexSPI = p_info->hz_SEMC / (CLOCK_GetDiv(kCLOCK_FlexspiDiv) + 1);
      break;
  }

  switch(CLOCK_GetMux(kCLOCK_Flexspi2Mux))
  {
    case 0:
      p_info->hz_flexSPI2 = p_info->hz_PLL2_PFD2_CLK / (CLOCK_GetDiv(kCLOCK_Flexspi2Div) + 1);
      break;
    case 3:
      p_info->hz_flexSPI2 = p_info->hz_PLL3_PFD0_CLK / (CLOCK_GetDiv(kCLOCK_Flexspi2Div) + 1);
      break;
    default:
      p_info->hz_flexSPI2 = p_info->hz_SEMC / (CLOCK_GetDiv(kCLOCK_Flexspi2Div) + 1);
      break;
  }

  switch(CLOCK_GetMux(kCLOCK_Usdhc1Mux))
  {
    case 0:
      p_info->hz_USDHC1 = p_info->hz_PLL2_PFD2_CLK / (CLOCK_GetDiv(kCLOCK_Usdhc1Div) + 1);
      break;
    default:
      p_info->hz_USDHC1 = p_info->hz_PLL2_PFD0_CLK / (CLOCK_GetDiv(kCLOCK_Usdhc1Div) + 1);
      break;
  }

  switch(CLOCK_GetMux(kCLOCK_Usdhc2Mux))
  {
    case 0:
      p_info->hz_USDHC2 = p_info->hz_PLL2_PFD2_CLK / (CLOCK_GetDiv(kCLOCK_Usdhc2Div) + 1);
      break;
    default:
      p_info->hz_USDHC2 = p_info->hz_PLL2_PFD0_CLK / (CLOCK_GetDiv(kCLOCK_Usdhc2Div) + 1);
      break;
  }

  p_info->hz_LCDIF = CLOCK_GetFreq(kCLOCK_VideoPllClk) / (CLOCK_GetDiv(kCLOCK_LcdifPreDiv) + 1) / (CLOCK_GetDiv(kCLOCK_LcdifDiv) + 1);
}





#ifdef _USE_HW_CMDIF
void clocksCmdifInit(void)
{
  cmdifAdd("clocks", clocksCmdif);
}

void clocksCmdif(void)
{
  bool ret = true;
  clocks_tbl_t info;


  if (cmdifGetParamCnt() == 1)
  {
    if(cmdifHasString("info", 0) == true)
    {
      clocksGetInfo(&info);

      cmdifPrintf( "OSC             : %d \n", info.hz_OSC);
      cmdifPrintf( "CPU             : %d \n", info.hz_CPU);
      cmdifPrintf( "PERIPH_CLK      : %d \n", info.hz_PERIPH_CLK);
      cmdifPrintf( "\n");
      cmdifPrintf( "SystemPLL       : %d \n", info.hz_SystemPLL);
      cmdifPrintf( "PLL2_PFD0_CLK   : %d \n", info.hz_PLL2_PFD0_CLK);
      cmdifPrintf( "PLL2_PFD1_CLK   : %d \n", info.hz_PLL2_PFD1_CLK);
      cmdifPrintf( "PLL2_PFD2_CLK   : %d \n", info.hz_PLL2_PFD2_CLK);
      cmdifPrintf( "PLL2_PFD3_CLK   : %d \n", info.hz_PLL2_PFD3_CLK);
      cmdifPrintf( "\n");
      cmdifPrintf( "PLL3_MAIN_CLK   : %d \n", info.hz_PLL3_MAIN_CLK);
      cmdifPrintf( "PLL3_PFD0_CLK   : %d \n", info.hz_PLL3_PFD0_CLK);
      cmdifPrintf( "PLL3_PFD1_CLK   : %d \n", info.hz_PLL3_PFD1_CLK);
      cmdifPrintf( "PLL3_PFD2_CLK   : %d \n", info.hz_PLL3_PFD2_CLK);
      cmdifPrintf( "PLL3_PFD3_CLK   : %d \n", info.hz_PLL3_PFD3_CLK);
      cmdifPrintf( "\n");
      cmdifPrintf( "SEMC            : %d \n", info.hz_SEMC);
      cmdifPrintf( "FlexSPI         : %d \n", info.hz_flexSPI);
      cmdifPrintf( "FlexSPI2        : %d \n", info.hz_flexSPI2);
      cmdifPrintf( "USDHC1          : %d \n", info.hz_USDHC1);
      cmdifPrintf( "USDHC2          : %d \n", info.hz_USDHC2);
      cmdifPrintf( "LCDIF           : %d \n", info.hz_LCDIF);
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
    cmdifPrintf( "clocks info \n");
  }
}
#endif
