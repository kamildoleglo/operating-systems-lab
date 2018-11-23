cmake_minimum_required(VERSION 3.10)
project(zest7 C)

set(CMAKE_C_STANDARD 99)

add_executable(zest7 main.c second.c zad2/common.h zad2/barber.c zad2/clients.c)