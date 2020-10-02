#!/bin/sh

luapp test-strokers.lua -test:wedge -stroker:rvg -driver:svg -generatrix -outline > fig-17c.svg
luapp test-strokers.lua -test:cusp -stroker:rvg -driver:svg -generatrix -outline > fig-17f.svg
luapp test-strokers.lua -test:cubic_loop_without_hole -stroker:rvg -driver:svg -generatrix -outline > fig-17h.svg
luapp test-strokers.lua -test:centurion_head -stroker:rvg -driver:svg -generatrix -outline > fig-17j.svg
luapp test-strokers.lua -test:hole -stroker:rvg -driver:svg -generatrix -outline > fig-17l.svg
luapp test-strokers.lua -test:zigzag -stroker:rvg -driver:svg -generatrix -outline > fig-17n.svg
