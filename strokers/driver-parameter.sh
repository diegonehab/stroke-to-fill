#!/bin/bash

set -e

stroker_test=$1
shift
driver=$1
shift
ext=$1
shift
n=300


p=$(command nproc) || p=$(command gnproc) || p=4

mkdir -p fun/$driver/$stroker_test

task() {
    local t=$1
    shift
    local output=$(lua -e "print(string.format('%04d.$ext', $t))")
    local parameter=$(lua -e "print($t/$n)")
    ./test-strokers.lua -stroker:native -test:$stroker_test -parameter:$parameter -driver:$driver -idempotent $*  > fun/$driver/$stroker_test/bare-$output;
    #./test-strokers.lua -stroker:native -test:$stroker_test -parameter:$parameter -driver:$driver -idempotent -generatrix $* >  fun/$driver/$stroker_test/outline-$output;
}

for t in $(seq 1 $n); do
    (((t-1)%p==0)) && echo batch $t && wait
    task "$t" $* &
done
wait
