/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_i2c.cpp
  * @brief   I2C库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_i2c.h"
#include <stdint.h>

/*YOUR CODE*/
Struct_I2C_Manage_Object I2C1_Manage_Object = {nullptr};
Struct_I2C_Manage_Object I2C2_Manage_Object = {nullptr};
Struct_I2C_Manage_Object I2C3_Manage_Object = {nullptr};


/**
 * @brief 初始化I2C
 *
 * @param hi2c I2C编号
 * @param Callback_Function 处理回调函数
 */
void I2C_Init(I2C_HandleTypeDef *hi2c, I2C_Callback Callback_Function)
{
    if (hi2c->Instance == I2C1)
    {
        I2C1_Manage_Object.I2C_Handler = hi2c;
        I2C1_Manage_Object.Callback_Function = Callback_Function;
    }
    else if (hi2c->Instance == I2C2)
    {
        I2C2_Manage_Object.I2C_Handler = hi2c;
        I2C2_Manage_Object.Callback_Function = Callback_Function;
    }
    else if (hi2c->Instance == I2C3)
    {
        I2C3_Manage_Object.I2C_Handler = hi2c;
        I2C3_Manage_Object.Callback_Function = Callback_Function;
    }
}

/**
 * @brief 发送数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Tx_Buffer 发送缓冲区
 * @param Tx_Length 发送长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Transmit_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Tx_Buffer, uint16_t Tx_Length)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Tx_Buffer == nullptr) return HAL_ERROR;
    if (Tx_Length == 0) return HAL_ERROR;

    if (hi2c->Instance == I2C1)
    {
        I2C1_Manage_Object.Dev_Address = Dev_Address;
        I2C1_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C1_Manage_Object.Rx_Buffer_Length = 0;
        I2C1_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C1_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Master_Transmit_DMA(hi2c, Dev_Address, I2C1_Manage_Object.Tx_Buffer, Tx_Length));
    }
    else if (hi2c->Instance == I2C2)
    {
        I2C2_Manage_Object.Dev_Address = Dev_Address;
        I2C2_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C2_Manage_Object.Rx_Buffer_Length = 0;
        I2C2_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C2_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Master_Transmit_DMA(hi2c, Dev_Address, I2C2_Manage_Object.Tx_Buffer, Tx_Length));
    }
    else if (hi2c->Instance == I2C3)
    {
        I2C3_Manage_Object.Dev_Address = Dev_Address;
        I2C3_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C3_Manage_Object.Rx_Buffer_Length = 0;
        I2C3_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C3_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Master_Transmit_DMA(hi2c, Dev_Address, I2C3_Manage_Object.Tx_Buffer, Tx_Length));
    }

    return (HAL_ERROR);
}

/**
 * @brief 接收数据
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Rx_Buffer 接收缓冲区
 * @param Rx_Length 接收长度
 * @return uint8_t 执行状态
 */
uint8_t I2C_Receive_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Rx_Buffer, uint16_t Rx_Length)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Rx_Buffer == nullptr) return HAL_ERROR;
    if (Rx_Length == 0) return HAL_ERROR;

    if (hi2c->Instance == I2C1)
    {
        I2C1_Manage_Object.Dev_Address = Dev_Address;
        I2C1_Manage_Object.Tx_Buffer_Length = 0;
        I2C1_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C1_Manage_Object.Tx_Buffer = nullptr;
        I2C1_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Master_Receive_DMA(hi2c, Dev_Address, I2C1_Manage_Object.Rx_Buffer, Rx_Length));
    }
    else if (hi2c->Instance == I2C2)
    {
        I2C2_Manage_Object.Dev_Address = Dev_Address;
        I2C2_Manage_Object.Tx_Buffer_Length = 0;
        I2C2_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C2_Manage_Object.Tx_Buffer = nullptr;
        I2C2_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Master_Receive_DMA(hi2c, Dev_Address, I2C2_Manage_Object.Rx_Buffer, Rx_Length));
    }
    else if (hi2c->Instance == I2C3)
    {
        I2C3_Manage_Object.Dev_Address = Dev_Address;
        I2C3_Manage_Object.Tx_Buffer_Length = 0;
        I2C3_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C3_Manage_Object.Tx_Buffer = nullptr;
        I2C3_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Master_Receive_DMA(hi2c, Dev_Address, I2C3_Manage_Object.Rx_Buffer, Rx_Length));
    }

    return (HAL_ERROR);
}

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
uint8_t I2C_Mem_Write_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Tx_Buffer, uint16_t Tx_Length)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Tx_Buffer == nullptr) return HAL_ERROR;
    if (Tx_Length == 0) return HAL_ERROR;

    if (hi2c->Instance == I2C1)
    {
        I2C1_Manage_Object.Dev_Address = Dev_Address;
        I2C1_Manage_Object.Mem_Address = Mem_Address;
        I2C1_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C1_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C1_Manage_Object.Rx_Buffer_Length = 0;
        I2C1_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C1_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Mem_Write_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C1_Manage_Object.Tx_Buffer, Tx_Length));
    }
    else if (hi2c->Instance == I2C2)
    {
        I2C2_Manage_Object.Dev_Address = Dev_Address;
        I2C2_Manage_Object.Mem_Address = Mem_Address;
        I2C2_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C2_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C2_Manage_Object.Rx_Buffer_Length = 0;
        I2C2_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C2_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Mem_Write_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C2_Manage_Object.Tx_Buffer, Tx_Length));
    }
    else if (hi2c->Instance == I2C3)
    {
        I2C3_Manage_Object.Dev_Address = Dev_Address;
        I2C3_Manage_Object.Mem_Address = Mem_Address;
        I2C3_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C3_Manage_Object.Tx_Buffer_Length = Tx_Length;
        I2C3_Manage_Object.Rx_Buffer_Length = 0;
        I2C3_Manage_Object.Tx_Buffer = Tx_Buffer;
        I2C3_Manage_Object.Rx_Buffer = nullptr;

        return (HAL_I2C_Mem_Write_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C3_Manage_Object.Tx_Buffer, Tx_Length));
    }

    return (HAL_ERROR);
}

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
uint8_t I2C_Mem_Read_Data(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Rx_Buffer, uint16_t Rx_Length)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Rx_Buffer == nullptr) return HAL_ERROR;
    if (Rx_Length == 0) return HAL_ERROR;

    if (hi2c->Instance == I2C1)
    {
        I2C1_Manage_Object.Dev_Address = Dev_Address;
        I2C1_Manage_Object.Mem_Address = Mem_Address;
        I2C1_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C1_Manage_Object.Tx_Buffer_Length = 0;
        I2C1_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C1_Manage_Object.Tx_Buffer = nullptr;
        I2C1_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Mem_Read_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C1_Manage_Object.Rx_Buffer, Rx_Length));
    }
    else if (hi2c->Instance == I2C2)
    {
        I2C2_Manage_Object.Dev_Address = Dev_Address;
        I2C2_Manage_Object.Mem_Address = Mem_Address;
        I2C2_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C2_Manage_Object.Tx_Buffer_Length = 0;
        I2C2_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C2_Manage_Object.Tx_Buffer = nullptr;
        I2C2_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Mem_Read_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C2_Manage_Object.Rx_Buffer, Rx_Length));
    }
    else if (hi2c->Instance == I2C3)
    {
        I2C3_Manage_Object.Dev_Address = Dev_Address;
        I2C3_Manage_Object.Mem_Address = Mem_Address;
        I2C3_Manage_Object.Mem_Address_Size = Mem_Address_Size;
        I2C3_Manage_Object.Tx_Buffer_Length = 0;
        I2C3_Manage_Object.Rx_Buffer_Length = Rx_Length;
        I2C3_Manage_Object.Tx_Buffer = nullptr;
        I2C3_Manage_Object.Rx_Buffer = Rx_Buffer;

        return (HAL_I2C_Mem_Read_DMA(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, I2C3_Manage_Object.Rx_Buffer, Rx_Length));
    }

    return (HAL_ERROR);
}

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
uint8_t I2C_Transmit_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Tx_Buffer, uint16_t Tx_Length, uint32_t Timeout)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Tx_Buffer == nullptr) return HAL_ERROR;
    if (Tx_Length == 0) return HAL_ERROR;

    // 如果I2C正在忙（可能DMA还没结束），直接返回忙
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;

    // 阻塞发送
    HAL_StatusTypeDef state = HAL_I2C_Master_Transmit(hi2c, Dev_Address, Tx_Buffer, Tx_Length, Timeout);

    return (uint8_t)state;
}

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
uint8_t I2C_Receive_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint8_t *Rx_Buffer, uint16_t Rx_Length, uint32_t Timeout)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Rx_Buffer == nullptr) return HAL_ERROR;
    if (Rx_Length == 0) return HAL_ERROR;

    // 如果I2C正在忙（可能DMA还没结束），直接返回忙
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;

    // 阻塞接收
    HAL_StatusTypeDef state = HAL_I2C_Master_Receive(hi2c, Dev_Address, Rx_Buffer, Rx_Length, Timeout);

    return (uint8_t)state;
}

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
uint8_t I2C_Mem_Write_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Tx_Buffer, uint16_t Tx_Length, uint32_t Timeout)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Tx_Buffer == nullptr) return HAL_ERROR;
    if (Tx_Length == 0) return HAL_ERROR;

    // 如果I2C正在忙（可能DMA还没结束），直接返回忙
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;

    // 阻塞写寄存器
    HAL_StatusTypeDef state = HAL_I2C_Mem_Write(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, Tx_Buffer, Tx_Length, Timeout);

    return (uint8_t)state;
}

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
uint8_t I2C_Mem_Read_Data_Blocking(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint16_t Mem_Address, uint16_t Mem_Address_Size, uint8_t *Rx_Buffer, uint16_t Rx_Length, uint32_t Timeout)
{
    if (hi2c == nullptr) return HAL_ERROR;
    if (Rx_Buffer == nullptr) return HAL_ERROR;
    if (Rx_Length == 0) return HAL_ERROR;

    // 如果I2C正在忙（可能DMA还没结束），直接返回忙
    if (hi2c->State != HAL_I2C_STATE_READY) return HAL_BUSY;

    // 阻塞读寄存器
    HAL_StatusTypeDef state = HAL_I2C_Mem_Read(hi2c, Dev_Address, Mem_Address, Mem_Address_Size, Rx_Buffer, Rx_Length, Timeout);

    return (uint8_t)state;
}

/**
 * @brief 检测I2C设备是否在线
 *
 * @param hi2c I2C编号
 * @param Dev_Address 从机设备地址，注意使用HAL格式，也就是7位地址左移1位
 * @param Trials 尝试次数
 * @param Timeout 超时（ms）
 * @return uint8_t 执行状态（HAL_OK / HAL_BUSY / HAL_ERROR / HAL_TIMEOUT）
 */
uint8_t I2C_Check_Device(I2C_HandleTypeDef *hi2c, uint16_t Dev_Address, uint32_t Trials, uint32_t Timeout)
{
    if (hi2c == nullptr) return HAL_ERROR;

    HAL_StatusTypeDef state = HAL_I2C_IsDeviceReady(hi2c, Dev_Address, Trials, Timeout);

    return (uint8_t)state;
}

/**
 * @brief HAL库I2C发送完成回调函数
 *
 * @param hi2c I2C编号
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (I2C1_Manage_Object.Callback_Function != nullptr)
        {
            I2C1_Manage_Object.Callback_Function(I2C1_Manage_Object.Tx_Buffer, NULL, I2C1_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
    else if (hi2c->Instance == I2C2)
    {
        if (I2C2_Manage_Object.Callback_Function != nullptr)
        {
            I2C2_Manage_Object.Callback_Function(I2C2_Manage_Object.Tx_Buffer, NULL, I2C2_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
    else if (hi2c->Instance == I2C3)
    {
        if (I2C3_Manage_Object.Callback_Function != nullptr)
        {
            I2C3_Manage_Object.Callback_Function(I2C3_Manage_Object.Tx_Buffer, NULL, I2C3_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
}

/**
 * @brief HAL库I2C接收完成回调函数
 *
 * @param hi2c I2C编号
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        //I2C1_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C1_Manage_Object.Callback_Function != nullptr)
        {
            I2C1_Manage_Object.Callback_Function(NULL, I2C1_Manage_Object.Rx_Buffer, 0, I2C1_Manage_Object.Rx_Buffer_Length);
        }
    }
    else if (hi2c->Instance == I2C2)
    {
        //I2C2_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C2_Manage_Object.Callback_Function != nullptr)
        {
            I2C2_Manage_Object.Callback_Function(NULL, I2C2_Manage_Object.Rx_Buffer, 0, I2C2_Manage_Object.Rx_Buffer_Length);
        }
    }
    else if (hi2c->Instance == I2C3)
    {
        //I2C3_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C3_Manage_Object.Callback_Function != nullptr)
        {
            I2C3_Manage_Object.Callback_Function(NULL, I2C3_Manage_Object.Rx_Buffer, 0, I2C3_Manage_Object.Rx_Buffer_Length);
        }
    }
}

/**
 * @brief HAL库I2C寄存器写完成回调函数
 *
 * @param hi2c I2C编号
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (I2C1_Manage_Object.Callback_Function != nullptr)
        {
            I2C1_Manage_Object.Callback_Function(I2C1_Manage_Object.Tx_Buffer, NULL, I2C1_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
    else if (hi2c->Instance == I2C2)
    {
        if (I2C2_Manage_Object.Callback_Function != nullptr)
        {
            I2C2_Manage_Object.Callback_Function(I2C2_Manage_Object.Tx_Buffer, NULL, I2C2_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
    else if (hi2c->Instance == I2C3)
    {
        if (I2C3_Manage_Object.Callback_Function != nullptr)
        {
            I2C3_Manage_Object.Callback_Function(I2C3_Manage_Object.Tx_Buffer, NULL, I2C3_Manage_Object.Tx_Buffer_Length, 0);
        }
    }
}

/**
 * @brief HAL库I2C寄存器读完成回调函数
 *
 * @param hi2c I2C编号
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        //I2C1_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C1_Manage_Object.Callback_Function != nullptr)
        {
            I2C1_Manage_Object.Callback_Function(NULL, I2C1_Manage_Object.Rx_Buffer, 0, I2C1_Manage_Object.Rx_Buffer_Length);
        }
    }
    else if (hi2c->Instance == I2C2)
    {
        //I2C2_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C2_Manage_Object.Callback_Function != nullptr)
        {
            I2C2_Manage_Object.Callback_Function(NULL, I2C2_Manage_Object.Rx_Buffer, 0, I2C2_Manage_Object.Rx_Buffer_Length);
        }
    }
    else if (hi2c->Instance == I2C3)
    {
        //I2C3_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

        if (I2C3_Manage_Object.Callback_Function != nullptr)
        {
            I2C3_Manage_Object.Callback_Function(NULL, I2C3_Manage_Object.Rx_Buffer, 0, I2C3_Manage_Object.Rx_Buffer_Length);
        }
    }
}
