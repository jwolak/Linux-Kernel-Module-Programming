cmake_minimum_required(VERSION 3.10)
project(test_driver_class_create_read_write)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

add_executable(test_driver_class_create_read_write.arm64 test_application.c)
