/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Handsome_Car_Task.c
  * @brief   Task层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Handsome_Car_Task.h"

#include "cmsis_os2.h"

/**
 * @brief Project initialization task.
 *
 * Add application and module initialization before osThreadExit(). The strong
 * definition in this file overrides CubeMX's weak implementation.
 */
void InitTaskFunction(void *argument)
{
    (void)argument;

    /* Add project initialization here. */

    osThreadExit();
}

/**
 * @brief Project main periodic task.
 *
 * The generated CubeMX symbol is intentionally spelled MainTaskFunction.
 */
void MainTaskFunction(void *argument)
{
    (void)argument;

    for (;;)
    {
        /* Add the 1 ms project update path here. */
        osDelay(1U);
    }
}
