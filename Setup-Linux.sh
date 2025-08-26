#!/bin/bash

git submodule update --init --recursive
cmake -S . -B Build
cmake --build Build