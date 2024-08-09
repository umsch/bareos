#!/bin/bash

cmake -DUSE_SYSTEM_FMT=ON -S /source -B /build -G Ninja
cmake --build /build

cd /build/systemtests/tests/grpc
runtime.sh
./test-setup

PID=$(pidof bareos_dir-grpc)

while kill -0 $PID 2> /dev/null; do
  sleep 1
done