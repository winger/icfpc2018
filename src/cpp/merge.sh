#!/usr/bin/env bash

git pull
./build/cpp_solver -mode merge $*
