#!/bin/bash

git submodule update --init --recursive

if [ ! -d "build-msys" ]; then 
mkdir build-msys
fi

pushd build-msys
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
popd
