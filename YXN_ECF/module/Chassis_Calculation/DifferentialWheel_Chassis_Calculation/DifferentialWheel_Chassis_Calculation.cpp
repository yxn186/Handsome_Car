/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    * @file    DifferentialWheel_Chassis_Calculation.cpp
  * @brief   差速轮底盘解算库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "DifferentialWheel_Chassis_Calculation.h"
#include <cmath>
#include <cstdint>

//一些规定
//前X左Y上Z
//0、1号轮为左侧轮 2、3号轮为右侧轮


/**
 * @brief 差速轮底盘解算类初始化函数
 * 
 * @param Chassis_b 旋转中心到左右轮中心的横向距离
 * @param Wheel_Radius 轮半径
 * @param Max_Wheel_Motor_Linear_Speed 轮电机最大线速度
 */
void Class_DifferentialWheel_Chassis_Calculation::Init(float Chassis_b, float Wheel_Radius, float Max_Wheel_Motor_Linear_Speed)
{
    if (Wheel_Radius <= 0.0f || Max_Wheel_Motor_Linear_Speed <= 0.0f || Chassis_b <= 0.0f)
    {
        return;
    }

    //保存传参
    this->Chassis_b = Chassis_b;
    this->Wheel_Radius = Wheel_Radius;
    this->Wheel_Radius_Reciprocal = 1.0f / Wheel_Radius;
    this->Max_Wheel_Motor_Linear_Speed = Max_Wheel_Motor_Linear_Speed;

    //初始化差速轮电机数据
    for (int i = 0; i < 4; i++)
    {
        Motor[i].Target.Wheel_Angular_Speed = 0.0f;
        Motor[i].Target.Wheel_Linear_Speed = 0.0f;

        Motor[i].Current.Wheel_Angular_Speed = 0.0f;
        Motor[i].Current.Wheel_Linear_Speed = 0.0f;
    }

    //初始化差速轮底盘数据
    Chassis.Target.Speed_X = 0.0f;
    Chassis.Target.W_Z = 0.0f;
    Chassis.Target.Speed = 0.0f;

    Chassis.Current.Speed_X = 0.0f;
    Chassis.Current.W_Z = 0.0f;
    Chassis.Current.Speed = 0.0f;
}

/**
 * @brief 设置差速轮底盘目标数据
 * 
 * @param Speed_X 底盘纵向速度分量 前进为正 后退为负
 * @param W_Z 底盘旋转角速度分量 逆时针为正 顺时针为负
 */
void Class_DifferentialWheel_Chassis_Calculation::Set_Target_Chassis_Data(float Speed_X, float W_Z)
{
    Chassis.Target.Speed_X = Speed_X;
    Chassis.Target.W_Z = W_Z;

    //计算总底盘速度
    Chassis.Target.Speed = fabsf(Speed_X);
}

/**
 * @brief 设置某个差速轮电机当前数据
 * 
 * @param Motor_Index 电机索引值（0~3，0、1为左侧，2、3为右侧）
 * @param Wheel_Angular_Speed 轮电机角速度
 */
void Class_DifferentialWheel_Chassis_Calculation::Set_Current_Wheel_Motor_Data(uint8_t Motor_Index, float Wheel_Angular_Speed)
{
    if (Motor_Index >= 4)
    {
        return;
    }

    Motor[Motor_Index].Current.Wheel_Angular_Speed = Wheel_Angular_Speed;
    Motor[Motor_Index].Current.Wheel_Linear_Speed = Wheel_Angular_Speed * Wheel_Radius;
}

/**
 * @brief 获取某个差速轮电机目标线速度
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @return float 目标线速度
 */
float Class_DifferentialWheel_Chassis_Calculation::Get_Target_Wheel_Linear_Speed(uint8_t Motor_Index)
{
    if (Motor_Index >= 4)
    {
        return 0.0f;
    }

    return Motor[Motor_Index].Target.Wheel_Linear_Speed;
}

/**
 * @brief 获取某个差速轮电机目标角速度
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @return float 目标角速度
 */
float Class_DifferentialWheel_Chassis_Calculation::Get_Target_Wheel_Angular_Speed(uint8_t Motor_Index)
{
    if (Motor_Index >= 4)
    {
        return 0.0f;
    }

    return Motor[Motor_Index].Target.Wheel_Angular_Speed;
}

/**
 * @brief 差速轮底盘数据更新
 * 
 */
void Class_DifferentialWheel_Chassis_Calculation::Update(void)
{
    //先计算四个轮子的目标线速度
    for (uint8_t i = 0; i < 4; i++)
    {
        float Yi = Get_Now_Motor_Group_Vector_b(i);

        //得到第i个电机的目标轮速
        Motor[i].Target.Wheel_Linear_Speed = Chassis.Target.Speed_X - Chassis.Target.W_Z * Yi;
    }

    //轮速限幅 防止某个轮电机过快 超出输出上限
    //得到最大速度电机索引
    uint8_t Max_Target_Wheel_Linear_Speed_Motor_Index = Find_Max_Linear_Speed();
    float Max_Target_Wheel_Linear_Speed_Abs = fabsf(Motor[Max_Target_Wheel_Linear_Speed_Motor_Index].Target.Wheel_Linear_Speed);

    //判读是否超出限制
    if (Max_Target_Wheel_Linear_Speed_Abs > Max_Wheel_Motor_Linear_Speed)
    {
        float k = Max_Wheel_Motor_Linear_Speed / Max_Target_Wheel_Linear_Speed_Abs;

        for (uint8_t j = 0; j < 4 ; j++)
        {
            //按比例降速（统一缩放）
            Motor[j].Target.Wheel_Linear_Speed = Motor[j].Target.Wheel_Linear_Speed * k;
        }
    }

    //计算四个轮电机目标角速度
    for (uint8_t i = 0; i < 4; i++)
    {
        Motor[i].Target.Wheel_Angular_Speed = Motor[i].Target.Wheel_Linear_Speed * Wheel_Radius_Reciprocal;
    }
}

/**
 * @brief 获取当前电机组的横向轴距向量（得到带方向的横向轴距）Yi
 * 
 * @param Motor_Index 电机索引
 * @return float 横向轴距
 */
float Class_DifferentialWheel_Chassis_Calculation::Get_Now_Motor_Group_Vector_b(uint8_t Motor_Index)
{
    //前X左Y上Z b是横向轴距 以x轴对成
    switch (Motor_Index)
    {
        case 0:
            return 1.0f * Chassis_b;//左前轮
        case 1:
            return 1.0f * Chassis_b;//左后轮
        case 2:
            return -1.0f * Chassis_b;//右后轮
        case 3:
            return -1.0f * Chassis_b;//右前轮
        default:
            return 0.0f;
    }
}

/**
 * @brief 查找最大线速度的电机索引值
 * 
 * @return uint8_t 最大线速度的电机索引值 
 */
uint8_t Class_DifferentialWheel_Chassis_Calculation::Find_Max_Linear_Speed(void)
{
    float Max_Wheel_Speed = 0.0f;
    uint8_t Max_Speed_Motor_Index = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        if (fabsf(Motor[i].Target.Wheel_Linear_Speed) > Max_Wheel_Speed)
        {
            Max_Wheel_Speed = fabsf(Motor[i].Target.Wheel_Linear_Speed);
            Max_Speed_Motor_Index = i;
        }
    }

    return Max_Speed_Motor_Index;
}
