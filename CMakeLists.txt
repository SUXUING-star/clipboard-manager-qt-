cmake_minimum_required(VERSION 3.15)
project(clipboard_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 只引入需要的 Qt 模块
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
)

# 编译优化：使用 Release 模式，启用 O3 优化并剔除调试信息
set(CMAKE_BUILD_TYPE Release)
# 根据不同的编译器设置优化标志
if(MSVC)
    # MSVC 编译器优化选项
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")
else()
    # GCC/Clang 编译器优化选项
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -s")
endif()


# 添加可执行文件
add_executable(clipboard_manager
    main/main.cpp
    main/mainwindow.cpp
    main/mainwindow.h
    resources.qrc
    components/animationmanager.h
    components/animationmanager.cpp
    components/autostartmanager.h
    components/autostartmanager.cpp
    components/downloadmanager.cpp
    components/downloadmanager.h
    components/storagemanager.h
    components/storagemanager.cpp
    components/ui/customdialog.h
    components/ui/customdialog.cpp
)

# 仅链接所需的 Qt 库
target_link_libraries(clipboard_manager PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# 如果是 Windows 系统，设置为窗口应用
if(WIN32)
    set_target_properties(clipboard_manager PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
