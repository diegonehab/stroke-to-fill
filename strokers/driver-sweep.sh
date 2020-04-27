#!/bin/bash

set -e

stroker_test=$1
shift
driver=$1
shift
ext=$1
shift
length=$(./test-strokers.lua -test:$stroker_test -arc-length $*)
n=300

p=$(command nproc) || p=$(command gnproc) || p=4

mkdir -p fun/$driver/$stroker_test

task() {
    local t=$1
    shift
    local output=$(lua -e "print(string.format('%04d.$ext', $t))")
    local dashes=$(lua -e "print($t/$n * $length)"),$(lua -e "print(2*$length)")
    ./test-strokers.lua -stroker:native -test:$stroker_test -dashes:$dashes -driver:$driver -idempotent $* > fun/$driver/$stroker_test/bare-$output;
#    ./test-strokers.lua -stroker:native -test:$stroker_test -dashes:$dashes -driver:$driver -idempotent -generatrix $* > fun/$driver/outline-$output;
}

for t in $(seq 1 $n); do
    (((t-1)%p==0)) && echo batch $t && wait
    task "$t" $* &
done
wait
