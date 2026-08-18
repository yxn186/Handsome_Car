/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Potentiometer.h
  * @brief   This file contains all the function prototypes for
  *          the Potentiometer.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __POTENTIOMETER_H__
#define __POTENTIOMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_adc.h"
/*YOUR CODE*/
class Class_Potentiometer
{
public:
    /**
     * @brief 初始化电位器
     * 
     * @param hadc ADC句柄
     * @param Channel_Rank ADC通道序号
     * @param Raw_Value_1 第一个标定原始值
     * @param Angle_1 第一个标定原始值对应角度
     * @param Raw_Value_2 第二个标定原始值
     * @param Angle_2 第二个标定原始值对应角度
     */
    void Init(ADC_HandleTypeDef *hadc, uint8_t Channel_Rank,
              uint16_t Raw_Value_1 = 0U, float Angle_1 = 0.0f,
              uint16_t Raw_Value_2 = 4095U, float Angle_2 = 0.0f);
    
    /**
     * @brief 更新电位器状态
     */
    void Update();

    /**
     * @brief 获取电位器归一化值
     * 
     * @return float 电位器归一化值，范围-1.0~1.0
     */
    float Get_Value() { return Value; }

    /**
     * @brief 获取电位器原始值
     * 
     * @return int16_t 电位器原始值，范围-2048~2047
     */
    int16_t Get_Raw_Value() { return Raw_Value; }

    /**
     * @brief 获取电位器对应角度
     *
     * @return float 电位器角度
     */
    float Get_Angle() { return Angle; }

    private:
    float Potentiometer_Angle_Convert(uint16_t ADC_Raw_Value);

    ADC_HandleTypeDef *Potentiometer_hadc;  // ADC句柄，假设电位器连接在这个ADC上
    uint8_t Channel_Index;  // 连接的ADC通道索引

    float Voltage;  // 电压值，单位伏特
    uint16_t Raw_Value_1;  // 第一个标定原始值
    uint16_t Raw_Value_2;  // 第二个标定原始值
    float Angle_1;  // 第一个标定原始值对应角度
    float Angle_2;  // 第二个标定原始值对应角度

    float Value;
    float Angle;  // 当前角度
    int16_t Raw_Value;
};






#ifdef __cplusplus
}
#endif

#endif /* __POTENTIOMETER_H__ */
