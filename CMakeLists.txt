cmake_minimum_required(VERSION 3.31)
project(jaml C)

set(CMAKE_C_STANDARD 11)

add_executable(jaml main.c
        vector2.h
        viewer_win32.c
)
