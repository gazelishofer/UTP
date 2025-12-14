cmake_minimum_required(VERSION 3.10)
project(UTP)

set(CMAKE_CXX_STANDARD 20)

add_executable(UTP main.cpp
        Student.cpp
        Student.h
)

# Copy data files to build directory
file(COPY forStudents.txt forStudents.bin DESTINATION ${CMAKE_BINARY_DIR})
