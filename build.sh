#!/bin/bash

cmake . -B ./build
cd build && make
rm -rf lib && mkdir lib
mv *.a lib && cp ../*.h lib
