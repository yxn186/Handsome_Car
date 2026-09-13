/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Light.h
  * @brief   App层巡检灯控接口
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __APP_LIGHT_H__
#define __APP_LIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

void App_Light_Init(void);
void App_Light_Update(bool Navigation_Command_Fresh,
                      bool Capture_Enable,
                      bool Remote_Online,
                      float Dial_Wheel);
bool App_Light_Get_Capture_Done(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LIGHT_H__ */
