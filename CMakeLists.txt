cmake_minimum_required(VERSION 3.15)
project(File_System C)

set(CMAKE_C_STANDARD 11)

add_executable(File_System scratch.c)
add_executable(Disk_intialization init.c)
add_executable(Disk_monitor disk_monitor.c)