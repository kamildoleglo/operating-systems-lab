cmake_minimum_required(VERSION 3.10)
project(zest8 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest8 zad1/main.c zad1/application_helper.h)
target_link_libraries(zest8 m pthread rt)
