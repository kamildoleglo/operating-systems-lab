cmake_minimum_required(VERSION 3.10)
project(zest6 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest6 server.c posix/server.c posix/header.h posix/client.c)