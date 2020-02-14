cmake_minimum_required(VERSION 3.15)
project(csv_parser)

set(CMAKE_CXX_STANDARD 17)

add_library(csv_parser INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include/)

add_executable(test main.cpp)
target_link_libraries(test csv_parser)