cmake_minimum_required(VERSION 3.19)
project(UCIEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(UCIEngine main.cpp)

add_subdirectory(Engine)

target_link_libraries(UCIEngine Engine)