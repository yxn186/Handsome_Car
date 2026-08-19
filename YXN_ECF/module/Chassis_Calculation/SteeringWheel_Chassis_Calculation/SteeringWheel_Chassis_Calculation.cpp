/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    * @file    SteeringWheel_Chassis_Calculation.cpp
  * @brief   舵轮底盘解算库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "SteeringWheel_Chassis_Calculation.h"
#include <cmath>
#include <cstdint>

//一些规定
//前X左Y上Z
//第一象限为0号轮 第二象限为1号轮....


/**
 * @brief 舵轮底盘解算类初始化函数
 * 
 * @param Chassis_a 旋转中心到前后舵轮中心的纵向距离
 * @param Chassis_b 旋转中心到左右舵轮中心的横向距离
 * @param Wheel_Radius 轮半径
 */
void Class_SteeringWheel_Chassis_Calculation::Init(float Chassis_a, float Chassis_b, float Wheel_Radius, float Max_Wheel_Motor_Linear_Speed)
{
    if (Wheel_Radius <= 0.0f || Max_Wheel_Motor_Linear_Speed <= 0.0f || Chassis_a <= 0.0f || Chassis_b <= 0.0f)
    {
        return;
    }

    //保存传参
    this->Chassis_a = Chassis_a;
    this->Chassis_b = Chassis_b;
    this->Wheel_Radius = Wheel_Radius;
    this->Wheel_Radius_Reciprocal = 1.0f / Wheel_Radius;
    this->Max_Wheel_Motor_Linear_Speed = Max_Wheel_Motor_Linear_Speed;

    //初始化舵轮电机数据
    for (int i = 0; i < 4; i++)
    {
        Motor[i].Target.Wheel_Angular_Speed = 0.0f;
        Motor[i].Target.Wheel_Linear_Speed = 0.0f;
        Motor[i].Target.Steering_Speed = 0.0f;
        Motor[i].Target.Steering_Angle = 0.0f;

        Motor[i].Current.Wheel_Angular_Speed = 0.0f;
        Motor[i].Current.Wheel_Linear_Speed = 0.0f;
        Motor[i].Current.Steering_Speed = 0.0f;
        Motor[i].Current.Steering_Angle = 0.0f;
    }

    //初始化舵轮底盘数据
    Chassis.Target.Speed_X = 0.0f;
    Chassis.Target.Speed_Y = 0.0f;
    Chassis.Target.W_Z = 0.0f;

    Chassis.Target.Speed = 0.0f;

    Chassis.Current.Speed_X = 0.0f;
    Chassis.Current.Speed_Y = 0.0f;
    Chassis.Current.W_Z = 0.0f;

    Chassis.Current.Speed = 0.0f;

}

/**
 * @brief 设置舵轮底盘目标数据
 * 
 * @param Speed_X 底盘纵向速度分量 前进为正 后退为负
 * @param Speed_Y 底盘横向速度分量 左移为正 右移为负
 * @param W_Z 底盘旋转角速度分量 逆时针为正 顺时针为负
 */
void Class_SteeringWheel_Chassis_Calculation::Set_Target_Chassis_Data(float Speed_X, float Speed_Y, float W_Z)
{
    Chassis.Target.Speed_X = Speed_X;
    Chassis.Target.Speed_Y = Speed_Y;
    Chassis.Target.W_Z = W_Z;

    //计算总底盘速度
    Chassis.Target.Speed = sqrtf(Speed_X * Speed_X + Speed_Y * Speed_Y);
}

/**
 * @brief 设置某个舵轮电机当前数据
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @param Wheel_Angular_Speed 轮电机角速度
 * @param Steering_Speed 舵向电机速度
 * @param Steering_Angle 舵向电机角度
 */
void Class_SteeringWheel_Chassis_Calculation::Set_Current_Wheel_Motor_Data(uint8_t Motor_Index, float Wheel_Angular_Speed, float Steering_Speed, float Steering_Angle)
{
    if (Motor_Index >= 4)
    {
        return;
    }

    Motor[Motor_Index].Current.Wheel_Angular_Speed = Wheel_Angular_Speed;
    Motor[Motor_Index].Current.Wheel_Linear_Speed = Wheel_Angular_Speed * Wheel_Radius;
    Motor[Motor_Index].Current.Steering_Speed = Steering_Speed;
    Motor[Motor_Index].Current.Steering_Angle = Steering_Angle;
}

/**
 * @brief 获取某个舵轮电机目标舵向角度
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @return float 目标舵向角度
 */
float Class_SteeringWheel_Chassis_Calculation::Get_Target_Steering_Angle(uint8_t Motor_Index)
{
    if (Motor_Index >= 4)
    {
        return 0.0f;
    }

    return Motor[Motor_Index].Target.Steering_Angle;
}

/**
 * @brief 获取某个舵轮电机目标线速度
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @return float 目标线速度
 */
float Class_SteeringWheel_Chassis_Calculation::Get_Target_Wheel_Linear_Speed(uint8_t Motor_Index)
{
    if (Motor_Index >= 4)
    {
        return 0.0f;
    }

    return Motor[Motor_Index].Target.Wheel_Linear_Speed;
}

/**
 * @brief 获取某个舵轮电机目标角速度
 * 
 * @param Motor_Index 电机索引值（0~3）
 * @return float 目标角速度
 */
float Class_SteeringWheel_Chassis_Calculation::Get_Target_Wheel_Angular_Speed(uint8_t Motor_Index)
{
    if (Motor_Index >= 4)
    {
        return 0.0f;
    }

    return Motor[Motor_Index].Target.Wheel_Angular_Speed;
}

/**
 * @brief 舵轮底盘数据更新
 * 
 */
void Class_SteeringWheel_Chassis_Calculation::Update(void)
{
    //第一遍：先计算四个轮子的原始线速度和原始舵向角（零速时不覆盖舵角）
    for(uint8_t i = 0; i < 4; i++)
    {
        float Xi = Get_Now_Motor_Group_Vector_a(i);
        float Yi = Get_Now_Motor_Group_Vector_b(i);

        float Target_Total_Speed_X = Chassis.Target.Speed_X - Chassis.Target.W_Z * Yi;
        float Target_Total_Speed_Y = Chassis.Target.Speed_Y + Chassis.Target.W_Z * Xi;

        //得到第i个电机的目标轮速
        Motor[i].Target.Wheel_Linear_Speed = sqrtf(Target_Total_Speed_X * Target_Total_Speed_X + Target_Total_Speed_Y * Target_Total_Speed_Y);

        if(Motor[i].Target.Wheel_Linear_Speed > 0.02f)
        {
            //保存原始舵轮目标角度（atan2返回弧度）
            Motor[i].Target.Steering_Angle = atan2f(Target_Total_Speed_Y, Target_Total_Speed_X) * RAD_TO_DEG;
        }
    }

    //轮速限幅 防止某个轮电机过快 超出输出上限
    //得到最大速度电机索引
    uint8_t Max_Target_Wheel_Linear_Speed_Motor_Index = Find_Max_Linear_Speed();
    float Max_Target_Wheel_Linear_Speed_Abs = fabsf(Motor[Max_Target_Wheel_Linear_Speed_Motor_Index].Target.Wheel_Linear_Speed);

    //判读是否超出限制
    if(Max_Target_Wheel_Linear_Speed_Abs > Max_Wheel_Motor_Linear_Speed)
    {
        float k = Max_Wheel_Motor_Linear_Speed / Max_Target_Wheel_Linear_Speed_Abs;

        for(uint8_t j = 0; j < 4 ; j++)
        {
            //按比例降速（统一缩放）
            Motor[j].Target.Wheel_Linear_Speed = Motor[j].Target.Wheel_Linear_Speed * k;
        }
    }

    //第二遍：最小转位、轮速投影、计算角速度
    for(uint8_t i = 0; i < 4; i++)
    {
        //设置速度死区
        if(Motor[i].Target.Wheel_Linear_Speed > 0.02f)
        {
            //设置舵轮目标最小角度 此时保证当前舵轮角已是最新值
            float Delta_Target_Angle = Find_Mini_Target_Steering_Angle(Motor[i].Target.Steering_Angle, Motor[i].Current.Steering_Angle, i);

            //轮速投影 防止舵轮电机转向未完成导致的轮电机转速过快
            Motor[i].Target.Wheel_Linear_Speed = Motor[i].Target.Wheel_Linear_Speed * cosf(Delta_Target_Angle * DEG_TO_RAD);
        }
        else 
        {
            Motor[i].Target.Wheel_Linear_Speed = 0.0f;
            //Steering_Angle则保持上一次目标，防止零速时舵角跳变
        }

        //计算轮电机目标角速度
        Motor[i].Target.Wheel_Angular_Speed = Motor[i].Target.Wheel_Linear_Speed * Wheel_Radius_Reciprocal;
    }

}

/**
 * @brief 寻找舵轮最小角度转向函数 计算出原始目标角后调用
 * 
 * @param Raw_Target_Steering_Angle 原始舵轮目标角度
 * @param Current_Steering_Angle 当前舵轮角度
 * @param Motor_Index 电机索引值
 * @return float 舵轮目标差值
 */
float Class_SteeringWheel_Chassis_Calculation::Find_Mini_Target_Steering_Angle(float Raw_Target_Steering_Angle,float Current_Steering_Angle,uint8_t Motor_Index)
{
    //方案1 可以使用原始角度 轮速不用反向
    float First_Way_Delta_Angle = Wrap(Raw_Target_Steering_Angle - Current_Steering_Angle);

    //方案2 舵轮目标角度增加180度 轮速反向
    float Second_Way_Delta_Angle = Wrap(Raw_Target_Steering_Angle + 180.0f - Current_Steering_Angle);

    //判断方案优劣
    if(fabsf(First_Way_Delta_Angle) < fabsf(Second_Way_Delta_Angle))//第一种转的角度小
    {
        this->Motor[Motor_Index].Target.Steering_Angle = Current_Steering_Angle + First_Way_Delta_Angle;
        return First_Way_Delta_Angle;
    }
    else//第二种小
    {
        this->Motor[Motor_Index].Target.Steering_Angle = Current_Steering_Angle + Second_Way_Delta_Angle;
        this->Motor[Motor_Index].Target.Wheel_Linear_Speed = -this->Motor[Motor_Index].Target.Wheel_Linear_Speed;//反向
        return Second_Way_Delta_Angle;
    }
}

/**
 * @brief 获取当前电机组的纵向轴距向量（得到带方向的纵向轴距）Xi
 * 
 * @param Motor_Index 电机索引
 * @return int8_t 纵向轴距
 */
float Class_SteeringWheel_Chassis_Calculation::Get_Now_Motor_Group_Vector_a(uint8_t Motor_Index)
{
    //前X左Y上Z a是纵向轴距 以y轴对成
    switch(Motor_Index)
    {
        case 0:
            return 1.0 * Chassis_a;//第一象限
        case 1:
            return -1.0 * Chassis_a;//第二象限
        case 2:
            return -1.0 * Chassis_a;//第三象限
        case 3:
            return 1.0 * Chassis_a;//第四象限
        default:
            return 0;
    }
}

/**
 * @brief 获取当前电机组的横向轴距向量（得到带方向的横向轴距）Yi
 * 
 * @param Motor_Index 电机索引
 * @return float 横向轴距
 */
float Class_SteeringWheel_Chassis_Calculation::Get_Now_Motor_Group_Vector_b(uint8_t Motor_Index)
{
    //前X左Y上Z b是横向轴距 以x轴对成
    switch(Motor_Index)
    {
        case 0:
            return 1.0 * Chassis_b;//第一象限
        case 1:
            return 1.0 * Chassis_b;//第二象限
        case 2:
            return -1.0 * Chassis_b;//第三象限
        case 3:
            return -1.0 * Chassis_b;//第四象限
        default:
            return 0;
    }
}

/**
 * @brief 查找最大线速度的电机索引值
 * 
 * @return uint8_t 最大线速度的电机索引值 
 */
uint8_t Class_SteeringWheel_Chassis_Calculation::Find_Max_Linear_Speed(void)
{
    float Max_Wheel_Speed = 0.0f;
    uint8_t Max_Speed_Motor_Index = 0;

    for(uint8_t i = 0; i < 4; i++)
    {
        if(fabsf(Motor[i].Target.Wheel_Linear_Speed) > Max_Wheel_Speed)
        {
            Max_Wheel_Speed = fabsf(Motor[i].Target.Wheel_Linear_Speed);
            Max_Speed_Motor_Index = i;
        }
    }

    return Max_Speed_Motor_Index;
}

/**
 * @brief 将角度限制在-180~180度之间
 * 
 * @param Angle 角度值
 * @return float 限制后的角度值
 */
float Class_SteeringWheel_Chassis_Calculation::Wrap(float Angle)
{
    while(Angle > 180.0f)
    {
        Angle -= 360.0f;
    }
    while(Angle < -180.0f)
    {
        Angle += 360.0f;
    }
    return Angle;
}