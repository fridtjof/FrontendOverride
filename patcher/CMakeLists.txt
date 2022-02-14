add_library(patcher
        source/patcher.cpp include/patcher/patcher.h
        )

target_compile_definitions(patcher PRIVATE IS_LIBRARY_BUILD)

target_include_directories(patcher PUBLIC include/)