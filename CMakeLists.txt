cmake_minimum_required(VERSION 3.10)

project(BinEdit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(exe bin_editor.cpp)
