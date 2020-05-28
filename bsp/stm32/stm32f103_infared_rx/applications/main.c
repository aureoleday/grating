/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sys_conf.h"
#include "amux_drv.h"
#include "addr_drv.h"
#include "dac_drv.h"
#include "sr_drv.h"

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(B, 15)
IWDG_HandleTypeDef hiwdg;

static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  //hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 2200;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    local_addr_init();
    gvar_init();
    dac_start();
    amux_drv_init();
    sr_init();
    MX_IWDG_Init();
    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(990);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(10);
        HAL_IWDG_Refresh(&hiwdg);
    }

    return RT_EOK;
}
