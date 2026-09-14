# Handsome_Car

基于 STM32F407IGH6 的差速底盘控制工程，使用 STM32 HAL、FreeRTOS
（CMSIS-RTOS2）和 CMake/Ninja 构建。工程将 CubeMX 生成代码、可复用公共模块
和本车业务代码分开管理，便于重新生成外设代码和持续扩展应用功能。

## 当前功能

- 差速底盘控制和四个 DJI M3508 电机控制；
- 双 CAN 总线电机通信，CAN 波特率为 1 Mbps；
- DR16 遥控器通过 USART3 + DMA 接收；
- USB FS CDC 与视觉模块通信；
- 巡检灯控制、视觉在线检测和底盘速度回传；
- PID、低通滤波、前馈、姿态和斜坡规划等基础算法。

## 硬件与软件

| 项目 | 配置 |
| --- | --- |
| MCU | STM32F407IGH6 |
| 芯片系列 | STM32F4 |
| 实时系统 | FreeRTOS，CMSIS-RTOS2 接口 |
| 外设 | CAN1、CAN2、USART3、USB OTG FS、DMA、TIM4 |
| 编译语言 | C11、C++17、ARM 汇编 |
| 构建工具 | CMake 3.22+、Ninja、GNU Arm Embedded Toolchain |
| 配置文件 | `Handsome_Car.ioc` |

## 目录结构

```text
Handsome_Car/
├── Core/                    CubeMX 生成的应用、外设和 FreeRTOS 代码
├── Drivers/                STM32 HAL 和 CMSIS
├── Middlewares/            FreeRTOS 和 USB Device Library
├── USB_DEVICE/             USB CDC 设备实现
├── YXN_ECF/                通用算法、BSP 和功能模块
├── Usercode/               本车专用 App 与 FreeRTOS 任务
│   ├── Application/        底盘、命令、灯光、遥控和视觉应用
│   └── Handsome_Car_Task/  初始化、主周期和 USB 任务
├── CMakeLists.txt          工程及 YXN_ECF 接入配置
├── Usercode/CMakeLists.txt Usercode 源文件清单
├── CMakePresets.json       Debug/Release 构建预设
└── Handsome_Car.ioc        STM32CubeMX 工程配置
```

## 任务与应用流程

`Usercode/Handsome_Car_Task/Handsome_Car_Task.cpp` 覆盖 CubeMX 在
`Core/Src/freertos.c` 中声明的 weak 任务入口：

- `InitTaskFunction`：初始化视觉、USB、底盘、灯光、遥控和命令模块；
- `MainTaskFunction`：每 1 ms 更新遥控、命令、底盘和视觉在线状态；
- `USBTaskFunction`：每 1 ms 发送灯光状态与底盘当前速度。

初始化完成后，`Global_Init_Finished` 才会允许 USART 和 USB 接收回调向应用层
分发数据。这样无需修改 CubeMX 生成的 `freertos.c`，重新生成代码时任务实现
仍保留在 `Usercode` 中。

## 构建

请先准备 GNU Arm Embedded Toolchain 和 Ninja，并确保 `cmake` 命令可用。在
工程根目录执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug --parallel 4
```

Release 构建：

```powershell
cmake --preset Release
cmake --build --preset Release --parallel 4
```

构建产物位于对应的构建目录，例如：

```text
build/Debug/Handsome_Car.elf
build/Debug/Handsome_Car.map
```

也可以在 VS Code 的 CMake Tools 中选择 `Debug` 或 `Release` preset 后执行
Configure 和 Build。不要在同一个构建目录中混用不同版本的 CMake 或工具链。

## 源码接入规则

工程不使用 `GLOB` 自动扫描源码，所有实现文件都必须显式加入 CMake：

- 本车 App 和任务：编辑 `Usercode/CMakeLists.txt`，分别加入
  `USER_APP_SOURCES` 或 `USER_TASK_SOURCES`；
- 公共算法、BSP 和模块：编辑根目录 `CMakeLists.txt`，同时加入
  `target_sources()` 和对应的 `target_include_directories()`；
- 头文件不需要写入 `target_sources()`；
- `YXN_ECF` 不要重复加入 `Usercode/CMakeLists.txt`。

STM32F407 使用 bxCAN，应接入 `YXN_ECF/bsp/CAN/bxCAN/`，不要启用面向 STM32H7
的 `YXN_ECF/bsp/CAN/FDCAN/`。SPI、I2C、PWM、BMI088、WS2812 等可选模块需要
先在 CubeMX 中生成对应外设、HAL 驱动、句柄和中断，再加入 CMake。

具体添加步骤见 [Usercode 使用说明](Usercode/README.md) 和
[YXN_ECF 使用指南](YXN_ECF/使用指南.md)。

## CubeMX 重新生成

可以通过 `Handsome_Car.ioc` 修改外设和 FreeRTOS 配置，但请注意：

1. 不要手工修改 `cmake/stm32cubemx/CMakeLists.txt`，该文件可能被重新生成；
2. 工程自有代码保持在根目录 `CMakeLists.txt` 和 `Usercode/CMakeLists.txt`；
3. 重新生成后确认 `InitTaskFunction`、`MainTaskFunction` 和
   `USBTaskFunction` 的函数名称与任务配置一致；
4. 新增外设后重新执行 CMake Configure，再进行编译。

## 验证边界

构建成功只代表源码通过编译并生成 ELF，不代表固件已下载或硬件功能正常。
CAN 总线、电机、遥控器、USB 视觉通信以及灯光仍需结合实际接线、供电、设备
ID 和控制参数完成在线调试与实机测试。
