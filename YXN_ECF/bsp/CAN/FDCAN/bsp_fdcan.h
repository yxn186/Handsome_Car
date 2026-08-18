/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the bsp_fdcan.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_FDCAN_H__
#define __BSP_FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fdcan.h"
#include "stm32h7xx_hal.h"
#include <string.h>

/**
 * @brief CAN通信接收回调函数数据类型
 *
 */
typedef void (*CAN_Callback)(FDCAN_RxHeaderTypeDef &Header, uint8_t *Buffer);

/**
 * @brief CAN通信处理结构体
 *
 */
struct Struct_CAN_Manage_Object
{
    FDCAN_HandleTypeDef *CAN_Handler;//对应 FDCAN 的 HAL 句柄
    CAN_Callback Callback_Function;//对应 FDCAN 的回调函数

    // 与接收相关的数据
    FDCAN_RxHeaderTypeDef Rx_Header;//FDCAN接收报文头
    uint8_t Rx_Buffer[64];//CAN FD 最大数据长度可以达到 64 字节
    //所以当前库实际上发送的是经典 CAN，最多只有 8 字节。
    //即使数组是64字节，经典CAN有效数据通常仍然只是 8 字节。

    // 接收时间戳
    uint64_t Rx_Timestamp;
};

extern bool init_finished;

extern struct Struct_CAN_Manage_Object CAN1_Manage_Object;
extern struct Struct_CAN_Manage_Object CAN2_Manage_Object;
extern struct Struct_CAN_Manage_Object CAN3_Manage_Object;

// extern uint8_t CAN1_0x1fe_Tx_Data[];
// extern uint8_t CAN1_0x1ff_Tx_Data[];
// extern uint8_t CAN1_0x200_Tx_Data[];
// extern uint8_t CAN1_0x2fe_Tx_Data[];
// extern uint8_t CAN1_0x2ff_Tx_Data[];
// extern uint8_t CAN1_0x3fe_Tx_Data[];
// extern uint8_t CAN1_0x4fe_Tx_Data[];

// extern uint8_t CAN2_0x1fe_Tx_Data[];
// extern uint8_t CAN2_0x1ff_Tx_Data[];
// extern uint8_t CAN2_0x200_Tx_Data[];
// extern uint8_t CAN2_0x2fe_Tx_Data[];
// extern uint8_t CAN2_0x2ff_Tx_Data[];
// extern uint8_t CAN2_0x3fe_Tx_Data[];
// extern uint8_t CAN2_0x4fe_Tx_Data[];

// extern uint8_t CAN3_0x1fe_Tx_Data[];
// extern uint8_t CAN3_0x1ff_Tx_Data[];
// extern uint8_t CAN3_0x200_Tx_Data[];
// extern uint8_t CAN3_0x2fe_Tx_Data[];
// extern uint8_t CAN3_0x2ff_Tx_Data[];
// extern uint8_t CAN3_0x3fe_Tx_Data[];
// extern uint8_t CAN3_0x4fe_Tx_Data[];

// extern uint8_t CAN_Supercap_Tx_Data[];


void CAN_Init(FDCAN_HandleTypeDef *hfdcan, CAN_Callback Callback_Function);

HAL_StatusTypeDef CAN_Transmit_Data(FDCAN_HandleTypeDef *hfdcan, uint16_t ID, uint8_t *Data, uint16_t Length);

// void TIM_100us_CAN_PeriodElapsedCallback();

// void TIM_1ms_CAN_PeriodElapsedCallback();

#ifdef __cplusplus
}
#endif

#endif /* __BSP_FDCAN_H__ */
