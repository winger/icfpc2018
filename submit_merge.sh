#!/usr/bin/env bash
set -e
git pull
cd src/cpp && make && ./build/cpp_solver -mode merge
