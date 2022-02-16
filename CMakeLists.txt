cmake_minimum_required(VERSION 3.21)
project(fe_hotreload)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_GENERATOR_PLATFORM x86)

add_subdirectory(patcher)

add_library(fe_hotreload SHARED
        dllmain.cpp
        common.cpp common.h
        fe_override.cpp fe_override.h)

target_link_libraries(fe_hotreload PUBLIC patcher)
