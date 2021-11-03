cmake_minimum_required(VERSION 3.16)
project(FilestreamReader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wpedantic -O3")

add_library(fstream FilestreamReader.cpp)
add_executable(test FilestreamReader.cpp TestFilestreamReader.cpp)
