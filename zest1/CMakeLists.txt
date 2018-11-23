cmake_minimum_required(VERSION 3.9)
project(zest1 C)

set(CMAKE_C_STANDARD 99)


set(SOURCE_FILES
        main.c
        library.c
        library.h
        )

#add_library(zest1 library.c library.h main.c)
add_executable(zest1 ${SOURCE_FILES})
