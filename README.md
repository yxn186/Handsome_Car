# Handsome_Car

Handsome_Car 是基于 STM32F407IGH6、STM32 HAL 和 FreeRTOS（CMSIS-RTOS2）的
嵌入式工程。工程使用 CMake/Ninja 构建，公共代码放在 `YXN_ECF`，本车专用
应用与任务代码放在 `Usercode`。

## 工程结构

```text
Handsome_Car/
├── Core/                       CubeMX 生成的应用与外设代码
├── Drivers/                    STM32 HAL 和 CMSIS
├── Middlewares/                FreeRTOS 等中间件
├── cmake/stm32cubemx/          CubeMX 生成的 CMake 配置
├── YXN_ECF/                    通用算法、BSP 和功能模块
├── Usercode/                   Handsome_Car 专用代码
│   └── Handsome_Car_Task/      FreeRTOS 任务强定义
├── CMakeLists.txt              工程和 YXN_ECF 接入配置
├── CMakePresets.json           Debug/Release 构建预设
└── Handsome_Car.ioc            CubeMX 工程配置
```

## CMake 分工

### 顶层 CMakeLists.txt

顶层 `CMakeLists.txt` 负责：

- 启用 C、C++ 和汇编；
- 为公共库的 C++ 文件启用 C++17；
- 接入 CubeMX 生成代码；
- 定义 `YXN_ECF_DIR`；
- 使用 `target_sources()` 显式启用需要的 YXN_ECF 源文件；
- 使用 `target_include_directories()` 显式添加对应头文件目录；
- 通过 `add_subdirectory(Usercode)` 接入本工程代码。

当前默认接入的 YXN_ECF 内容包括：

- FeedForward、LowPassFilter、MahonyAHRS、MyMath、PID、SlopePlaning；
- STM32F4 bxCAN BSP 和 bxCAN Adapter；
- DWT、Encoder；
- Key、Relay、DJI Motor。

SPI、I2C、USART、USB、PWM、BMI088、WS2812 等模块需要先在 CubeMX 中生成
对应外设和 HAL 支持，再在顶层 CMake 中启用。

### Usercode/CMakeLists.txt

`Usercode/CMakeLists.txt` 只管理 Handsome_Car 自己的 App 和 Task，不管理
YXN_ECF 公共代码。新增 `.c/.cpp` 后，需要把相对路径显式加入：

```cmake
set(USER_APP_SOURCES
    # Application/App_Car/App_Car.c
)

set(USER_TASK_SOURCES
    Handsome_Car_Task/Handsome_Car_Task.c
)
```

工程不使用 `GLOB` 自动扫描源码。只有明确写入 CMake 的 `.c/.cpp` 才会参与
编译，头文件不需要写进 `target_sources()`。

## YXN_ECF 使用方式

`YXN_ECF` 存放工程使用的通用算法、BSP 和功能模块：

- 需要的 `.c/.cpp` 在顶层 `CMakeLists.txt` 中显式加入；
- 对应头文件目录加入 `target_include_directories()`；
- 新模块接入前先检查 MCU、外设和 HAL 依赖。

当前 MCU 是 STM32F407，应使用：

```text
YXN_ECF/bsp/CAN/bxCAN/
```

不要直接启用 STM32H7 使用的 `YXN_ECF/bsp/CAN/FDCAN/`。

完整说明见 [YXN_ECF 使用指南](YXN_ECF/使用指南.md)。

## FreeRTOS 任务入口

CubeMX 在 `Core/Src/freertos.c` 中生成以下 weak 函数：

```c
__weak void InitTaskFunction(void *argument);
__weak void MainTaskFunction(void *argument);
```

`Usercode/Handsome_Car_Task/Handsome_Car_Task.c` 提供完全同名的强定义：

- `InitTaskFunction`：放置初始化流程，完成后退出初始化任务；
- `MainTaskFunction`：放置主要周期更新逻辑，当前循环周期为约 1 ms。

这样不需要修改 CubeMX 生成的 `freertos.c`，重新生成 CubeMX 代码后任务实现
仍保留在 `Usercode` 中。

更多 Usercode 添加方法见 [Usercode 使用说明](Usercode/README.md)。

## 构建工程

### STM32Cube VS Code 环境

推荐使用工程当前配置的 STM32Cube CMake、GNU Arm Toolchain 和 Ninja，执行：

1. CMake Configure；
2. 选择 `Debug` 或 `Release` preset；
3. CMake Build。

### 命令行

在已经加载 STM32Cube 工具环境的终端中执行：

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug --parallel 4
```

Release 构建：

```powershell
cube-cmake --preset Release
cube-cmake --build --preset Release --parallel 4
```

不要在同一个 `build` 缓存中混用不同版本的 CMake。构建产物位于：

```text
build/Debug/Handsome_Car.elf
build/Debug/Handsome_Car.map
```

`build/` 已由 `.gitignore` 忽略。

## 添加新功能

### 添加 Handsome_Car 专用代码

1. 在 `Usercode/Application` 或新的 Task 文件夹中创建 `.c/.cpp` 和 `.h`；
2. 把源文件加入 `Usercode/CMakeLists.txt`；
3. 在 `Handsome_Car_Task.c` 中调用初始化或周期更新接口；
4. 重新执行 CMake Configure 和 Build。

### 添加 YXN_ECF 公共模块

1. 把可复用代码放入 `YXN_ECF/algorithm`、`bsp` 或 `module`；
2. 检查模块与 STM32F407 及当前 CubeMX 外设是否兼容；
3. 在顶层 `CMakeLists.txt` 的 `target_sources()` 中加入实现文件；
4. 在 `target_include_directories()` 中加入头文件目录；
5. 重新配置、编译和链接。

## CubeMX 重新生成注意事项

- 可以通过 `Handsome_Car.ioc` 修改外设和 FreeRTOS 配置；
- 不要手工修改 `cmake/stm32cubemx/CMakeLists.txt`，它可能被重新生成；
- 工程自有接入保持在顶层 `CMakeLists.txt` 和 `Usercode/CMakeLists.txt`；
- 重新生成后检查任务函数名称是否仍为 `InitTaskFunction` 和
  `MainTaskFunction`；
- 新增外设后确认对应 HAL 驱动、头文件、句柄和中断均已生成。

## 验证边界

构建成功只表示源码通过编译并生成 ELF，不代表以下环节已经验证：

- 固件已经烧录到目标板；
- CAN、SPI、I2C、UART 等物理总线正常；
- 电机和传感器供电、接线与 ID 正确；
- 实际控制参数和运行时序满足硬件要求。

涉及硬件功能时，还需要完成下载、在线调试和实机测试。
