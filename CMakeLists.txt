cmake_minimum_required(VERSION 3.21)
project(fe_hotreload)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_GENERATOR_PLATFORM x86)

add_subdirectory(patcher)

add_library(fe_hotreload SHARED
        src/dllmain.cpp
        src/common.cpp src/common.h
        src/alloc.cpp src/alloc.h
        src/init.cpp
        src/fe_override.cpp src/fe_override.h
        src/hotkey.cpp
        )

set_target_properties(fe_hotreload PROPERTIES SUFFIX ".asi")

target_link_libraries(fe_hotreload PUBLIC patcher)
