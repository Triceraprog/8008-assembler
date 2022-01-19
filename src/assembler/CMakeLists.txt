cmake_minimum_required(VERSION 3.13)

project(8008-assembler-library
        DESCRIPTION "A library to create an assembler for the Intel 8008 processor."
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(ASSEMBLER_TEST_NAME ${ASSEMBLER_LIB_NAME}_tests)

set(ASSEMBLER_LIB_FILES
        src/reference.cpp
        src/options.cpp src/options.h
        src/files.cpp src/files.h
        src/symbol_table.cpp src/symbol_table.h
        src/line_tokenizer.cpp src/line_tokenizer.h
        src/utils.cpp src/utils.h
        src/opcodes.cpp src/opcodes.h)

add_library(${ASSEMBLER_LIB_NAME} ${ASSEMBLER_LIB_FILES})
