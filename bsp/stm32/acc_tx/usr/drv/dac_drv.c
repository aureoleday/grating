#include <stdlib.h>
#include <rtthread.h>
#include <board.h>
#include "sys_conf.h"

DAC_HandleTypeDef hdac;

void dac_init(void)
{
      /* USER CODE BEGIN DAC_Init 0 */

      /* USER CODE END DAC_Init 0 */

      DAC_ChannelConfTypeDef sConfig = {0};

      /* USER CODE BEGIN DAC_Init 1 */

      /* USER CODE END DAC_Init 1 */
      /** DAC Initialization 
      */
      hdac.Instance = DAC;
      if (HAL_DAC_Init(&hdac) != HAL_OK)
      {
        Error_Handler();
      }
      /** DAC channel OUT1 config 
      */
      sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
      sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
      if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
      {
        Error_Handler();
      }
      /* USER CODE BEGIN DAC_Init 2 */

      /* USER CODE END DAC_Init 2 */
}

void dac_set(uint32_t dac_val)
{    
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dac_val);
}

static void dac_set_val(int argc, char**argv)
{
    if (argc < 2)
    {
        rt_kprintf("Please input'dac_set_val <dac_val>'\n");
        return;
    }
    dac_set(atoi(argv[1])); 
}

int dac_start(void)
{
    extern sys_reg_st  g_sys;
    dac_init();
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    dac_set(g_sys.conf.dac_ref);
    return 0;
}


//INIT_APP_EXPORT(dac_app_init);

MSH_CMD_EXPORT(dac_set_val, set dac);
