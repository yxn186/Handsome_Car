/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    SlopePlaning.cpp
  * @brief   斜坡规划器
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "SlopePlaning.h"
#include "MyMath.h"

/**
 * @brief 初始化
 * @details 初始化斜坡规划器相关参数 Increase_Value = Increase_a * dt Decrease_Value = Decrease_a * dt
 * @param Increase_a 增长加速度
 * @param Decrease_a 减小加速度
 * @param dt_value 计算周期
 * @param SlopePlaning_Mode 规划优先类型
 */
void Class_SlopePlaning::Init(float Increase_a, float Decrease_a, float dt_value, SlopePlning_Mode_e SlopePlaning_Mode)
{
    dt = dt_value;
    Increase_Value = Increase_a * dt;
    Decrease_Value = Decrease_a * dt;
    this->SlopePlaning_Mode = SlopePlaning_Mode;
}

/**
 * @brief 重置斜坡规划内部状态
 * @param Reset_Value 重置后的规划值/目标值/输出值
 */
void Class_SlopePlaning::Reset(float Reset_Value)
{
    After_Target = Reset_Value;
    Now_Planning = Reset_Value;
    Now_Real = Reset_Value;
    Target = Reset_Value;
}

/**
 * @brief 斜坡计算循环函数
 *
 */
void Class_SlopePlaning::Calculate_Loop()
{
    // 规划为当前真实值优先的额外逻辑
    if (SlopePlaning_Mode == SlopePlaning_REAL)
    {
        // 当前真实值已经位于规划值与目标值之间时，直接用真实值接管规划输出
        if ((Target >= Now_Real && Now_Real >= Now_Planning) || (Target <= Now_Real && Now_Real <= Now_Planning))
        {
            After_Target = Now_Real;
        }
    }

    // 按当前规划值所在区间分别处理，正值、负值和零点附近的加减速方向不同
    if (Now_Planning > 0.0f)//规划值为正
    {
        if (Target > Now_Planning)//目标更正 需要加速
        {
            // 正值加速
            // 距离目标还较远时按加速步长逼近，否则直接到达目标避免抖动
            if (MyMath_Abs(Now_Planning - Target) > Increase_Value)
            {
                After_Target += Increase_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
        else if (Target < Now_Planning)//目标更负 需要减速
        {
            // 正值减速
            // 朝更小的目标回落，使用减速步长限制单次变化量
            if (MyMath_Abs(Now_Planning - Target) > Decrease_Value)
            {
                After_Target -= Decrease_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
    }
    else if (Now_Planning < 0.0f)//规划值为负
    {
        if (Target < Now_Planning)//目标更负 需要加速
        {
            // 负值加速
            // 负向继续增大绝对值时，输出向更小方向推进
            if (MyMath_Abs(Now_Planning - Target) > Increase_Value)
            {
                After_Target -= Increase_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
        else if (Target > Now_Planning)//目标更正 需要减速
        {
            // 负值减速
            // 目标向零点或正方向回归时，输出按减速步长抬升
            if (MyMath_Abs(Now_Planning - Target) > Decrease_Value)
            {
                After_Target += Decrease_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
    }
    else
    {
        // 当前规划值在零点时，直接根据目标方向决定正向或负向起步
        if (Target > Now_Planning)
        {
            // 0值正加速
            if (MyMath_Abs(Now_Planning - Target) > Increase_Value)
            {
                After_Target += Increase_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
        else if (Target < Now_Planning)
        {
            // 0值负加速
            if (MyMath_Abs(Now_Planning - Target) > Increase_Value)
            {
                After_Target -= Increase_Value;
            }
            else
            {
                After_Target = Target;
            }
        }
    }

    // 将本次输出回写为下一周期的规划起点
    Now_Planning = After_Target;
}