/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    * @file    SteeringWheel_Chassis_Calculation.h
  * @brief   This file contains all the function prototypes for
    *          the SteeringWheel_Chassis_Calculation.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STEERINGWHEEL_CHASSIS_CALCULATION_H__
#define __STEERINGWHEEL_CHASSIS_CALCULATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define RAD_TO_DEG 57.295779513082320876798154814105f
#define DEG_TO_RAD 0.017453292519943295769236907684886f

/**
 * @brief 舵轮电机数据结构体
 * 
 */
typedef struct
{
    //轮电机线速度
    float Wheel_Angular_Speed = 0.0f;
    float Wheel_Linear_Speed = 0.0f;
    float Steering_Speed = 0.0f;
    float Steering_Angle = 0.0f;
} SteeringWheel_Motor_Data_t;

/**
 * @brief 舵轮电机结构体
 * 
 */
typedef struct
{
    SteeringWheel_Motor_Data_t    Target;
    SteeringWheel_Motor_Data_t    Current;
} SteeringWheel_Motor_t;

/**
 * @brief 舵轮底盘数据结构体
 * 
 */
typedef struct
{
    //前X左Y上Z

    //纵向速度分量 前进为正 后退为负
    float Speed_X = 0.0f;

    //横向速度分量 左移为正 右移为负
    float Speed_Y = 0.0f;

    //旋转角速度分量 逆时针为正 顺时针为负
    float W_Z = 0.0f;

    //总底盘速度（X和Y的合成速度）
    float Speed = 0.0f;
} SteeringWheel_Chassis_Data_t;

/**
 * @brief 舵轮底盘结构体
 * 
 */
typedef struct
{
    SteeringWheel_Chassis_Data_t Target;
    SteeringWheel_Chassis_Data_t Current;
} SteeringWheel_Chassis_t;

class Class_SteeringWheel_Chassis_Calculation
{
    public:

    /**
    * @brief 舵轮底盘解算类初始化函数
    * 
    * @param Chassis_a 旋转中心到前后舵轮中心的纵向距离
    * @param Chassis_b 旋转中心到左右舵轮中心的横向距离
    * @param Wheel_Radius 轮半径
    * @param Max_Wheel_Motor_Linear_Speed 轮电机最大线速度
    */
    void Init(float Chassis_a, float Chassis_b, float Wheel_Radius, float Max_Wheel_Motor_Linear_Speed);

    /**
    * @brief 舵轮底盘数据更新
    * 
    */
    void Update(void);

    /**
    * @brief 设置舵轮底盘目标数据
    * 
    * @param Speed_X 底盘纵向速度分量 前进为正 后退为负
    * @param Speed_Y 底盘横向速度分量 左移为正 右移为负
    * @param W_Z 底盘旋转角速度分量 逆时针为正 顺时针为负
    */
    void Set_Target_Chassis_Data(float Speed_X, float Speed_Y, float W_Z);

    /**
    * @brief 设置某个舵轮电机当前数据
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @param Wheel_Angular_Speed 轮电机角速度
    * @param Steering_Speed 舵向电机速度
    * @param Steering_Angle 舵向电机角度
    */
    void Set_Current_Wheel_Motor_Data(uint8_t Motor_Index, float Wheel_Angular_Speed, float Steering_Speed, float Steering_Angle);

    /**
    * @brief 获取某个舵轮电机目标舵向角度
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @return float 目标舵向角度
    */
    float Get_Target_Steering_Angle(uint8_t Motor_Index);

    /**
    * @brief 获取某个舵轮电机目标线速度
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @return float 目标线速度
    */
    float Get_Target_Wheel_Linear_Speed(uint8_t Motor_Index);

    /**
    * @brief 获取某个舵轮电机目标角速度
    * 
    * @param Motor_Index 电机索引值（0~3）
    * @return float 目标角速度
    */
    float Get_Target_Wheel_Angular_Speed(uint8_t Motor_Index);
    
    private:
    //---工具函数---

    /**
    * @brief 获取当前电机组的纵向轴距向量（得到带方向的纵向轴距）
    * 
    * @param Motor_Index 电机索引
    * @return float 纵向轴距符号 
    */
    float Get_Now_Motor_Group_Vector_a(uint8_t Motor_Index);

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

    /**
    * @brief 寻找舵轮最小角度转向函数 计算出原始目标角后调用
    * 
    * @param Raw_Target_Steering_Angle 原始舵轮目标角度
    * @param Current_Steering_Angle 当前舵轮目标角度
    * @param Motor_Index 电机索引值
    * @return float 舵轮目标差值
    */
    float Find_Mini_Target_Steering_Angle(float Raw_Target_Steering_Angle,float Current_Steering_Angle,uint8_t Motor_Index);

    /**
     * @brief 将角度限制在-180~180度之间
     * 
     * @param Angle 角度值
     * @return float 限制后的角度值
     */
    float Wrap(float Angle);

    //---舵轮底盘数据---
    SteeringWheel_Chassis_t Chassis = { };
    SteeringWheel_Motor_t Motor[4] = { };

    //---舵轮底盘参数---

    //旋转中心到前后舵轮中心的纵向距离
    float Chassis_a = 0.0f;

    //旋转中心到左右舵轮中心的横向距离
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

#endif /* __STEERINGWHEEL_CHASSIS_CALCULATION_H__ */
