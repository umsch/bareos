#!/bin/bash

cmake -DUSE_SYSTEM_FMT=ON -S /source -B /build -G Ninja
cmake --build /build

cd /build/systemtests/tests/grpc


#runtime.sh
#bin/bareos start
#PID=$(pidof bareos_dir-grpc)
#kill -9 $PID
#




## verhindern, dass testergebnisse gelöscht werden
# rm test-cleanup

runtime.sh
./test-setup
./testrunner-create-backup
./testrunner-restore

# ctest -R grpc

PID=$(pidof bareos_dir-grpc)
kill -9 $PID

sbin/bareos_dir-grpc -f -d 1000 -c etc/bareos/

while kill -0 $PID 2> /dev/null; do
  sleep 1
done
