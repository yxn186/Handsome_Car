/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Joystick.h
  * @brief   This file contains all the function prototypes for
  *          the Joystick.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

#define JOYSTICK_DEFAULT_MIN_ADC      0U
#define JOYSTICK_DEFAULT_CENTER_ADC   2048U
#define JOYSTICK_DEFAULT_MAX_ADC      4095U

//摇杆校准数据结构体
typedef struct {
    uint16_t Min_ADC_Values;     // 摇杆推到负方向极限
    uint16_t Center_ADC_Values;  // 摇杆松手中位
    uint16_t Max_ADC_Values;     // 摇杆推到正方向极限
    uint8_t  Reverse; // 是否反向
} JoystickCalib_t;
class Class_Joystick
{
public:
    /**
     * @brief 初始化摇杆
     * 
     * @param hadc ADC句柄
     * @param X_Channel_Rank X轴通道序号
     * @param Y_Channel_Rank Y轴通道序号
     */
    void Init(ADC_HandleTypeDef *hadc, uint8_t X_Channel_Rank, uint8_t Y_Channel_Rank, 
              uint16_t X_Min_ADC = JOYSTICK_DEFAULT_MIN_ADC,
              uint16_t X_Center_ADC = JOYSTICK_DEFAULT_CENTER_ADC,
              uint16_t X_Max_ADC = JOYSTICK_DEFAULT_MAX_ADC,
              bool X_Reverse = false,
              uint16_t Y_Min_ADC = JOYSTICK_DEFAULT_MIN_ADC,
              uint16_t Y_Center_ADC = JOYSTICK_DEFAULT_CENTER_ADC,
              uint16_t Y_Max_ADC = JOYSTICK_DEFAULT_MAX_ADC,
              bool Y_Reverse = false);
    
    /**
     * @brief 更新摇杆状态
     */
    void Update();

    /**
     * @brief 获取X轴归一化值
     * 
     * @return float X轴归一化值，范围-1.0~1.0
     */
    float Get_X() { return X_Value; }
    /**
     * @brief 获取Y轴归一化值
     * 
     * @return float Y轴归一化值，范围-1.0~1.0
     */
    float Get_Y() { return Y_Value; }

    /**
     * @brief 获取X轴原始值
     * 
     * @return int16_t X轴原始值，范围-2048~2047
     */
    int16_t Get_Raw_X() { return Raw_X_Value; }

    /**
     * @brief 获取Y轴原始值
     * 
     * @return int16_t Y轴原始值，范围-2048~2047
     */
    int16_t Get_Raw_Y() { return Raw_Y_Value; }

    private:
    float Joystick_Normalize(uint16_t adc, const JoystickCalib_t *calib);

    JoystickCalib_t Calib_Data_X;  // X轴摇杆校准数据
    JoystickCalib_t Calib_Data_Y;  // Y轴摇杆校准数据
    ADC_HandleTypeDef *Joystick_hadc;  // ADC句柄，假设摇杆连接在这个ADC上
    uint8_t X_Channel_Index;  // X轴连接的ADC通道索引
    uint8_t Y_Channel_Index;  // Y轴连接的ADC通道索引

    float X_Voltage;  // X轴电压值，单位伏特
    float Y_Voltage;  // Y轴电压值，单位伏特
    float X_Value;
    float Y_Value;
    int16_t Raw_X_Value;
    int16_t Raw_Y_Value;
};






#ifdef __cplusplus
}
#endif

#endif /* __JOYSTICK_H__ */
