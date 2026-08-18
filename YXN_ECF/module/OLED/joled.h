/**
 * @file    joled.h
 * @brief   JOLED 旧版 OLED 接口兼容层（基于新库 oled.c 的帧缓冲实现）
 * @details
 * 该文件提供一组与旧版 OLED 库相同“行/列 + 固定 8x16 字符”的快捷接口，
 * 底层仍然使用新库的 OLED_GRAM + OLED_ShowFrame() 机制。
 *
 * 典型使用：
 * 1) JOLED_Init()
 * 2) JOLED_Clear()
 * 3) JOLED_ShowString / JOLED_ShowNum / ...
 *
 * 默认行为：每次 JOLED_xxx 调用后都会自动刷新（OLED_ShowFrame）。
 * 若需要批量写入以提升效率，可调用 JOLED_SetAutoRefresh(0)，最后手动 JOLED_Refresh()。
 */

#ifndef __JOLED_H__
#define __JOLED_H__

#include <stdint.h>

/*选择是否使用DMA版OLED*/
/*使用DMA版OLED需注意I2C中断回调函数的使用！*/
#include "oled_DMA.h"

//#include "oled_DMA.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化 JOLED（内部会调用新库 OLED_Init）
 * @note   需要确保 I2C 等底层外设已经初始化（如 MX_I2C1_Init 已执行）
 * @note   本兼容层默认使用 128x64 屏幕、8x16 ASCII 字体（afont16x8）
 */
void JOLED_Init(void);

/**
 * @brief  清屏（清空帧缓冲并立即刷新到屏幕）
 * @note   等价于：OLED_NewFrame() + OLED_ShowFrame()
 */
void JOLED_Clear(void);

/**
 * @brief  设置是否自动刷新
 * @param  enabled
 *         - 0：关闭自动刷新（JOLED_xxx 只写入显存，不刷屏）
 *         - 非 0：开启自动刷新（默认，JOLED_xxx 写入后自动刷屏）
 * @note   关闭自动刷新后，建议最后调用一次 JOLED_Refresh()
 */
void JOLED_SetAutoRefresh(uint8_t enabled);

/**
 * @brief  手动刷新（把当前帧缓冲整帧刷到 OLED）
 * @note   等价于：OLED_ShowFrame()
 */
void JOLED_Refresh(void);

/**
 * @brief  在指定行列显示一个字符（8x16）
 * @param  Line   行号（旧库习惯：1~4）
 * @param  Column 列号（旧库习惯：1~16）
 * @param  Char   要显示的 ASCII 字符（不可见字符会被替换为空格）
 */
void JOLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

/**
 * @brief  在指定行列显示字符串（8x16，超出列范围会被裁剪）
 * @param  Line   行号（1~4）
 * @param  Column 列号（1~16）
 * @param  String 以 '\0' 结尾的 C 字符串指针（NULL 会被忽略）
 * @note   字符串过长会在到达第 16 列时停止（不自动换行）
 */
void JOLED_ShowString(uint8_t Line, uint8_t Column, char *String);

/**
 * @brief  显示无符号十进制数字（固定长度，前导补 0）
 * @param  Line   行号（1~4）
 * @param  Column 列号（1~16）
 * @param  Number 数值
 * @param  Length 显示长度（例如 Length=3：007、123）
 */
void JOLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  显示有符号十进制数字（固定格式：符号位 + Length 位数字）
 * @param  Line   行号（1~4）
 * @param  Column 列号（1~16）
 * @param  Number 有符号数值
 * @param  Length 数字位数（不含符号位）
 * @note   显示总宽度为 Length+1：例如 "+001" 或 "-123"
 */
void JOLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

/**
 * @brief  显示十六进制数字（固定长度，大写 A~F）
 * @param  Line   行号（1~4）
 * @param  Column 列号（1~16）
 * @param  Number 数值
 * @param  Length 显示长度（例如 Length=4：1A3F）
 */
void JOLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  显示二进制数字（固定长度）
 * @param  Line   行号（1~4）
 * @param  Column 列号（1~16）
 * @param  Number 数值
 * @param  Length 显示长度（例如 Length=8：01010101）
 */
void JOLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __JOLED_H__ */
