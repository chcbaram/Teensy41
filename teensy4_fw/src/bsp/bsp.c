/*
 * bsp.c
 *
 *  Created on: 2020. 3. 11.
 *      Author: Baram
 */




#include "bsp.h"
#include "rtos.h"




static volatile uint32_t systick_counter = 0;
extern void swtimerISR(void);

void SysTick_Handler(void)
{
  systick_counter++;
  swtimerISR();
  osSystickHandler();
}



void bspInit(void)
{
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();


  SysTick_Config(SystemCoreClock / 1000U);

#if 0
  /* Disable I cache and D cache */
  if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
  {
      SCB_DisableICache();
  }
  if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
  {
      SCB_DisableDCache();
  }

  /* Disable MPU */
  ARM_MPU_Disable();

  MPU->RBAR = ARM_MPU_RBAR(0, 0x70000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_16MB);

  /* Enable MPU */
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
#endif

  SCB_EnableDCache();
  SCB_EnableICache();

  rtosInit();
}

void bspDeInit(void)
{
  // Disable Interrupts
  //
  for (int i=0; i<8; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFF;
    __DSB();
    __ISB();
  }
  SysTick->CTRL = 0;

  SCB_DisableDCache();
  SCB_DisableICache();
}

void delay(uint32_t ms)
{
  uint32_t pre_time = systick_counter;

#ifdef _USE_HW_RTOS
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    osDelay(ms);
  }
  else
  {
    while(systick_counter-pre_time < ms);
  }
#else
  while(systick_counter-pre_time < ms);
#endif
}

uint32_t millis(void)
{
  return systick_counter;
}

