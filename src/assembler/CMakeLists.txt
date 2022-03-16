cmake_minimum_required(VERSION 3.13)

project(8008-assembler-library
        DESCRIPTION "A library to create an assembler for the Intel 8008 processor."
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(ASSEMBLER_TEST_NAME ${ASSEMBLER_LIB_NAME}_tests)

set(ASSEMBLER_LIB_FILES
        src/options.cpp src/options.h
        src/files.cpp src/files.h
        src/symbol_table.cpp src/symbol_table.h
        src/line_tokenizer.cpp src/line_tokenizer.h
        src/utils.cpp src/utils.h
        src/evaluator.cpp src/evaluator.h
        src/byte_writer.cpp src/byte_writer.h
        src/data_extraction.cpp src/data_extraction.h
        src/first_pass.cpp src/first_pass.h
        src/second_pass.cpp src/second_pass.h
        src/errors.cpp src/errors.h
        src/listing.cpp src/listing.h
        src/listing_line.cpp src/listing_line.h
        src/parsed_line.cpp src/parsed_line.h
        src/instruction.cpp src/instruction.h
        src/opcodes/opcodes.cpp src/opcodes/opcodes.h
        src/opcodes/opcode_action.cpp src/opcodes/opcode_action.h
        src/opcodes/opcode_action_noarg.cpp src/opcodes/opcode_action_noarg.h
        src/opcodes/opcode_action_onebyte_arg.cpp src/opcodes/opcode_action_onebyte_arg.h
        src/opcodes/opcode_action_twobyte_arg.cpp src/opcodes/opcode_action_twobyte_arg.h
        src/opcodes/opcode_action_inpout.cpp src/opcodes/opcode_action_inpout.h
        src/opcodes/opcode_action_rst.cpp src/opcodes/opcode_action_rst.h
        src/context.h
        src/listing_pass.cpp src/listing_pass.h
        src/file_reader.cpp src/file_reader.h)

set(ASSEMBLER_TEST_FILES
        tests/evaluator_tests.cpp
        tests/utils_tests.cpp
        tests/byte_writer_tests.cpp
        tests/data_extractions_tests.cpp
        tests/opcodes_tests.cpp
        tests/listing_line_tests.cpp
        tests/line_tokenizer_tests.cpp
        tests/instruction_tests.cpp
        tests/opcode_action_tests.cpp
        tests/file_reader_tests.cpp)

add_library(${ASSEMBLER_LIB_NAME} ${ASSEMBLER_LIB_FILES})
target_include_directories(${ASSEMBLER_LIB_NAME} PUBLIC src/)

if(WITH_TESTS)
    add_executable(${ASSEMBLER_TEST_NAME} ${ASSEMBLER_TEST_FILES})
    set_target_properties(${ASSEMBLER_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${ASSEMBLER_TEST_NAME}
            PRIVATE gtest gtest_main gmock
            PUBLIC ${ASSEMBLER_LIB_NAME})
    target_include_directories(${ASSEMBLER_TEST_NAME} PRIVATE src/)
    target_include_directories(${ASSEMBLER_TEST_NAME} PRIVATE ../)

    add_test(NAME ${ASSEMBLER_TEST_NAME} COMMAND ${ASSEMBLER_TEST_NAME})
endif()
