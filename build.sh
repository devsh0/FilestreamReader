#!/bin/bash

cmake . -B ./build
cd build && make
mkdir lib
mv *.a lib && cp ../*.h lib
