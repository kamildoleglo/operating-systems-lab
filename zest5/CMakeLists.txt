cmake_minimum_required(VERSION 3.10)
project(zest5 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest5 zad1/main.c zad1/main2.c zad2/master.c zad2/slave.c)