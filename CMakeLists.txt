# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(clipboard_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
   Core
   Gui
   Widgets
)

add_executable(clipboard_manager
   main.cpp
   mainwindow.cpp
   mainwindow.h
   resources.qrc
   animationmanager.h
   animationmanager.cpp
   autostartmanager.h autostartmanager.cpp
)

target_link_libraries(clipboard_manager PRIVATE
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
)

if(WIN32)
   set_target_properties(clipboard_manager PROPERTIES
       WIN32_EXECUTABLE TRUE
   )
endif()
