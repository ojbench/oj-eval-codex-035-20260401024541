cmake_minimum_required(VERSION 3.10)
project(problem_035 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(code
    src/main.cpp
    src/MyString.hpp
)

target_compile_options(code PRIVATE -O2 -pipe -s)
