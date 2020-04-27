-- Stroke-to-fill conversion program and test harness
-- Copyright (C) 2020 Diego Nehab
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as published
-- by the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- Contact information: diego.nehab@gmail.com
--
local _M = { }

local filter = require"filter"

local function tod(v)
    return string.format("%.8f", v)
end

local function make_input_path_f_to_json_path(out)
    local filter = {}
    local first = true
    local function comma()
        if not first then
            out:write(',')
        else
            first = false
        end
    end
    function filter:begin_contour(x0, y0)
        comma()
        out:write('["M",', tod(x0), ',', tod(y0), ']')
    end
    function filter:end_closed_contour(x0, y0)
        comma()
        out:write('["Z"]')
    end
    function filter:end_open_contour(x0, y0)
    end
    function filter:degenerate_segment(x0, y0, dx, dy, x1, y1)
        comma()
        out:write('["L",', tod(x1), ',', tod(y1), ']')
    end
    function filter:linear_segment(x0, y0, x1, y1)
        comma()
        out:write('["L",', tod(x1), ',', tod(y1), ']')
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        comma()
        out:write('["Q",', tod(x1), ',', tod(y1), ',', tod(x2), ',', tod(y2), ']')
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        comma()
        out:write('["C",', tod(x1), ',', tod(y1), ',', tod(x2), ',', tod(y2), ',', tod(x3), ',', tod(y3), ']')
    end
    return filter
end

local function stroke(script, input_shape, screen_xf, width, style)
    local input_path_data = input_shape:as_path_data(input_shape:get_xf():
        transformed(screen_xf))
    local inname = os.tmpname()
--io.stderr:write(inname, "\n")
    local infile = assert(io.open(inname, "w"))
    infile:write('{"width":', tod(width), ',')
    infile:write('"cap":"', tostring(style:get_initial_cap()), '",')
    infile:write('"join":"', tostring(style:get_join()), '",')
    infile:write('"miterlimit":', tod(style:get_miter_limit()), ',')
    infile:write('"dashoffset":', tod(style:get_dash_offset()), ',')
    infile:write('"resetsonmove":', tostring(style:get_resets_on_move()), ',')
    infile:write('"dasharray":[')
    local dashes = style:get_dashes()
    for i, d in ipairs(dashes) do
        infile:write(tod(d))
        if dashes[i+1] then
            infile:write(",")
        end
    end
    infile:write("],")
    infile:write('"path":[')
    input_path_data:iterate(
        filter.make_input_path_f_xform(input_shape:get_xf(),
            filter.make_input_path_f_rational_quadratic_to_cubics(
                make_input_path_f_to_json_path(infile))))
    infile:write("]}\n")
    local java = assert(io.popen(string.format(script, inname), "r"))
    local outpath = java:read("*a")
--io.stderr:write(outpath, "\n")
    java:close()
    os.remove(inname)
    return path(outpath)
end

function _M.openjdk8(input_shape, screen_xf, width, style)
    return stroke("strokers/java/openjdk8.sh < %s",
        input_shape, screen_xf, width, style)
end

function _M.openjdk11(input_shape, screen_xf, width, style)
    return stroke("strokers/java/openjdk11.sh < %s",
        input_shape, screen_xf, width, style)
end

function _M.oraclejdk8(input_shape, screen_xf, width, style)
    return stroke("strokers/java/oraclejdk8.sh < %s",
        input_shape, screen_xf, width, style)
end

return _M
