/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Potentiometer.cpp
  * @brief   电位器库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Potentiometer.h"

/**
 * @brief 初始化电位器
 * 
 * @param hadc ADC句柄
 * @param channel ADC通道
 * @param Raw_Value_1 第一个标定原始值
 * @param Angle_1 第一个标定原始值对应角度
 * @param Raw_Value_2 第二个标定原始值
 * @param Angle_2 第二个标定原始值对应角度
 */
void Class_Potentiometer::Init(ADC_HandleTypeDef *hadc, uint8_t Channel_Rank,
                               uint16_t Raw_Value_1, float Angle_1,
                               uint16_t Raw_Value_2, float Angle_2)
{
    Potentiometer_hadc = hadc;  // 假设电位器连接在 ADC1 上
    this->Channel_Index = Channel_Rank-1;  //Index 从0开始，所以减1
    this->Raw_Value_1 = Raw_Value_1;
    this->Raw_Value_2 = Raw_Value_2;
    this->Angle_1 = Angle_1;
    this->Angle_2 = Angle_2;
}

/**
 * @brief 更新电位器状态
 */
void Class_Potentiometer::Update()
{
    Raw_Value = ADC_Get_Raw(Potentiometer_hadc, Channel_Index);

    Angle = Potentiometer_Angle_Convert(Raw_Value);

    Value = ADC_Get_Normalized(Potentiometer_hadc, Channel_Index);
    
    Voltage = ADC_Get_Voltage(Potentiometer_hadc, Channel_Index);
}

/**
 * @brief 将ADC原始值线性转换为角度
 *
 * @param ADC_Raw_Value ADC原始值
 * @return float 转换后的角度
 */
float Class_Potentiometer::Potentiometer_Angle_Convert(uint16_t ADC_Raw_Value)
{
    if (Raw_Value_1 == Raw_Value_2)
    {
        return Angle_1;
    }

    float Ratio = (float)((int32_t)ADC_Raw_Value - (int32_t)Raw_Value_1) /
                  (float)((int32_t)Raw_Value_2 - (int32_t)Raw_Value_1);

    if (Ratio > 1.0f) Ratio = 1.0f;
    if (Ratio < 0.0f) Ratio = 0.0f;

    return Angle_1 + Ratio * (Angle_2 - Angle_1);
}
