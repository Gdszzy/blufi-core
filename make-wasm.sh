#!/bin/sh

set -e

emcmake cmake -S . -B build-wasm
cmake --build build-wasm --target wasm --parallel 8