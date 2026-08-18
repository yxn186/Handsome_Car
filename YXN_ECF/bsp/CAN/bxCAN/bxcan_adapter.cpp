/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bxcan_adapter.cpp
  * @brief   bxcan适应层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bxcan_adapter.h"

/**
 * @brief 绑定bxCAN的HAL句柄
 *
 * @param hcan bxCAN句柄，例如&hcan1、&hcan2
 */
void Class_BxCAN_Adapter::Init(CAN_HandleTypeDef *hcan)
{
    bxCAN_Handle = hcan;//保存bxCAN的HAL句柄
}

/**
 * @brief 发送经典CAN标准数据帧
 *
 * @param id 标准帧ID，范围0x000~0x7FF
 * @param data 发送数据地址
 * @param length 数据长度，范围0~8
 * @return Enum_CAN_Transmit_Status_e 发送结果
 */
Enum_CAN_Transmit_Status_e Class_BxCAN_Adapter::Transmit(uint16_t id,const uint8_t *data,uint8_t length)
{
    //防空
    if (bxCAN_Handle == nullptr || data == nullptr)
    {
        return Enum_CAN_Transmit_Status_e::Error;
    }

    //检查ID和长度是否合法
    //标准帧ID：0x000~0x7FF
    if (id > 0x7FF || length > 8)
    {
        return Enum_CAN_Transmit_Status_e::Error;
    }

    //调用bspcan库的发送函数
    HAL_StatusTypeDef status = CAN_Send_Data(bxCAN_Handle,id,const_cast<uint8_t *>(data),length);

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