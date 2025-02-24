#!/bin/sh

set -e

mkdir -p build
cd build
emcmake cmake ..
make -j4