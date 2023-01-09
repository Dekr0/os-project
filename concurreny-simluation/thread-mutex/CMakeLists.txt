cmake_minimum_required(VERSION 3.10)
project(A4)

set(CMAKE_CXX_STANDARD 11)

add_executable(a4w22 main.cpp resource_manager.cpp utils.cpp simulator.cpp simulator.h monitor.cpp monitor.h)
