/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    SlopePlaning.h
  * @brief   This file contains all the function prototypes for
  *          the SlopePlaning.c file
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SLOPEPLANING_H__
#define __SLOPEPLANING_H__


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief 规划优先类型, 分为目标值优先和真实值优先
 * 目标值优先, 即硬规划
 * 真实值优先, 即当前真实值夹在当前规划值和目标值之间, 当前规划值转为当前真实值
 *
 */
typedef enum 
{
    SlopePlaning_REAL = 0,
    SlopePlaning_TARGET,
}SlopePlning_Mode_e;

class Class_SlopePlaning
{
public:

    /**
    * @brief 初始化
    * @details 初始化斜坡规划器相关参数 Increase_Value = Increase_a * dt Decrease_Value = Decrease_a * dt
    * @param Increase_a 增长加速度
    * @param Decrease_a 减小加速度
    * @param dt 计算周期
    * @param SlopePlaning_Mode 规划优先类型
    */
    void Init(float Increase_a, float Decrease_a, float dt, SlopePlning_Mode_e SlopePlaning_Mode);

    inline float Get_After_Target();

    inline void Set_Now_Real(float __Now_Real);

    inline void Set_Increase_Value(float __Increase_Value);

    inline void Set_Decrease_Value(float __Decrease_Value);

    inline void Set_Target(float __Target);

    /**
    * @brief 重置斜坡规划内部状态
    * @param Reset_Value 重置后的规划值/目标值/输出值
    */
    void Reset(float Reset_Value = 0.0f);

    /**
    * @brief 斜坡计算循环函数
    *
    */
    void Calculate_Loop();

protected:
    // 初始化相关常量

    // 常量

    // 内部变量

    // 读变量

    // 输出值
    float After_Target = 0.0f;

    // 写变量

    // 规划优先类型
    SlopePlning_Mode_e SlopePlaning_Mode = SlopePlaning_REAL;
    // 当前规划值
    float Now_Planning = 0.0f;

    // 当前真实值 外部系统真实值
    float Now_Real = 0.0f;

    // 绝对值增量, 一次计算周期改变值
    float Increase_Value = 0.0f;
    // 绝对值减量, 一次计算周期改变值
    float Decrease_Value = 0.0f;
    // 目标值
    float Target = 0.0f;

    float dt = 0.001f; //计算周期, 单位s
    // 读写变量

    // 内部函数 
};

/**
 * @brief 获取输出值
 *
 * @return 输出值
 */
inline float Class_SlopePlaning::Get_After_Target()
{
    return (After_Target);
}

/**
 * @brief 设定当前真实值
 *
 * @param Now_Real_Value 当前真实值
 */
inline void Class_SlopePlaning::Set_Now_Real(float Now_Real_Value)
{
    Now_Real = Now_Real_Value;
}

/**
 * @brief 设定加速度 间接设定绝对值增量, 一次计算周期改变值
 *
 * @param Increase_a 加速度
 */
inline void Class_SlopePlaning::Set_Increase_Value(float Increase_a)
{
    Increase_Value = Increase_a * dt;
}

/**
 * @brief 设定减速度 间接设定绝对值减量, 一次计算周期改变值
 *
 * @param Decrease_a 减速度
 */
inline void Class_SlopePlaning::Set_Decrease_Value(float Decrease_a)
{
    Decrease_Value = Decrease_a * dt;
}

/**
 * @brief 设定目标值
 *
 * @param Target_Value 目标值
 */
inline void Class_SlopePlaning::Set_Target(float Target_Value)
{
    Target = Target_Value;
}





#endif /* __SLOPEPLANING_H__ */
