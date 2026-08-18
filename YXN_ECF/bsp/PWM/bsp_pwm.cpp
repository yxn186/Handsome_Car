/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_pwm.cpp
  * @brief   PWM库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_pwm.h"

/**
 * @brief 初始化PWM
 * 
 * @param PWM_htim 定时器句柄
 * @param PWM_Channel PWM通道
 * @param MaxDuty 最大占空比 默认为0.98
 */
void Class_PWM::Init(TIM_HandleTypeDef *PWM_htim,uint32_t PWM_Channel,float MaxDuty)
{
    this->PWM_htim = PWM_htim;
    this->PWM_Channel = PWM_Channel;

    if (MaxDuty < 0.0f) MaxDuty = 0.0f;
    if (MaxDuty > 1.0f) MaxDuty = 1.0f;
    this->MaxDuty = MaxDuty;

    HAL_TIM_PWM_Start(this->PWM_htim, this->PWM_Channel);
}

/**
 * @brief 设置比较值
 * 
 * @param Compare 比较值
 */
void Class_PWM::SetCompare(uint32_t Compare)
{
    if (this->PWM_htim == nullptr) return;
    
    uint32_t Arr = __HAL_TIM_GET_AUTORELOAD(this->PWM_htim);

    if (Compare > Arr)
    {
        Compare = Arr;
    }

    __HAL_TIM_SET_COMPARE(this->PWM_htim, this->PWM_Channel, Compare);
}

/**
 * @brief 设置占空比
 * 
 * @param Duty 占空比，范围为0.0到MaxDuty
 */
void Class_PWM::SetDuty(float Duty)
{
    if (Duty < 0.0f) Duty = 0.0f;
    if (Duty > this->MaxDuty) Duty = this->MaxDuty;

    uint32_t Arr = __HAL_TIM_GET_AUTORELOAD(this->PWM_htim);
    uint32_t Ccr = (uint32_t)(Arr * Duty);

    SetCompare(Ccr);
}