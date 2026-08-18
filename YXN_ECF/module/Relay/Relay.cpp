/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Relay.cpp
  * @brief   继电器库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Relay.h"


/**
 * @brief 初始化继电器
 * 
 * @param Relay_GPIO_Port 
 * @param Relay_Pin 
 * @param Open_PinState 
 */
void Class_Relay::Init(GPIO_TypeDef *Relay_GPIO_Port, uint16_t Relay_Pin, GPIO_PinState Open_PinState)
{
    this->Relay_GPIO_Port = Relay_GPIO_Port;
    this->Relay_Pin = Relay_Pin;
    this->Open_State = Open_PinState;
    SetState(!Open_State);
}

/**
 * @brief 设置继电器状态
 * 
 * @param state 继电器状态，true为开启，false为关闭 高电平触发?
 */
void Class_Relay::SetState(bool state)
{
    State = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState pin_state = state
                                  ? Open_State
                                  : (Open_State == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(Relay_GPIO_Port, Relay_Pin, pin_state);
}

/**
 * @brief 获取继电器状态
 * 
 * @return true 继电器开启
 * @return false 继电器关闭
 */
bool Class_Relay::GetState()
{
    return State;
}