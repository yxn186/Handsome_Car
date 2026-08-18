/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.cpp
  * @brief   按键库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "key.h"

/**
 * @brief 检查特定按钮是否按下
 * 
 * @param Keynum 按钮编号
 * @return Key_Pressed_State_e：KEY_PRESSED/KEY_UNPRESSED
 */
Key_Pressed_State_e Class_Key::Get_State(void)
{
    if(HAL_GPIO_ReadPin(KEY_GPIO, KEY_PIN) == GPIO_PIN_RESET)
    {
        return KEY_PRESSED;
    }
    else 
    {
        return KEY_UNPRESSED;
    }
}

/**
 * @brief  初始化按键
 * 
 * @param GPIOx 按键对应的GPIO端口
 * @param GPIO_Pin 按键对应的GPIO引脚
 */
void Class_Key::Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    this->KEY_GPIO = GPIOx;
    this->KEY_PIN = GPIO_Pin;

    Key_Switch = false;
    Last_Click_Time = HAL_GetTick();
    Last_State = Get_State();
    Click_Flag = 0;
}

/**
 * @brief 更新按键状态
 * 
 */
void Class_Key::Update(void)
{
    Now_State = Get_State();

    if (Last_State == KEY_PRESSED && Now_State == KEY_UNPRESSED)
    {
        if (HAL_GetTick() - Last_Click_Time > 20)
        {
            Key_Switch = !Key_Switch;
            Click_Flag = 1;
            Last_Click_Time = HAL_GetTick();
        }
    }

    Last_State = Now_State;
}