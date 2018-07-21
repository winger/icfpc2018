#!/usr/bin/env bash
./build/cpp_solver -threads 1 &
prlimit --rss=1000000 --pid $!
wait
