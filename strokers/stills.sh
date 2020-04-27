#!/bin/bash

stroker=$1
shift

p=$(command nproc) || p=$(command gnproc) || p=4

mkdir -p fun/stills/$stroker/bare
mkdir -p fun/stills/$stroker/outline
mkdir -p fun/stills/$stroker/compare

task() {
	local test=$1
	shift
	echo $test
    local bare="fun/stills/$stroker/bare/${test}_${stroker}_bare"
    local outline="fun/stills/$stroker/outline/${test}_${stroker}_outline"
    local compare="fun/stills/$stroker/compare/${test}_${stroker}_compare.png"
    local distroke="a/distroke/${test}_distroke.png"
    luapp test-strokers.lua -test:$test -driver:skia -stroker:$stroker $* > ${bare}.png
    luapp test-strokers.lua -test:$test -driver:svg -stroker:$stroker $* > ${bare}.svg
    luapp test-strokers.lua -test:$test -driver:stats -no-background -stroker:$stroker $* | grep segment > ${bare}.stats
    luapp test-strokers.lua -test:$test -driver:skia -stroker:$stroker -outline -generatrix$* > ${outline}.png
    luapp test-strokers.lua -test:$test -driver:svg -stroker:$stroker -outline -generatrix$* > ${outline}.svg
    compare -fuzz 20% $distroke ${bare}.png $compare
}

t=1;
for test in $(luapp test-strokers.lua -test:list | sort); do
	((t=t+1));
	(((t-1)%p==0)) && wait
	task "$test" $* &
done
wait
