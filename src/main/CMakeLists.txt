cmake_minimum_required(VERSION 3.13)

project(8008-assembler-exe
        DESCRIPTION "The assembler executable for the Intel 8008 processor assembler."
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(ASSEMBLER_EXE_NAME "as-8008")
set(ASSEMBLER_EXE_FILES src/main.cpp)

add_executable(${ASSEMBLER_EXE_NAME} ${ASSEMBLER_EXE_FILES})

target_link_libraries(${ASSEMBLER_EXE_NAME} ${ASSEMBLER_LIB_NAME})
target_include_directories(${ASSEMBLER_EXE_NAME} PRIVATE ../)

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${ASSEMBLER_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO activated for ${ASSEMBLER_EXE_NAME}")
endif()

