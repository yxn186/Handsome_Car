# YXN_ECF

YXN Embedded Common Framework

用于存放多个 STM32 主控工程之间可复用的嵌入式 C/C++ 代码。


## 使用原则

本仓库不包含：

- STM32CubeMX 生成代码
- main.c
- freertos.c
- 具体主控的 GPIO 配置
- 具体主控的外设初始化
- 云台和底盘应用层控制逻辑
- 编译输出文件