/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    * @file    DifferentialWheel_Chassis_Calculation.h
  * @brief   This file contains all the function prototypes for
    *          the DifferentialWheel_Chassis_Calculation.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DIFFERENTIALWHEEL_CHASSIS_CALCULATION_H__
#define __DIFFERENTIALWHEEL_CHASSIS_CALCULATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief 差速轮电机数据结构体
 * 
 */
typedef struct
{
    //轮电机线速度
    float Wheel_Angular_Speed = 0.0f;
    float Wheel_Linear_Speed = 0.0f;
} DifferentialWheel_Motor_Data_t;

/**
 * @brief 差速轮电机结构体
 * 
 */
typedef struct
{
    DifferentialWheel_Motor_Data_t    Target;
    DifferentialWheel_Motor_Data_t    Current;
} DifferentialWheel_Motor_t;

/**
 * @brief 差速轮底盘数据结构体
 * 
 */
typedef struct
{
    //前X左Y上Z

    //纵向速度分量 前进为正 后退为负
    float Speed_X = 0.0f;

    //旋转角速度分量 逆时针为正 顺时针为负
    float W_Z = 0.0f;

    //总底盘速度
    float Speed = 0.0f;
} DifferentialWheel_Chassis_Data_t;

/**
 * @brief 差速轮底盘结构体
 * 
 */
typedef struct
{
    DifferentialWheel_Chassis_Data_t Target;
    DifferentialWheel_Chassis_Data_t Current;
} DifferentialWheel_Chassis_t;

class Class_DifferentialWheel_Chassis_Calculation
{
    public:

    /**
    * @brief 差速轮底盘解算类初始化函数
    * 
    * @param Chassis_b 旋转中心到左右轮中心的横向距离
    * @param Wheel_Radius 轮半径
    * @param Max_Wheel_Motor_Linear_Speed 轮电机最大线速度
    */
    void Init(float Chassis_b, float Wheel_Radius, float Max_Wheel_Motor_Linear_Speed);

    /**
    * @brief 差速轮底盘数据更新
    * 
    */
    void Update(void);

    /**
    * @brief 设置差速轮底盘目标数据
    * 
    * @param Speed_X 底盘纵向速度分量 前进为正 后退为负
    * @param W_Z 底盘旋转角速度分量 逆时针为正 顺时针为负
    */
    void Set_Target_Chassis_Data(float Speed_X, float W_Z);

    /**
    * @brief 设置某个差速轮电机当前数据
    * 
    * @param Motor_Index 电机索引值（0~3，0、1为左侧，2、3为右侧）
    * @param Wheel_Angular_Speed 轮电机角速度
    */
    void Set_Current_Wheel_Motor_Data(uint8_t Motor_Index, float Wheel_Angular_Speed);

    /**
    * @brief 获取某个差速轮电机目标线速度
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @return float 目标线速度
    */
    float Get_Target_Wheel_Linear_Speed(uint8_t Motor_Index);

    /**
    * @brief 获取某个差速轮电机目标角速度
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @return float 目标角速度
    */
    float Get_Target_Wheel_Angular_Speed(uint8_t Motor_Index);
    
    private:
    //---工具函数---

    /**
    * @brief 获取当前电机组的横向轴距向量（得到带方向的横向轴距）
    * 
    * @param Motor_Index 电机索引
    * @return float 横向轴距符号 
    */
    float Get_Now_Motor_Group_Vector_b(uint8_t Motor_Index);

    /**
    * @brief 查找最大线速度的电机索引值  
    * 
    * @return uint8_t 最大线速度的电机索引值 
    */
    uint8_t Find_Max_Linear_Speed(void);

    //---差速轮底盘数据---
    DifferentialWheel_Chassis_t Chassis = { };
    DifferentialWheel_Motor_t Motor[4] = { };

    //---差速轮底盘参数---

    //旋转中心到左右轮中心的横向距离
    float Chassis_b = 0.0f;

    //轮半径
    float Wheel_Radius = 0.0f;

    //轮半径倒数
    float Wheel_Radius_Reciprocal = 0.0f;

    //轮电机最大线速度
    float Max_Wheel_Motor_Linear_Speed = 0.0f;
};






#ifdef __cplusplus
}
#endif

#endif /* __DIFFERENTIALWHEEL_CHASSIS_CALCULATION_H__ */
