cmake_minimum_required(VERSION 3.17)
project(ATM_Concurency_IPC C)

set(CMAKE_C_STANDARD 99)

add_executable(ATM_Concurency_IPC main.c DBserver.c DBserver.h extra_file.h)