/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Relay.h
  * @brief   This file contains all the function prototypes for
  *          the Relay.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RELAY_H__
#define __RELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal_gpio.h"
/*YOUR CODE*/
class Class_Relay
{
    public:
        /**
        * @brief 初始化继电器
        * 
        * @param Relay_GPIO_Port 继电器GPIO端口
        * @param Relay_Pin 继电器引脚
        */
        void Init(GPIO_TypeDef *Relay_GPIO_Port, uint16_t Relay_Pin, GPIO_PinState Open_PinState);

        /**
         * @brief 设置继电器状态
         * 
         * @param state 继电器状态，true为开启，false为关闭
         */
        void SetState(bool state);

        /**
        * @brief 获取继电器状态
        * 
        * @return true 继电器开启
        * @return false 继电器关闭
        */
        bool GetState();
        
    private:
        GPIO_TypeDef *Relay_GPIO_Port;
        uint16_t Relay_Pin;
        GPIO_PinState Open_State = GPIO_PIN_RESET;
        GPIO_PinState State = GPIO_PIN_RESET;
};






#ifdef __cplusplus
}
#endif

#endif /* __RELAY_H__ */
