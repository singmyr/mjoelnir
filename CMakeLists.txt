cmake_minimum_required(VERSION 3.29)

project(MJOELNIR)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall -Werror -Wpedantic)
add_executable(main src/main.c)
