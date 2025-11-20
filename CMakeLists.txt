cmake_minimum_required(VERSION 3.10)
project(UTP)

set(CMAKE_CXX_STANDARD 20)

add_executable(UTP main.cpp
        Student.cpp
        Student.h
)
