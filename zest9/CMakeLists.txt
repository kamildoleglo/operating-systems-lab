cmake_minimum_required(VERSION 3.10)
project(zest9 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest9 zad1/main.c)
target_link_libraries(zest9 pthread rt)
