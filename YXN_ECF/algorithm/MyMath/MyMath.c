/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    MyMath.c
  * @brief   数学计算相关库
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "MyMath.h"

float MyMath_Abs(float value)
{
  if (value < 0.0f)
  {
    return -value;
  }

  return value;
}
