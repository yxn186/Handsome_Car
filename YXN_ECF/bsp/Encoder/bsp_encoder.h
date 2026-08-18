/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_encoder.h
  * @brief   This file contains all the function prototypes for
  *          the bsp_encoder.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

class Class_Encoder
{
    public:

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
        void Init(TIM_HandleTypeDef *Encoder_htim, 
                        float dt_s, int32_t Encoder_Number, int16_t Encoder_Multiple,
                        float Gear_Ratio,int8_t Encoder_Direction = 1);

        /**
         * @brief 获取编码器原始计数值
         * 
         * @return int32_t 编码器原始计数值
         */
        int32_t Get_RawCount();

        /**
        * @brief 获取本次 Update 与上次 Update 之间的增量
        */
        int32_t Get_SpeedCount(void)
        {
            return Speed_Count;
        }

        /**
        * @brief 获取累计编码器计数
        */
        int64_t Get_TotalCount(void)
        {
            return Total_Count;
        }

        /**
         * @brief 获取经过 Encoder_Direction 修正后的累计编码器计数
         */
        int64_t Get_Directed_TotalCount(void)
        {
            return Total_Count * (int64_t)Encoder_Direction;
        }

        /**
         * @brief 重置编码器计数值为0
         */
        void Reset_Count();

        /**
         * @brief 更新编码器状态，计算速度和总计数
         */
        void Update(void);

        /**
         * @brief 获取编码器转速，单位为RPM
         * 
         * @return float 编码器转速
         */
        float Get_RPM(void)
        {
            return RPM;
        }

        /**
         * @brief 获取编码器输出轴转速，单位为RPM
         * 
         * @return float 编码器输出轴转速
         */
        float Get_Output_RPM(void)
        {
            return Output_RPM;
        }

        /**
         * @brief 获取编码器输出轴角速度，单位为rad/s
         * 
         * @return float 编码器输出轴角速度
         */
        float Get_Output_W(void)
        {
            return Output_W;
        }

    private:
        TIM_HandleTypeDef *Encoder_htim;

        uint32_t Last_Count = 0;
        int32_t Speed_Count = 0;
        int64_t Total_Count = 0;

        float RPM = 0.0f;
        float Output_RPM = 0.0f;
        float Output_W = 0.0f;

        int32_t Counts_Per_Revolution = 0;
        float Counts_Per_Output_Revolution = 0.0f;
        float dt_s = 0.0f;
        float Gear_Ratio = 0.0f;

        int8_t Encoder_Direction = 1;
};






#ifdef __cplusplus
}
#endif

#endif /* __BSP_ENCODER_H__ */
