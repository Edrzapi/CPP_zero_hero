# ============================================================
# C++ Zero to Hero - Top-Level Build
# ============================================================
# Builds every example, task and test in the repository.
#
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build --output-on-failure
#
# Every example is its own executable named <folder>_<file>,
# for example basics_02_types_and_sizes. Executables land in
# build/bin (build/bin/Debug with Visual Studio generators).
# ============================================================

cmake_minimum_required(VERSION 3.20)

project(CPP_zero_hero
    VERSION 1.0
    DESCRIPTION "C++ Zero to Hero teaching repository"
    LANGUAGES C CXX)

# ------------------------------------------------------------
# Language Standard
# ------------------------------------------------------------
# C++20 everywhere. Targets that need C++23 raise their own
# standard and are only built when CPPZH_ENABLE_CXX23 is ON.
# Extensions OFF means -std=c++20 rather than -std=gnu++20, so
# code that only works on one compiler fails early.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Writes compile_commands.json for clangd, clang-tidy and VS Code.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# One output folder for every executable.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ------------------------------------------------------------
# Options
# ------------------------------------------------------------

option(CPPZH_WARNINGS_AS_ERRORS "Treat compiler warnings as errors (-Werror or /WX)" OFF)
option(CPPZH_ENABLE_CXX23       "Build the examples that need C++23 library features" OFF)
option(CPPZH_BUILD_UB_DEMOS     "Build the deliberate undefined behaviour demos" OFF)
option(CPPZH_SANITIZE           "Build with AddressSanitizer and UBSan (GCC and Clang)" OFF)

include(cmake/zero_hero.cmake)

# MSVC's AddressSanitizer cannot be combined with the /RTC1
# run-time checks that CMake adds to Debug builds by default.
if(MSVC AND CPPZH_SANITIZE)
    foreach(lang C CXX)
        string(REGEX REPLACE "/RTC[1csu]*" "" CMAKE_${lang}_FLAGS_DEBUG "${CMAKE_${lang}_FLAGS_DEBUG}")
    endforeach()
endif()

# ctest runs every example and fails on a non-zero exit code.
enable_testing()

# ------------------------------------------------------------
# Entry Point
# ------------------------------------------------------------
# Mirrors main.py in Python Zero to Hero: prints the syllabus.

add_executable(zero_hero main.cpp)
zh_apply_settings(zero_hero)

# ------------------------------------------------------------
# Topics (in syllabus order)
# ------------------------------------------------------------

add_subdirectory(basics)
add_subdirectory(control_flow)
add_subdirectory(iteration)
add_subdirectory(functions)
add_subdirectory(compilation)
add_subdirectory(c_interop)
add_subdirectory(memory)
add_subdirectory(oop)
add_subdirectory(collections)
add_subdirectory(templates)
add_subdirectory(exceptions)
add_subdirectory(io)
add_subdirectory(datetime)
add_subdirectory(concurrency)
add_subdirectory(testing)
add_subdirectory(under_the_hood)
