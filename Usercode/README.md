# Usercode 使用说明

`Usercode` 只存放 Handsome_Car 自己的应用代码和 FreeRTOS 任务代码。公共的
YXN_ECF 源码位于工程根目录 `YXN_ECF/`，并由顶层 `CMakeLists.txt` 接入。

## 当前结构

```text
Usercode/
├── Handsome_Car_Task/
│   ├── Handsome_Car_Task.c
│   └── Handsome_Car_Task.h
├── CMakeLists.txt
└── README.md
```

## 添加应用文件

例如创建：

```text
Usercode/Application/App_Car/App_Car.c
Usercode/Application/App_Car/App_Car.h
```

然后在 `Usercode/CMakeLists.txt` 的 `USER_APP_SOURCES` 中加入：

```cmake
set(USER_APP_SOURCES
    Application/App_Car/App_Car.c
)
```

## 添加任务文件

任务源文件写入 `USER_TASK_SOURCES`：

```cmake
set(USER_TASK_SOURCES
    Handsome_Car_Task/Handsome_Car_Task.c
    Another_Task/Another_Task.c
)
```

这里只显式列出 `.c/.cpp` 源文件，不需要把 `.h` 写进 `target_sources()`。
工程不使用 `GLOB` 自动收集，新文件只有加入 CMake 列表后才会参与编译。

## FreeRTOS weak 任务覆盖

当前 CubeMX 在 `Core/Src/freertos.c` 中生成了：

```c
__weak void InitTaskFunction(void *argument);
__weak void MainTaskFunction(void *argument);
```

`Handsome_Car_Task.c` 提供完全同名的强定义。以后如果在 CubeMX 中修改任务
名称，需要同步修改这里的函数声明和实现。
