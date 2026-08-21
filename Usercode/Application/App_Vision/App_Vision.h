/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Vision.h
  * @brief   App层视觉USB通信接口
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_VISION_H__
#define __APP_VISION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

//============================== 对外接口 ==============================//

/**
 * @brief 初始化视觉USB通信并注册接收回调
 */
void App_Vision_Init(void);

/**
 * @brief 发送仅包含帧头和帧尾的视觉数据帧
 */
void App_Vision_Transmit(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_VISION_H__ */
