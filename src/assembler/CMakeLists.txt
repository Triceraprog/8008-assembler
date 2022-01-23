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
        src/opcodes.cpp src/opcodes.h
        src/evaluator.cpp src/evaluator.h)

set(ASSEMBLER_TEST_FILES
        tests/reference_tests.cpp
        tests/evaluator_tests.cpp tests/utils_tests.cpp)

add_library(${ASSEMBLER_LIB_NAME} ${ASSEMBLER_LIB_FILES})

if(WITH_TESTS)
    add_executable(${ASSEMBLER_TEST_NAME} ${ASSEMBLER_TEST_FILES})
    set_target_properties(${ASSEMBLER_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${ASSEMBLER_TEST_NAME}
            PRIVATE gtest gtest_main gmock
            PUBLIC ${ASSEMBLER_LIB_NAME})
    target_include_directories(${ASSEMBLER_TEST_NAME} PRIVATE src/)
    target_include_directories(${ASSEMBLER_TEST_NAME} PRIVATE ../)
    #if(${LINK_STATIC_EXE})
    #    target_link_options(${ASSEMBLER_TEST_NAME} PRIVATE -static-libgcc -static-libstdc++)
    #endif()

    add_test(NAME ${ASSEMBLER_TEST_NAME} COMMAND ${ASSEMBLER_TEST_NAME})
endif()