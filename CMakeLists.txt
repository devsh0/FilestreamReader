cmake_minimum_required(VERSION 3.17)
project(FilestreamReader)

set(CMAKE_CXX_STANDARD 20)

add_library(libfstream FilestreamReader.cpp)
