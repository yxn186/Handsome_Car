/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan_adapter.cpp
  * @brief   FDCAN适应层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "fdcan_adapter.h"

/**
 * @brief 绑定FDCAN的HAL句柄
 *
 * @param hfdcan FDCAN句柄，例如&hfdcan1、&hfdcan2
 */
void Class_FDCAN_Adapter::Init(FDCAN_HandleTypeDef *hfdcan)
{
    FDCAN_Handle = hfdcan;
}

/**
 * @brief 发送经典CAN标准数据帧
 *
 * @param id 标准帧ID，范围0x000~0x7FF
 * @param data 发送数据地址
 * @param length 数据长度，范围0~8
 * @return Enum_CAN_Transmit_Status_e 发送结果
 */
Enum_CAN_Transmit_Status_e_e Class_FDCAN_Adapter::Transmit(uint16_t id,const uint8_t *data,uint8_t length)
{
    //防空
    if (FDCAN_Handle == nullptr || data == nullptr)
    {
        return Enum_CAN_Transmit_Status_e::Error;
    }

    //检查ID和长度是否合法
    if (id > 0x7FF || length > 8)
    {
        return Enum_CAN_Transmit_Status_e::Error;
    }

    //调用bsp_fdcan库的发送函数
    HAL_StatusTypeDef status = CAN_Transmit_Data(FDCAN_Handle,id,const_cast<uint8_t *>(data),length);

    if (status == HAL_OK)
    {
        return Enum_CAN_Transmit_Status_e::Success;
    }

    if (status == HAL_BUSY)
    {
        return Enum_CAN_Transmit_Status_e::Busy;
    }

    return Enum_CAN_Transmit_Status_e::Error;
}