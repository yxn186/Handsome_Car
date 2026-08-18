/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_encoder.cpp
  * @brief   编码器库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_encoder.h"
#include <cstdint>

/**
 * @brief 编码器初始化
 * 
 * @param Encoder_htim 编码器定时器句柄
 * @param dt_s 时间间隔设置 为Update函数被调用的周期 单位为秒
 * @param Encoder_Number 编码器脉冲数
 * @param Encoder_Multiple 编码器CubeMX设置的倍增数
 * @param Gear_Ratio 减速比
 * @param Encoder_Direction 编码器方向 1或-1 便于人为调整 默认是1
 */
void Class_Encoder::Init(TIM_HandleTypeDef *Encoder_htim, 
                        float dt_s, int32_t Encoder_Number, int16_t Encoder_Multiple,
                        float Gear_Ratio,int8_t Encoder_Direction)
{
    this->Encoder_htim = Encoder_htim;
    this->dt_s = dt_s;
    this->Counts_Per_Revolution = Encoder_Number * Encoder_Multiple;
    this->Gear_Ratio = Gear_Ratio;
    this->Counts_Per_Output_Revolution = Counts_Per_Revolution * Gear_Ratio;
    this->Encoder_Direction = Encoder_Direction;

    __HAL_TIM_SET_COUNTER(this->Encoder_htim, 0);

    Last_Count = 0;
    Speed_Count = 0;
    Total_Count = 0;

    HAL_TIM_Encoder_Start(this->Encoder_htim, TIM_CHANNEL_ALL);
}

/**
 * @brief 获取编码器计数值
 * 
 * @return int32_t 编码器计数值
 */
int32_t Class_Encoder::Get_RawCount()
{
    return (int32_t)__HAL_TIM_GET_COUNTER(this->Encoder_htim);
}

/**
 * @brief 重置编码器计数值为0
 * 
 */
void Class_Encoder::Reset_Count(void)
{
    __HAL_TIM_SET_COUNTER(this->Encoder_htim, 0);

    Last_Count = 0;
    Speed_Count = 0;
    Total_Count = 0;
}

/**
 * @brief 更新编码器状态，计算速度和总计数
 * 
 */
void Class_Encoder::Update(void)
{
    uint32_t Now_Count = __HAL_TIM_GET_COUNTER(this->Encoder_htim);

    //得到计数器周期
    uint64_t Period = (uint64_t)__HAL_TIM_GET_AUTORELOAD(this->Encoder_htim) + 1ULL;

    //计算普通差值
    int64_t Delta = (int64_t)Now_Count - (int64_t)Last_Count;

    //处理计数器溢出情况
    if (Delta > (int64_t)(Period / 2ULL))
    {
        Delta -= (int64_t)Period;//正向溢出
    }
    else if (Delta < -(int64_t)(Period / 2ULL))
    {
        Delta += (int64_t)Period;//反向溢出
    }

    Speed_Count = (int32_t)Delta;

    Total_Count += Speed_Count;

    Last_Count = Now_Count;

    //计算尾轴RPM
    RPM = (Speed_Count * 60.0f / (Counts_Per_Revolution * dt_s)) * Encoder_Direction;

    //计算输出轴RPM
    Output_RPM = (Speed_Count * 60.0f / (Counts_Per_Output_Revolution * dt_s)) * Encoder_Direction;

    //计算输出轴角速度
    Output_W = Output_RPM * 2.0f * 3.1415926f / 60.0f;
}
