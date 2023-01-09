cmake_minimum_required(VERSION 3.10)
project(A4)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(a4w22 main.cpp resource_manager.cpp utils.cpp simulator.cpp simulator.h)
target_link_libraries(a4w22 Threads::Threads)
