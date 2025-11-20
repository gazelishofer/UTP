cmake_minimum_required(VERSION 4.0)
project(UTP)

set(CMAKE_CXX_STANDARD 20)

add_executable(UTP main.cpp
        Student.cpp
        Student.h
        lab2vp.cpp
        lab2vp.h
)
