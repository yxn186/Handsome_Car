/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan_adapter.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan_adapter.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FDCAN_ADAPTER_H__
#define __FDCAN_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "CAN_Interface.h"
#include "bsp_fdcan.h"
#include <stdint.h>
/*YOUR CODE*/

/**
 * @brief STM32 FDCAN适配器
 *
 * 将统一CAN接口转换为FDCAN底层库调用。
 * 当前用于发送经典CAN标准数据帧，最大数据长度为8字节。
 */
class Class_FDCAN_Adapter final : public Class_CAN_Interface//Class_FDCAN_Adapter是Class_CAN_Interface的子类，final表示不能再被继承
{
public:
    /**
     * @brief 绑定FDCAN的HAL句柄
     *
     * @param hfdcan FDCAN句柄，例如&hfdcan1、&hfdcan2
     */
    void Init(FDCAN_HandleTypeDef *hfdcan);

    /**
     * @brief 发送经典CAN标准数据帧
     *
     * @param id 标准帧ID，范围0x000~0x7FF
     * @param data 发送数据地址
     * @param length 数据长度，范围0~8
     * @return Enum_CAN_Transmit_Status_e 发送结果
     */
    Enum_CAN_Transmit_Status_e Transmit(uint16_t id,const uint8_t *data,uint8_t length) override;

private:
    FDCAN_HandleTypeDef *FDCAN_Handle = nullptr;
};






#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_ADAPTER_H__ */
