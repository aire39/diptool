#!/bin/bash

git submodule update --init --recursive

if [ ! -d "build-msys" ]; then 
mkdir build-msys
fi

pushd build-msys
cmake -G "Unix Makefiles" -DCMAKE_POLICY_VERSION_MINIMUM=3.11 -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
popd
