cmake_minimum_required(VERSION 3.17)
project(FilestreamReader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wpedantic -O3")

add_library(libfstream FilestreamReader.cpp)
add_executable(fstream FilestreamReader.cpp TestFilestreamReader.cpp)
