cmake_minimum_required(VERSION 3.10)
project(test_simple_module)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

add_executable(test_simple_module.arm64 test_application.c)
