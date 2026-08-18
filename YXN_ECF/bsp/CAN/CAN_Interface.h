/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    CAN_Interface.h
  * @brief   This file contains all the function prototypes for
  *          the CAN_Interface.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_INTERFACE_H__
#define __CAN_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief CAN发送结果
 */
enum class Enum_CAN_Transmit_Status_e : uint8_t
{
    Success,
    Busy,
    Error
};

/**
 * @brief CAN传输层统一接口
 */
class Class_CAN_Interface
{
    public:
        virtual Enum_CAN_Transmit_Status_e Transmit(uint16_t id,const uint8_t *data,uint8_t length) = 0;//定义 发送Transmit 纯虚函数

        virtual ~Class_CAN_Interface() = default;//析构函数
};


#ifdef __cplusplus
}
#endif

#endif /* __CAN_INTERFACE_H__ */
