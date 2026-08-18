/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ThreeKey.cpp
  * @brief   三档开关库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "ThreeKey.h"
#include "stm32f103xb.h"

/**
 * @brief 初始化三档开关
 * 
 * @param GPIOx_1 第一个GPIO端口
 * @param GPIO_Pin_1 第一个GPIO引脚
 * @param GPIOx_2 第二个GPIO端口
 * @param GPIO_Pin_2 第二个GPIO引脚
 */
void Class_ThreeKey::Init(GPIO_TypeDef *GPIOx_1, uint16_t GPIO_Pin_1,GPIO_TypeDef *GPIOx_2, uint16_t GPIO_Pin_2)
{
    ThreeKey_GPIO.GPIOx_1 = GPIOx_1;
    ThreeKey_GPIO.GPIO_Pin_1 = GPIO_Pin_1;
    ThreeKey_GPIO.GPIOx_2 = GPIOx_2;
    ThreeKey_GPIO.GPIO_Pin_2 = GPIO_Pin_2;
    States = THREE_KEY_ERROR;  // 初始化状态为错误状态
}

/**
 * @brief 更新三档开关状态
 * 
 */
void Class_ThreeKey::Update()
{
    uint8_t ThreeKey1_State = HAL_GPIO_ReadPin(ThreeKey_GPIO.GPIOx_1, ThreeKey_GPIO.GPIO_Pin_1);
    uint8_t ThreeKey2_State = HAL_GPIO_ReadPin(ThreeKey_GPIO.GPIOx_2, ThreeKey_GPIO.GPIO_Pin_2);

    if (ThreeKey1_State == GPIO_PIN_SET && ThreeKey2_State == GPIO_PIN_RESET)
    {
        States = THREE_KEY_UP;  // 上档
    }
    else if (ThreeKey1_State == GPIO_PIN_RESET && ThreeKey2_State == GPIO_PIN_SET)
    {
        States = THREE_KEY_DOWN;  // 下档
    }
    else if (ThreeKey1_State == GPIO_PIN_SET && ThreeKey2_State == GPIO_PIN_SET)
    {
        States = THREE_KEY_MID;  // 中档
    }
    else
    {
        States = THREE_KEY_ERROR;  // 错误状态
    }
}

