/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bxcan_adapter.h
  * @brief   This file contains all the function prototypes for
  *          the bxcan_adapter.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BXCAN_ADAPTER_H__
#define __BXCAN_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "CAN_Interface.h"
#include "bsp_can.h"
#include <stdint.h>
/*YOUR CODE*/

/**
 * @brief STM32 bxCAN适配器
 *
 * 将统一的CAN接口转换为普通CAN底层库调用。
 */
class Class_BxCAN_Adapter final : public Class_CAN_Interface //Class_BxCAN_Adapter是Class_CAN_Interface的子类，final表示不能再被继承
{
public:
    /**
     * @brief 绑定普通CAN的HAL句柄
     *
     * @param hcan CAN句柄，例如&hcan1、&hcan2
     */
    void Init(CAN_HandleTypeDef *hcan);

    /**
     * @brief 发送经典CAN标准数据帧
     *
     * @param id 标准帧ID，范围0x000~0x7FF
     * @param data 发送数据地址
     * @param length 数据长度，范围0~8
     * @return Enum_CAN_Transmit_Status_e 发送结果
     */
    Enum_CAN_Transmit_Status_e Transmit(uint16_t id,const uint8_t *data,uint8_t length) override;//override是让编译器检查用的

private:
    CAN_HandleTypeDef *bxCAN_Handle = nullptr;
};






#ifdef __cplusplus
}
#endif

#endif /* __BXCAN_ADAPTER_H__ */
