cmake_minimum_required(VERSION 3.7)
project(BTree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp BTree.h BTree.cpp)
add_executable(BTree ${SOURCE_FILES})