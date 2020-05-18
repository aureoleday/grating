#include <stdlib.h>
#include <rtthread.h>
#include <board.h>
#include <rtthread.h>

#define LOCAL_BOARD_CAN_ID              0x001
#define SENSOR_BOARD_CAN_ID             0x002
#define ANOTHER_SENSOR_BOARD_CAN_ID     0x003
#define THIRD_SENSOR_BOARD_CAN_ID       0x004

CAN_TxHeaderTypeDef     TxHeader;
CAN_RxHeaderTypeDef     RxMeg;

CAN_HandleTypeDef       hcan;

static void can_usr_init(void)   //用户初始化函数
{
    CAN_FilterTypeDef  sFilterConfig;
    /* Configure the CAN Filter */
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
    
    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
        /* Filter configuration Error */
        Error_Handler();
    }
    
    /* Start the CAN peripheral */
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
    else
    {
        rt_kprintf("can start!\n");
    }
    
    /* Activate CAN RX notification */
    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        /* Notification Error */
        Error_Handler();
    }
    
    /* Activate CAN TX notification */
    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
    {
        /* Notification Error */
        Error_Handler();
    }
    
    /* Configure Transmission process */
    TxHeader.StdId = 0x321;
    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void can_init(void)
{
    /* USER CODE BEGIN CAN_Init 0 */
    
    /* USER CODE END CAN_Init 0 */
    
    /* USER CODE BEGIN CAN_Init 1 */
    
    /* USER CODE END CAN_Init 1 */
    hcan.Instance = CAN1;
    hcan.Init.Prescaler = 16;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = DISABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN_Init 2 */
    
    /* USER CODE END CAN_Init 2 */
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)  //接收回调函数
{
    uint8_t  Data[8];
    HAL_StatusTypeDef	HAL_RetVal;
    if(hcan_ptr == &hcan)
    {
        HAL_RetVal=HAL_CAN_GetRxMessage(&hcan,  CAN_RX_FIFO0, &RxMeg,  Data);
        if ( HAL_OK==HAL_RetVal)
        {                              			
            //在这里接收数据
        }
    }
}


uint8_t               TxData[8];
uint8_t               RxData[8];
uint32_t              TxMailbox;

void USER_CAN_Send()
{	
	HAL_Delay(1000);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY);//开启中断
	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}		
    rt_kprintf("can sent\n");
	HAL_Delay(100);
	TxData[0]++;
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	static uint8_t num = 0;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
 
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TxMailbox0CompleteCallback could be implemented in the
            user file
   */
	/* if() 数据 要到 */
	HAL_CAN_DeactivateNotification(hcan, CAN_IT_TX_MAILBOX_EMPTY);
	HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, &TxMailbox);
	TxData[1] = TxData[1]+ 1;
}

//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle)
//{
//  /* Get RX message */
//  if (HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
//  {
//    /* Reception Error */
//    Error_Handler();
//  }
// 
//  /* Display LEDx */
//  if ((RxHeader.StdId == 0x321) && (RxHeader.IDE == CAN_ID_STD) && (RxHeader.DLC == 2))
//  {
//		RxData[0] = RxData[0];
//  }
//}


uint8_t can_tx_sample(void)
{
    USER_CAN_Send();
    return 0;
}

static int can_app_init(void)
{
    can_init();
    can_usr_init();
    return 0;
}

static void can_test(void)
{
//    uint8_t txd[8];
//    for(int8_t i=0;i<8;i++)
//        txd[i] = i;
    can_tx_sample();
}

INIT_APP_EXPORT(can_app_init);
MSH_CMD_EXPORT(can_test, can tx test);
