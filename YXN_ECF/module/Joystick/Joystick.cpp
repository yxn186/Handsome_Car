/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Joystick.cpp
  * @brief   摇杆库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Joystick.h"
#include "bsp_adc.h"
#include "main.h"
#include <cstdint>

/**
 * @brief 初始化摇杆
 * 
 * @param hadc ADC句柄
 * @param X_Channel_Rank X轴通道序号
 * @param Y_Channel_Rank Y轴通道序号
 */
void Class_Joystick::Init(ADC_HandleTypeDef *hadc, uint8_t X_Channel_Rank, uint8_t Y_Channel_Rank,
                        uint16_t X_Min_ADC, uint16_t X_Center_ADC, uint16_t X_Max_ADC, bool X_Reverse,
                        uint16_t Y_Min_ADC, uint16_t Y_Center_ADC, uint16_t Y_Max_ADC, bool Y_Reverse)
{
    if (X_Channel_Rank == 0 || Y_Channel_Rank == 0)
    {
        return;
    }
    Joystick_hadc = hadc;  // 假设摇杆连接在 ADC1 上
    this->X_Channel_Index = X_Channel_Rank-1;  //Index 从0开始，所以减1
    this->Y_Channel_Index = Y_Channel_Rank-1;  //Index 从0开始，所以减1

    // 设置校准数据
    Calib_Data_X.Min_ADC_Values = X_Min_ADC;
    Calib_Data_X.Center_ADC_Values = X_Center_ADC;
    Calib_Data_X.Max_ADC_Values = X_Max_ADC;
    Calib_Data_X.Reverse = X_Reverse;

    Calib_Data_Y.Min_ADC_Values = Y_Min_ADC;
    Calib_Data_Y.Center_ADC_Values = Y_Center_ADC;
    Calib_Data_Y.Max_ADC_Values = Y_Max_ADC;
    Calib_Data_Y.Reverse = Y_Reverse;
}

/**
 * @brief 更新摇杆状态
 */
void Class_Joystick::Update()
{
    Raw_X_Value = ADC_Get_Raw(Joystick_hadc, X_Channel_Index);
    Raw_Y_Value = ADC_Get_Raw(Joystick_hadc, Y_Channel_Index);

    X_Value = Joystick_Normalize(Raw_X_Value, &Calib_Data_X);
    Y_Value = Joystick_Normalize(Raw_Y_Value, &Calib_Data_Y);

    X_Voltage = ADC_Get_Voltage(Joystick_hadc, X_Channel_Index);
    Y_Voltage = -ADC_Get_Voltage(Joystick_hadc, Y_Channel_Index);
}

/**
 * @brief 将ADC值归一化为-1.0到1.0之间的浮点数
 * 
 * @param ADC_Raw_Value ADC原始值
 * @param calib 摇杆校准数据
 * @return float 归一化后的值，范围-1.0~1.0
 */
float Class_Joystick::Joystick_Normalize(uint16_t ADC_Raw_Value, const JoystickCalib_t *Calib_Data)
{
    float out;

    if (ADC_Raw_Value >= Calib_Data->Center_ADC_Values)
    {
        out = (float)(ADC_Raw_Value - Calib_Data->Center_ADC_Values) / (float)(Calib_Data->Max_ADC_Values - Calib_Data->Center_ADC_Values);
    } else 
    {
        out = -(float)(Calib_Data->Center_ADC_Values - ADC_Raw_Value) / (float)(Calib_Data->Center_ADC_Values - Calib_Data->Min_ADC_Values);
    }

    // 限制输出在-1.0到1.0之间
    if (out > 1.0f) out = 1.0f;
    if (out < -1.0f) out = -1.0f;

    // 死区处理，避免摇杆松手时输出微小的非零值
    if (out > -0.1f && out < 0.1f)
    {
        out = 0.0f;
    }

    if (Calib_Data->Reverse) out = -out;

    return out;
}
