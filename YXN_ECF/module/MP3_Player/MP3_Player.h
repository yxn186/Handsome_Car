/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    MP3_Player.h
  * @brief   MP3-TF-16P / Mini MP3 Player 串口控制库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MP3_PLAYER_H__
#define __MP3_PLAYER_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include "bsp_usart.h"
/* Exported macros -----------------------------------------------------------*/

// MP3-TF-16P 一帧标准串口命令长度：7E FF 06 CMD FB DH DL CHK_H CHK_L EF
#define MP3_PLAYER_FRAME_LENGTH             10U

// 默认最小发包间隔，避免连续发包太快导致模块忙或漏收
#define MP3_PLAYER_DEFAULT_SEND_INTERVAL_MS 20U

/* Exported types ------------------------------------------------------------*/

/**
 * @brief MP3模块播放设备
 */
typedef enum
{
    MP3_Player_Device_U_Disk = 0x0001,   // U盘
    MP3_Player_Device_TF     = 0x0002,   // TF卡 / SD卡
    MP3_Player_Device_AUX    = 0x0003,   // AUX，部分模块未使用
    MP3_Player_Device_Flash  = 0x0004,   // Flash
    MP3_Player_Device_PC     = 0x0005,   // PC声卡/读卡模式
    MP3_Player_Device_Sleep  = 0x0006    // 设备睡眠
} MP3_Player_Device_Typedef;

/**
 * @brief MP3模块EQ音效
 */
typedef enum
{
    MP3_Player_EQ_Normal  = 0,
    MP3_Player_EQ_Pop     = 1,
    MP3_Player_EQ_Rock    = 2,
    MP3_Player_EQ_Jazz    = 3,
    MP3_Player_EQ_Classic = 4,
    MP3_Player_EQ_Bass    = 5
} MP3_Player_EQ_Typedef;

/**
 * @brief MP3模块播放状态，来自0x42查询返回
 */
typedef enum
{
    MP3_Player_Status_Stopped = 0x0000,  // 停止/播放完毕
    MP3_Player_Status_Playing = 0x0001,  // 正在播放
    MP3_Player_Status_Paused  = 0x0002,  // 暂停
    MP3_Player_Status_Sleep   = 0x0008,  // 睡眠
    MP3_Player_Status_Unknown = 0xFFFF   // 未知/还没有查询到状态
} MP3_Player_Status_Typedef;

/**
 * @brief MP3模块错误码，来自0x40错误返回
 */
typedef enum
{
    MP3_Player_Error_None                = 0x0000,
    MP3_Player_Error_Busy                = 0x0001,  // 模块忙，常见于文件系统初始化
    MP3_Player_Error_Sleep               = 0x0002,  // 当前处于睡眠模式
    MP3_Player_Error_Frame_Not_Complete  = 0x0003,  // 串口一帧没接收完整
    MP3_Player_Error_Checksum            = 0x0004,  // 校验出错
    MP3_Player_Error_File_Out_Of_Range   = 0x0005,  // 文件序号超范围
    MP3_Player_Error_File_Not_Found      = 0x0006,  // 未找到指定文件
    MP3_Player_Error_Advert              = 0x0007   // 插播指令错误
} MP3_Player_Error_Typedef;

/**
 * @brief MP3-TF-16P / Mini MP3 Player 类
 *
 * @note 推荐文件命名方式：TF卡根目录 /MP3/0001.mp3、0002.mp3 ...
 *       然后调用 Play_MP3_Track(1)、Play_MP3_Track(2) 播放。
 */
class Class_MP3_Player
{
public:
    /**
     * @brief 初始化MP3模块串口
     *
     * @param huart MP3模块连接的UART句柄，例如 &huart3
     *
     * @note 本函数会调用 bsp_usart 的 UART_Init 注册接收回调。
     *       请不要和 Serial_Init 使用同一个串口，否则后初始化者会覆盖回调。
     */
    void Init(UART_HandleTypeDef *huart);

    /**
     * @brief 初始化MP3模块串口，并绑定Busy播放状态引脚
     *
     * @param huart MP3模块连接的UART句柄
     * @param busy_gpio_port Busy引脚GPIO端口；不接Busy可传nullptr
     * @param busy_gpio_pin Busy引脚GPIO Pin
     *
     * @note MP3-TF-16P 的 Busy 通常是：播放时低电平，不播放时高电平。
     */
    void Init(UART_HandleTypeDef *huart, GPIO_TypeDef *busy_gpio_port, uint16_t busy_gpio_pin);

    /**
     * @brief 单独设置Busy播放状态引脚
     *
     * @param busy_gpio_port Busy引脚GPIO端口；传nullptr表示不使用Busy引脚
     * @param busy_gpio_pin Busy引脚GPIO Pin
     */
    void Set_Busy_Pin(GPIO_TypeDef *busy_gpio_port, uint16_t busy_gpio_pin);

    /**
     * @brief 设置两条串口命令之间的最小间隔
     *
     * @param interval_ms 间隔时间，单位ms；设置为0表示不主动延时
     */
    void Set_Send_Interval(uint32_t interval_ms);

    /**
     * @brief 上电后快速进入TF卡播放准备状态
     *
     * @param volume 音量，范围0~30
     * @param power_on_delay_ms 上电初始化等待时间，建议1500~3000ms
     * @return HAL_StatusTypeDef HAL_OK表示最后一条命令发送成功
     *
     * @note 这个函数会阻塞等待：先等模块上电初始化，再选TF卡，再设置音量。
     */
    HAL_StatusTypeDef Start_TF_Card(uint8_t volume = 20, uint32_t power_on_delay_ms = 2000);

    /**
     * @brief 发送原始命令
     *
     * @param cmd 命令字节
     * @param param 16位参数
     * @param need_feedback 是否需要应答，false=不需要，true=需要
     * @return HAL_StatusTypeDef 发送状态
     */
    HAL_StatusTypeDef Send_Command(uint8_t cmd, uint16_t param = 0, bool need_feedback = false);

    /* -------------------- 常用播放控制 -------------------- */

    HAL_StatusTypeDef Next(void);                         // 下一曲
    HAL_StatusTypeDef Previous(void);                     // 上一曲
    HAL_StatusTypeDef Play(void);                         // 继续播放
    HAL_StatusTypeDef Pause(void);                        // 暂停
    HAL_StatusTypeDef Stop(void);                         // 停止播放
    HAL_StatusTypeDef Reset(void);                        // 模块复位
    HAL_StatusTypeDef Sleep(void);                        // 进入睡眠

    HAL_StatusTypeDef Set_Volume(uint8_t volume);         // 设置音量，0~30
    HAL_StatusTypeDef Volume_Up(void);                    // 音量+
    HAL_StatusTypeDef Volume_Down(void);                  // 音量-
    HAL_StatusTypeDef Set_EQ(MP3_Player_EQ_Typedef eq);   // 设置EQ
    HAL_StatusTypeDef Set_DAC(bool enable);               // 开启/关闭DAC输出

    HAL_StatusTypeDef Select_Device(MP3_Player_Device_Typedef device); // 指定播放设备

    /**
     * @brief 按物理顺序播放第track首
     *
     * @param track 曲目序号，从1开始
     * @note 这种方式受文件复制顺序影响，不如 Play_MP3_Track() 稳定。
     */
    HAL_StatusTypeDef Play_Root_Track(uint16_t track);

    /**
     * @brief 播放 /MP3 文件夹下的指定曲目
     *
     * @param track 曲目序号，例如1对应 /MP3/0001.mp3
     */
    HAL_StatusTypeDef Play_MP3_Track(uint16_t track);

    /**
     * @brief 播放指定文件夹下的指定曲目
     *
     * @param folder 文件夹编号，1~99，对应文件夹名 01、02 ... 99
     * @param track 曲目编号，1~255，对应文件名前缀 001、002 ... 255
     */
    HAL_StatusTypeDef Play_Folder_Track(uint8_t folder, uint8_t track);

    /**
     * @brief 播放指定文件夹下的曲目，支持更大的曲目编号
     *
     * @param folder 文件夹编号，1~15
     * @param track 曲目编号，1~4095，常用于0001~1999这类命名
     */
    HAL_StatusTypeDef Play_Folder_Track_1000(uint8_t folder, uint16_t track);

    HAL_StatusTypeDef Loop_All(bool enable);              // 全部循环播放开关
    HAL_StatusTypeDef Loop_Specified_Track(uint16_t track);// 循环播放指定曲目
    HAL_StatusTypeDef Loop_Current(bool enable);          // 当前曲目循环开关
    HAL_StatusTypeDef Loop_Folder(uint8_t folder);        // 指定文件夹循环播放
    HAL_StatusTypeDef Random_Play(uint16_t param = 0);    // 随机播放

    HAL_StatusTypeDef Insert_Advert(uint16_t track);      // 插播 /ADVERT/0001.mp3 ...
    HAL_StatusTypeDef Stop_Advert(void);                  // 停止插播，回到背景音乐

    /* -------------------- 查询指令 -------------------- */

    HAL_StatusTypeDef Query_Status(void);                 // 查询当前状态
    HAL_StatusTypeDef Query_Volume(void);                 // 查询当前音量
    HAL_StatusTypeDef Query_EQ(void);                     // 查询当前EQ
    HAL_StatusTypeDef Query_TF_Total_Files(void);         // 查询TF卡总文件数
    HAL_StatusTypeDef Query_TF_Current_Track(void);       // 查询TF卡当前曲目
    HAL_StatusTypeDef Query_Folder_Total_Files(uint8_t folder); // 查询文件夹曲目数

    /* -------------------- 状态读取 -------------------- */

    /**
     * @brief 通过Busy硬件引脚判断是否正在播放
     *
     * @return true 正在播放；false 未播放/未接Busy引脚
     */
    bool Is_Playing_By_BusyPin(void) const;

    /**
     * @brief 通过最近一次解析到的状态判断是否正在播放
     *
     * @return true 正在播放
     */
    bool Is_Playing(void) const;

    bool Is_Module_Online(void) const
    {
        return Module_Online;
    }

    uint16_t Get_Device_Online_Mask(void) const
    {
        return Device_Online_Mask;
    }

    MP3_Player_Status_Typedef Get_Status(void) const
    {
        return Last_Status;
    }

    MP3_Player_Error_Typedef Get_Error(void) const
    {
        return Last_Error;
    }

    uint16_t Get_Last_Command(void) const
    {
        return Last_Rx_Command;
    }

    uint16_t Get_Last_Parameter(void) const
    {
        return Last_Rx_Parameter;
    }

    uint16_t Get_Last_Finished_Track(void) const
    {
        return Last_Finished_Track;
    }

    uint8_t Get_Last_Volume(void) const
    {
        return Last_Volume;
    }

    uint16_t Get_Last_Query_Result(void) const
    {
        return Last_Query_Result;
    }

protected:
    /**
     * @brief USART接收回调入口函数，注册给bsp_usart
     */
    static void UART_RxCallback_Entry(void *context, uint8_t *Buffer, uint16_t Length);

    /**
     * @brief USART接收回调处理函数
     */
    void UART_RxCallback(uint8_t *Buffer, uint16_t Length);

    /**
     * @brief 解析单个字节，拼成完整10字节帧
     */
    void Parse_Byte(uint8_t data);

    /**
     * @brief 处理一帧合法返回数据
     */
    void Process_Frame(const uint8_t frame[MP3_PLAYER_FRAME_LENGTH]);

    /**
     * @brief 计算MP3协议校验
     */
    static uint16_t Calculate_Checksum(uint8_t cmd, uint8_t feedback, uint16_t param);

    /**
     * @brief 检查接收帧是否合法
     */
    static bool Check_Frame(const uint8_t frame[MP3_PLAYER_FRAME_LENGTH]);

private:
    UART_HandleTypeDef *huart = nullptr;

    GPIO_TypeDef *Busy_GPIO_Port = nullptr;
    uint16_t Busy_GPIO_Pin = 0;

    uint32_t Min_Send_Interval_ms = MP3_PLAYER_DEFAULT_SEND_INTERVAL_MS;
    uint32_t Last_Send_Tick = 0;

    uint8_t Tx_Frame[MP3_PLAYER_FRAME_LENGTH] = {0};
    uint8_t Rx_Frame[MP3_PLAYER_FRAME_LENGTH] = {0};
    uint8_t Rx_Frame_Index = 0;

    bool Module_Online = false;
    bool Last_Frame_Valid = false;
    bool Local_Checksum_Error = false;

    uint16_t Device_Online_Mask = 0;
    MP3_Player_Status_Typedef Last_Status = MP3_Player_Status_Unknown;
    MP3_Player_Error_Typedef Last_Error = MP3_Player_Error_None;

    uint16_t Last_Rx_Command = 0;
    uint16_t Last_Rx_Parameter = 0;
    uint16_t Last_Finished_Track = 0;
    uint8_t Last_Finished_Device_Command = 0;
    uint8_t Last_Volume = 0;
    uint8_t Last_EQ = 0;
    uint16_t Last_Query_Result = 0;

    uint32_t Last_Rx_Tick = 0;
    uint32_t Last_Ack_Tick = 0;
};

#endif /* __MP3_PLAYER_H__ */
