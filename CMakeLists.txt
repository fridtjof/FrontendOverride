cmake_minimum_required(VERSION 3.21)
project(fe_hotreload)

set(CMAKE_CXX_STANDARD 98)

add_subdirectory(patcher)

add_library(fe_hotreload SHARED dllmain.cpp)

target_link_libraries(fe_hotreload PUBLIC patcher)