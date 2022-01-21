#!/bin/bash

git submodule update --init --recursive

eval $(./externals/nxdk/bin/activate -s)

make -j$((`nproc`+1))
