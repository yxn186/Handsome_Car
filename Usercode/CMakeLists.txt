# ============================================================
# Handsome_Car project-owned application and task sources
# ============================================================
#
# This file manages only code written specifically for Handsome_Car.
# YXN_ECF is configured in the project's top-level CMakeLists.txt.
#
# No file(GLOB ...) is used. After creating a new .c/.cpp file, add its path
# explicitly to USER_APP_SOURCES or USER_TASK_SOURCES below.

# Application source files
set(USER_APP_SOURCES
    Application/App_Chassis/App_Chassis.cpp
    Application/App_Remote/App_Remote.cpp
    Application/App_Vision/App_Vision.cpp
)

# FreeRTOS task source files
set(USER_TASK_SOURCES
    Handsome_Car_Task/Handsome_Car_Task.cpp
)

# Combine application and task sources into one list.
set(USERCODE_SOURCES
    ${USER_APP_SOURCES}
    ${USER_TASK_SOURCES}
)

# Add Usercode sources directly to the final ELF target.
# Direct integration ensures that InitTaskFunction, MainTaskFunction and
# USBTaskFunction in Handsome_Car_Task.cpp override CubeMX's weak implementations.
if(USERCODE_SOURCES)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${USERCODE_SOURCES}
    )
endif()

# Make the Usercode root visible to the compiler. This supports includes such
# as: #include "Handsome_Car_Task/Handsome_Car_Task.h"
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
