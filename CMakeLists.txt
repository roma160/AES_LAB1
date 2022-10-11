cmake_minimum_required(VERSION 3.15)
project(LAB1_AES VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(main LAB1.cpp)