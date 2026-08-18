/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.h
  * @brief   This file contains all the function prototypes for
  *          the key.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KEY_H__
#define __KEY_H__

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief 枚举按钮按下/没按下
 * 
 */
typedef enum{
    KEY_PRESSED,
    KEY_UNPRESSED
} Key_Pressed_State_e;


class Class_Key
{
    public:

    /**
     * @brief 初始化按键
     * 
     * @param GPIOx 按键对应的GPIO端口
     * @param GPIO_Pin 按键对应的GPIO引脚
     */
    void Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
    
    /**
     * @brief 更新按键状态
     * 
     */
    void Update(void);

    /**
     * @brief 获取按键点击状态
     * 
     * @return uint8_t 1 表示按键被点击，0 表示按键未被点击
     */
    uint8_t Get_Click()
    {
        if (Click_Flag)
        {
            Click_Flag = 0;
            return 1;
        }
        return 0;
    }

    /**
     * @brief 获取按键开关状态
     * 
     * @return true 按键开关已按下
     * @return false 按键开关未按下
     */
    bool Get_Switch()
    {
        return Key_Switch;
    }

    private:
    GPIO_TypeDef *KEY_GPIO;
    uint16_t KEY_PIN;

    Key_Pressed_State_e Now_State;
    Key_Pressed_State_e Last_State;

    uint8_t Click_Flag;
    bool Key_Switch;

    uint32_t Last_Click_Time;

    /**
    * @brief 检查特定按钮是否按下
    * 
    * @return Key_Pressed_State：KEY_PRESSED/KEY_UNPRESSED
    */
    Key_Pressed_State_e Get_State(void);

    


    
};

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H__ */
