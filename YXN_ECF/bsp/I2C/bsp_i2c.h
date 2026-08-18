/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_i2c.h
  * @brief   This file contains all the function prototypes for
  *          the bsp_i2c.c file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal_i2c.h"
#include <string.h>
/*YOUR CODE*/

/**
 * @brief I2C通信接收回调函数数据类型
 *
 */
typedef void (*I2C_Callback)(uint8_t *Tx_Buffer, uint8_t *Rx_Buffer, uint16_t Tx_Length, uint16_t Rx_Length);

/**
 * @brief I2C通信处理结构体
 *
 */
struct Struct_I2C_Manage_Object
{
    I2C_HandleTypeDef *I2C_Handler;
    I2C_Callback Callback_Function;

    // 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
    uint16_t Dev_Address;

    // 寄存器/内存地址
    uint16_t Mem_Address;
    uint16_t Mem_Address_Size;

    // 一次收发对应的数据长度
    uint8_t *Tx_Buffer;
    uint8_t *Rx_Buffer;
    uint16_t Tx_Buffer_Length;
    uint16_t Rx_Buffer_Length;

    // 接收时间戳
    uint64_t Rx_Timestamp;
};

/**
 * @brief 初始化I2C
 *
 * @param hi2c I2C编号
 * @param Callback_Function 处理回调函数
 */
void I2C_Init(I2C_HandleTypeDef *hi2c, I2C_Callback Callback_Function);

/**
 * @brief 发送数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Tx_Buffer 发送缓冲区
 * @param Tx_Length 发送长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Transmit_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Tx_Buffer, uint16_t Tx_Length);

/**
 * @brief 接收数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Rx_Buffer 接收缓冲区
 * @param Rx_Length 接收长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Receive_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Rx_Buffer, uint16_t Rx_Length);

/**
 * @brief 写寄存器数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Mem_Address 寄存器/内存地址
 * @param Mem_Address_Size 寄存器/内存地址长度，I2C_MEMADD_SIZE_8BIT 或 I2C_MEMADD_SIZE_16BIT
 * @param Tx_Buffer 发送缓冲区
 * @param Tx_Length 发送长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Mem_Write_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Tx_Buffer, uint16_t Tx_Length);

/**
 * @brief 读寄存器数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Mem_Address 寄存器/内存地址
 * @param Mem_Address_Size 寄存器/内存地址长度，I2C_MEMADD_SIZE_8BIT 或 I2C_MEMADD_SIZE_16BIT
 * @param Rx_Buffer 接收缓冲区
 * @param Rx_Length 接收长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Mem_Read_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Rx_Buffer, uint16_t Rx_Length);

/**
 * @brief 阻塞发送数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Tx_Buffer 发送缓冲区
 * @param Tx_Length 发送长度
 * @param Timeout 超时（ms），比如 10/100，或 HAL_MAX_DELAY
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Transmit_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Tx_Buffer, uint16_t Tx_Length, uint32_t Timeout);

/**
 * @brief 阻塞接收数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Rx_Buffer 接收缓冲区
 * @param Rx_Length 接收长度
 * @param Timeout 超时（ms），比如 10/100，或 HAL_MAX_DELAY
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Receive_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Rx_Buffer, uint16_t Rx_Length, uint32_t Timeout);

/**
 * @brief 阻塞写寄存器数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Mem_Address 寄存器/内存地址
 * @param Mem_Address_Size 寄存器/内存地址长度，I2C_MEMADD_SIZE_8BIT 或 I2C_MEMADD_SIZE_16BIT
 * @param Tx_Buffer 发送缓冲区
 * @param Tx_Length 发送长度
 * @param Timeout 超时（ms），比如 10/100，或 HAL_MAX_DELAY
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Mem_Write_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Tx_Buffer, uint16_t Tx_Length, uint32_t Timeout);

/**
 * @brief 阻塞读寄存器数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Mem_Address 寄存器/内存地址
 * @param Mem_Address_Size 寄存器/内存地址长度，I2C_MEMADD_SIZE_8BIT 或 I2C_MEMADD_SIZE_16BIT
 * @param Rx_Buffer 接收缓冲区
 * @param Rx_Length 接收长度
 * @param Timeout 超时（ms），比如 10/100，或 HAL_MAX_DELAY
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Mem_Read_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Rx_Buffer, uint16_t Rx_Length, uint32_t Timeout);

/**
 * @brief 检测I2C设备是否在线
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Trials 尝试次数
 * @param Timeout 超时（ms）
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Check_Device(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint32_t Trials, uint32_t Timeout);



#ifdef __cplusplus
}
#endif

#endif /* __BSP_I2C_H__ */
