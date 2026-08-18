/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_pwm.h
  * @brief   This file contains all the function prototypes for
  *          the bsp_pwm.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
/*YOUR CODE*/

#define PWM_MAX_DUTY 0.98f

class Class_PWM
{
    public:
        void Init(TIM_HandleTypeDef *pwm_htim,uint32_t pwm_Channel,float MaxDuty = PWM_MAX_DUTY);
        void SetCompare(uint32_t Compare);
        void SetDuty(float Duty);
    private:
        TIM_HandleTypeDef *PWM_htim = nullptr;
        uint32_t PWM_Channel = 0;
        float MaxDuty = PWM_MAX_DUTY;
};


#ifdef __cplusplus
}
#endif

#endif /* __BSP_PWM_H__ */

