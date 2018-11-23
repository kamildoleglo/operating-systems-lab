cmake_minimum_required(VERSION 3.10)
project(zest10 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest10 zad1/server.c zad1/server.h zad1/client.c)
target_link_libraries(zest10 -lpthread -lrt)