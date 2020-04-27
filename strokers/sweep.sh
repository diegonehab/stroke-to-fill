#!/bin/bash

set -e

stroker_test=$1
shift
stroker=$1
shift
length=$(./test-strokers.lua -test:$stroker_test -arc-length $*)
n=300

p=$(command nproc) || p=$(command gnproc) || p=4

task() {
    local t=$1
    shift
    local output=$(lua -e "print(string.format('%04d.png', $t))")
    local dashes=$(lua -e "print($t/$n * $length)"),$(lua -e "print(2*$length)")
    ./test-strokers.lua -stroker:$stroker -test:$stroker_test -dashes:$dashes -driver:skia $* > fun/png/bare-$output;
    ./test-strokers.lua -stroker:$stroker -test:$stroker_test -dashes:$dashes -driver:cairo -outline -generatrix $* > fun/png/outline-$output;
    local distroke=distroke_${stroker_test}_${output}
    if [ ! -f fun/png/$distroke ]; then
        echo $distroke
        echo ./test-strokers.lua -stroker:native -test:$stroker_test -dashes:$dashes -driver:distroke fun/png/$distroke;
        ./test-strokers.lua -stroker:native -test:$stroker_test -dashes:$dashes -driver:distroke > fun/png/$distroke;
    fi
    compare -fuzz 20% fun/png/$distroke fun/png/bare-$output fun/png/compare-$output
}

for t in $(seq 1 $n); do
    (((t-1)%p==0)) && echo batch $t && wait
    task "$t" $* &
done
wait

ffmpeg -y -i fun/png/bare-%04d.png -c:v libx264 -pix_fmt yuv420p -preset:v slow -profile:v baseline -crf 20 -vf 'crop=trunc(iw/2)*2:trunc(ih/2)*2' fun/${stroker_test}_sweep_${stroker}_bare.m4v

ffmpeg -y -i fun/png/compare-%04d.png -c:v libx264 -pix_fmt yuv420p -preset:v slow -profile:v baseline -crf 20 -vf 'crop=trunc(iw/2)*2:trunc(ih/2)*2' fun/${stroker_test}_sweep_${stroker}_compare.m4v

ffmpeg -y -i fun/png/outline-%04d.png -c:v libx264 -pix_fmt yuv420p -preset:v slow -profile:v baseline -crf 20 -vf 'crop=trunc(iw/2)*2:trunc(ih/2)*2' fun/${stroker_test}_sweep_${stroker}_outline.m4v


rm fun/png/bare-0*.png fun/png/compare-0*png fun/png/outline-0*png
