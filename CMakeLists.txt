cmake_minimum_required(VERSION 3.13)

project(8008_Assembler
        VERSION 0.1
        DESCRIPTION "Assembler for the Intel 8008 processor."
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Prepare Extern
if(WITH_TESTS)
    find_package(Git QUIET)

    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        # Update submodules as needed
        option(GIT_SUBMODULE "Check submodules during build" ON)

        if(GIT_SUBMODULE)
            message(STATUS "Submodule update")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
            endif()
        endif()
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/googletest/CMakeLists.txt")
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()

    include(CTest)
endif()

# Google Test
if(WITH_TESTS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(extern/googletest)
endif()

# Prepare for LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_result OUTPUT output)

if(NOT LTO_result)
    message(WARNING "IPO is not supported: ${output}")
endif()

# Project
set(ASSEMBLER_LIB_NAME "assembler-lib")

add_subdirectory(src/assembler)
add_subdirectory(src/main)

