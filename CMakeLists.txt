cmake_minimum_required(VERSION 3.13)

project(8008_Assembler
        VERSION 0.1
        DESCRIPTION "Assembler for the Intel 8008 processor."
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(ASSEMBLER_LIB_NAME "assembler-lib")

add_subdirectory(src/assembler)
add_subdirectory(src/main)
