/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ThreeKey.h
  * @brief   This file contains all the function prototypes for
  *          the ThreeKey.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __THREEKEY_H__
#define __THREEKEY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/
typedef enum
{
    THREE_KEY_UP = 0,
    THREE_KEY_MID,
    THREE_KEY_DOWN,
    THREE_KEY_ERROR
} ThreeKey_State_e;

typedef struct
{
    GPIO_TypeDef *GPIOx_1;
    uint16_t GPIO_Pin_1;
    GPIO_TypeDef *GPIOx_2;
    uint16_t GPIO_Pin_2;
} ThreeKey_GPIO_t;

class Class_ThreeKey
{
    public:
    

    /**
    * @brief 初始化三档开关
    * 
    * @param GPIOx_1 第一个GPIO端口
    * @param GPIO_Pin_1 第一个GPIO引脚
    * @param GPIOx_2 第二个GPIO端口
    * @param GPIO_Pin_2 第二个GPIO引脚
    */
    void Init(GPIO_TypeDef *GPIOx_1, uint16_t GPIO_Pin_1,GPIO_TypeDef *GPIOx_2, uint16_t GPIO_Pin_2);

    
    /**
    * @brief 更新三档开关状态
    * 
    */
    void Update();

    /**
     * @brief 获取三档开关状态
     * 
     * @return ThreeKey_State_e 三档开关的当前状态枚举
     */
    ThreeKey_State_e Get_State()
    {
        return States;
    }

    private:
    ThreeKey_State_e States;
    ThreeKey_GPIO_t ThreeKey_GPIO;
};






#ifdef __cplusplus
}
#endif

#endif /* __THREEKEY_H__ */
