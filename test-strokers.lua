#!/usr/local/bin/luapp
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

local strokers = require"strokers"

local filterx = require"filterx"
local filter = require"filter"
local bezier = require"bezier"

strokers.mpvg = true
strokers.openjdk8 = true
strokers.openjdk11 = true
strokers.oraclejdk8 = true

strokers.native = function(path, screen_xf, width, style)
    return path:stroked(width, style)
end

local function reverse_path(p)
    local rpath_data = path_data()
    p:as_path_data():riterate(rpath_data)
    return path(rpath_data):transformed(p:get_xf())
end

local function stderr(...)
    if not quiet then
        io.stderr:write(string.format(...), "\n")
    end
end

local function help()
stderr([[test-strokers.lua -test:<test name> -stroker:<stroker name> [ options ]

Mandatory arguments:

  -test:<test name>               test defining the input path to be stroked
    Test names:
      list                        print all test names to stdout

  -stroker:<stroker name>         stroker to use
    Stroker names:
      list                        print all stroker names

Optional arguments:

  -help                           prints this message and exit

  -driver:<driver name>           output driver (default: "svg")
    Driver names:
      svg                         output an svg file
      eps                         output an eps file
      rvg_lua                     output an rvg file
      skia                        output a png file
      cairo                       output a png file
      qt5                         output a png file
      nvpr                        output a png file
      ...

  -output:<output name>           output file name
                                  (default: output to stdout)

  -flip                           flip Y in output

  -parameter:<value>              pass parameter <value> to test

  -reverse                        reverse input path to be stroked

  -generatrix                     render input path to be stroked

  -arc-length                     print arc-length of input path and exit

  -control-points                 render all output outline control points

  -interpolated-points            render interpolated output outline
                                  control points

  -outline                        render output outline

  -idempotent                     fill with opaque color

  -no-idempotent                  fill with transparent color

  -split                          split output outlines into independent paths

  -no-background                  render transparent background

  -no-fill                        do not fill output path

  -width-scale:<value>            scale stroke width by value

  -join:<join name>               select outer join
    Join names:
      miter_clip
      miter_or_bevel
      round
      bevel

  -inner-join:<join name>         select inner join
    Join names:
      round
      bevel

  -cap:<cap name>                 select all caps
  -initial-cap:<cap name>         select initial cap
  -terminal-cap:<cap name>        select terminal cap
  -dash-initial-cap:<cap name>    select initial cap for dashes
  -dash-terminal-cap:<cap name>   select terminal cap for dashes
  -dash-cap:<cap name>            select all caps for dashes
    Cap names:
      butt
      round
      square
      triangle
      fletching

  -miter-limit:<value>            set limit for mitter joins

  -dash-offset:<value>            set initial dash offset

  -resets-on-move                 reset dash pattern for between outlines

  -dashes:<dash array>            comma-separated list of dash lengths

  -outline-width:<value>          stroke outline width in pixels

  -viewport-width:<value>         width of output viewport

  -viewport-border:<value>        viewport border in units of viewport width

  -animate-outline                generate animation of outline

  -animate-outline-speed          px per frame in animation

  -animate-fill                   generate animation of fill

]])
    os.exit()
end

local quiet = false
local arc_length = false
local generatrix = false
local reverse = false
local outline_width = 1
local no_background = false
local no_fill = false
local idempotent = false
local no_idempotent = false
local split = false
local differences = false
local outline = false
local animate_outline = false
local animate_outline_speed = 20
local animate_fill = false
local precomputed_scale = 1
local width_scale = 1
local inner_points = false
local interpolated_points = false
local inner_input_points = false
local interpolated_input_points = false
local initial_cap
local terminal_cap
local join
local flip
local inner_join
local miter_limit
local dash_initial_cap
local dash_terminal_cap
local dash_offset
local resets_on_move
local dashes
local testname, strokername, outputname
local drivername = "svg"
local precomputed
local parameter = 0.5
local viewport_border = 1/8
local viewport_width = 512

local options = {
    { "^%-help$", function(w)
        if w then
            help()
            return true
        else
            return false
        end
    end },
    { "^%-test%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        testname = o
        return true
    end },
    { "^%-precomputed%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        precomputed = o
        return true
    end },
    { "^%-generatrix$", function(o)
        if not o then return false end
        generatrix = true
        return true
    end },
    { "^%-flip$", function(o)
        if not o then return false end
        flip = true
        return true
    end },
    { "^%-arc%-length$", function(o)
        if not o then return false end
        arc_length = true
        return true
    end },
    { "^%-differences$", function(o)
        if not o then return false end
        differences = true
        return true
    end },
    { "^%-split$", function(o)
        if not o then return false end
        split = true
        return true
    end },
    { "^%-control%-points$", function(o)
        if not o then return false end
        inner_points = true
        interpolated_points = true
        return true
    end },
    { "^%-input%-control%-points$", function(o)
        if not o then return false end
        inner_input_points = true
        interpolated_input_points = true
        return true
    end },
    { "^%-interpolated%-points$", function(o)
        if not o then return false end
        interpolated_points = true
        return true
    end },
    { "^%-interpolated%-input%-points$", function(o)
        if not o then return false end
        interpolated_input_points = true
        return true
    end },
    { "^%-outline$", function(o)
        if not o then return false end
        outline = true
        return true
    end },
    { "^%-animate%-outline$", function(o)
        if not o then return false end
        animate_outline = true
        return true
    end },
    { "^%-animate%-fill$", function(o)
        if not o then return false end
        animate_fill = true
        return true
    end },
    { "^%-reverse$", function(o)
        if not o then return false end
        reverse = true
        return true
    end },
    { "^%-no%-background$", function(o)
        if not o then return false end
        no_background = true
        return true
    end },
    { "^%-no%-fill$", function(o)
        if not o then return false end
        no_fill = true
        return true
    end },
    { "^%-idempotent$", function(o)
        if not o then return false end
        idempotent = true
        return true
    end },
    { "^%-no%-idempotent$", function(o)
        if not o then return false end
        no_idempotent = true
        return true
    end },
    { "^%-stroker%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        strokername = o
        return true
    end },
    { "^%-driver%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        drivername = o
        return true
    end },
    { "^%-width%-scale%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        width_scale = tonumber(o)
        if not width_scale then
            error("invalid width scale")
        end
        return true
    end },
    { "^%-precomputed%-scale%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        precomputed_scale = tonumber(o)
        if not precomputed_scale then
            error("invalid precomputed scale")
        end
        return true
    end },
    { "^%-outline%-width%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        outline_width = tonumber(o)
        if not outline_width then
            error("invalid outline width")
        end
        return true
    end },
    { "^%-animate%-outline%-speed%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        animate_outline_speed = tonumber(o)
        if not animate_outline_speed then
            error("invalid outline animation speed")
        end
        return true
    end },
    { "^%-viewport%-width%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        viewport_width = math.ceil(tonumber(o))
        if not viewport_width then
            error("invalid viewport width")
        end
        return true
    end },
    { "^%-viewport%-border%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        viewport_border = tonumber(o)
        if not viewport_border then
            error("invalid viewport width")
        end
        return true
    end },
    { "^%-parameter%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        parameter = tonumber(o)
        if not parameter then
            error("invalid width parameter")
        end
        return true
    end },
    { "^%-output%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        outputname = o
        return true
    end },

    { "^%-join%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        join = o
        return true
    end },
    { "^%-inner%-join%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        inner_join = o
        return true
    end },
    { "^%-initial%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        initial_cap = o
        return true
    end },
    { "^%-terminal%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        terminal_cap = o
        return true
    end },
    { "^%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        terminal_cap = o
        initial_cap = o
        dash_initial_cap = o
        dash_terminal_cap = o
        return true
    end },
    { "^%-dash%-initial%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        dash_initial_cap = o
        return true
    end },
    { "^%-dash%-terminal%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        dash_terminal_cap = o
        return true
    end },
    { "^%-dash%-cap%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        dash_terminal_cap = o
        dash_initial_cap = o
        return true
    end },
    { "^%-miter%-limit%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        miter_limit = tonumber(o)
        if not miter_limit or miter_limit < 0 then
            error("invalid miter limit")
        end
        return true
    end },
    { "^%-dash%-offset%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        dash_offset = tonumber(o)
        if not dash_offset then
            error("invalid dash offset")
        end
        return true
    end },
    { "^%-resets%-on%-move$", function(o)
        if not o then return false end
        resets_on_move = true
        return true
    end },
    { "^%-dashes:(.*)$", function(o)
        if not o or #o < 1 then return false end
        dashes = o
        return true
    end },
    { ".*", function(all)
        error("unrecognized option " .. all)
    end }
}

-- rejected options are passed to driver
local rejected = {}
local nrejected = 0
-- value do not start with -
local values = {}
local nvalues = 0

for i, argument in ipairs({...}) do
    if argument:sub(1,1) == "-" then
        local recognized = false
        for j, option in ipairs(options) do
            if option[2](argument:match(option[1])) then
                recognized = true
                break
            end
        end
        if not recognized then
            nrejected = nrejected + 1
            rejected[nrejected] = argument
        end
    else
        nvalues = nvalues + 1
        values[nvalues] = argument
    end
end

if not package.searchpath(drivername, package.cpath) and
   not package.searchpath(drivername, package.path) then
    drivername = "driver." .. drivername
end

local driver = require(drivername)
setmetatable(_G, { __index = driver } )

-- load mpvg stroker after loading driver
local mpvg = require"strokers.mpvg"
strokers.mpvg = mpvg.stroke

-- load java stroker after loading driver
local java = require"strokers.java"
strokers.openjdk8 = java.openjdk8
strokers.openjdk11 = java.openjdk11
strokers.oraclejdk8 = java.oraclejdk8

local bevels = stroke_style():inner_joined(stroke_join.bevel):
    joined(stroke_join.bevel)
local outer_bevel = stroke_style():joined(stroke_join.bevel)
local rounds = stroke_style():inner_joined(stroke_join.round):
    joined(stroke_join.round)

local function append(t, u)
    for i,v in ipairs(u) do
        t[#t+1] = v
    end
end

local tests = {
    ["jw2_1"] = {path{M, -8.47214,-3.23607, L, -4,-1, C, -1,-2, 3, -2, 6, -1}, 10, outer_bevel},
    ["rjw2_1"] = {path{M, 6, -1, C, 3, -2, -1,-2, -4,-1, L, -8.47214,-3.23607}, 10, outer_bevel},
    ["jw2_2"] = {path{M, -8.92876,-1.84104, L, -4,-1, C, 0.39,-5.18, 1.4, -3.44, 6, -1}, 10, outer_bevel},
    ["rjw2_2"] = {path{M, 6, -1, C, 1.4, -3.44, 0.39,-5.18, -4,-1, L, -8.92876,-1.84104}, 10, outer_bevel},
    ["jw1"] = {path{M, -5.75564,-5.68164, L, -4,-1, C, 0.39,-5.18, 1.4, -3.44, 6, -1}, 10, outer_bevel},
    ["rjw1"] = {path{M, 6, -1, C, 1.4, -3.44, 0.39,-5.18, -4,-1, L, -5.75564,-5.68164}, 10, outer_bevel},
    ["jw0"] = {path{M, -3,-3, L, -4,-1, C, -2.06,-2.78,-0.72,-3.16,0.44,-3.24}, 10, outer_bevel},
    ["rjw0"] = {path{M, 0.44,-3.24, C, -0.72,-3.16, -2.06,-2.78, -4,-1, L, -3,-3}, 10, outer_bevel},
    ["wedge-q"] = {path{M,0., 0., Q, 37.5, -45, 75, 0.}, 225, bevels},
    ["wedge-hq"] = {path{M, 18.75, -16.875, Q, 28.125, -22.5, 37.5, -22.5}, 225, bevels},
    ["error_wedge"] = {path{M,30,50,40,40,50,50}, 200, stroke_style():joined(stroke_join.round)},
    ["error_cusp"] = {path{M,20,260,C,80,200,20,200,80,260}, 50, stroke_style():joined(stroke_join.round)},
    ["error_curvature"] = {path{M,40,120,C,30,120,30,150,30,160}, 50, stroke_style():joined(stroke_join.round)},
    ["2nd"] = {path{M,409.0625,205.41666666666671,C,332.62,101.86669999999999,428.4375,91.25,303.4375,242.91666666666671}, 20.000000},
    ["almost_cubic_triple_X-X-Y-X"] = {path{M,410.3125,203.75,C,409.6875,203.75,271.5625,124.58333333333331,409.6875,203.75}, 20.000000},
    ["almost_outside_cubic_line"] = {path{M,400,300,C,202.1875,199.58333333333331,300,250,500,350}, 20.000000},
    ["almost_outside_cubic_line2"] = {path{M,200,200,C,301.5625,244.58333333333331,400,300,500,350}, 20.000000},
    ["almost_outside_quadratic_line"] = {path{M,300,300,Q,248.4375,248.75,350,350,}, 20.000000},
    ["alternative_lazy_loop"] = {path{M,296.5625,124.58333333333331,C,245.3125,119.58333333333331,457.1875,104.58333333333331,413.4375,153.75}, 64.000000},
    ["another_huge_quadratic_hull"] = {path{M,295.9375,210.41666666666666,Q,397.8125,289.58333333333331,363.4375,169.58333333333331,}, 256.000000},
    ["centurion_head"] = {path{M,368.4375,162.91666666666666,C,364.0625,138.75,445.3125,113.75,407.8125,132.91666666666663}, 108.000000},
    ["cubic_loop_with_hole_and_negative_radius"] = {path{M,409.0625,205.41666666666671,C,232.62,131.86670000000001,428.4375,91.25,343.4375,242.91666666666671}, 22.000000},
    ["cubic_loop_with_hole,_reverse_order"] = {path{M,343.4375,242.91666666666671,C,428.4375,91.25,232.62,131.86670000000001,409.0625,205.41666666666671}, 20.000000},
    ["cubic_loop_with_hole"] = {path{M,409.0625,205.41666666666671,C,232.62,131.86670000000001,428.4375,91.25,343.4375,242.91666666666671}, 20.000000},
    ["cubic_loop_without_hole"] = {path{M,409.0625,205.41666666666671,C,232.62,131.86670000000001,428.4375,91.25,343.4375,242.91666666666671}, 38.000000},
    ["cubic_serpentine"] = {path{M,489.0625,21.25,C,212.1875,19.583333333333314,471.5625,368.75,222.8125,379.58333333333331}, 20.000000},
    ["cubic_triple_X-X-Y-X"] = {path{M,409.6875,203.75,C,409.6875,203.75,271.5625,124.58333333333331,409.6875,203.75}, 20.000000},
    ["cusp_with_empty_hull"] = {path{M,250.00004577636719,100.00004577636719,C,450,200,350,200,350,100}, 20.000000},
    ["easy_wishbone_quadratic"] = {path{M,403.4375,159.58333333333331,Q,350.3125,316.25,299.0625,147.08333333333331,}, 20.000000},
    ["exact_cusp"] = {path{M,250,100,C,450,200,350,200,350,100}, 20.000000},
    ["hard_hole"] = {path{M,368.4375,162.91666666666666,C,364.0625,138.75,445.3125,113.75,410.3125,145.41666666666666}, 108.000000},
    ["hard_quadratic_extension_case"] = {path{M,374.86232207393869,174.82420053647371,C,262.8125,285.41666666666663,374.86402487963846,174.82420584529896,374.86402487963846,174.82420584529896}, 20.000000},
    ["huge_quadratic_hull_with_3_nearby_ctrl_points_+_big_radius"] = {path{M,357.1875,170.41666666666666,Q,360.3125,175.41666666666666,363.4375,169.58333333333331,}, 256.000000},
    ["huge_quadratic_with_knotched_edge,_far_off_control_point,_almost_deeply_recursive_cusp"] = {path{M,357.1875,170.41666666666666,Q,360.3125,175.41666666666666,1304.0625,-507.92971801757812,}, 256.000000},
    ["huge_quadratic_with_knotched_edge,_far_off_control_point,_deeply_recursive_cusp"] = {path{M,357.1875,170.41666666666666,Q,360.3125,175.41666666666666,1304.0625,-507.91666666666674,}, 256.000000},
    ["huge_radius_quadratic_hull_escape"] = {path{M,290.9375,323.75,Q,379.0625,193.75,365.9375,347.91666666666669,}, 122.000000},
    ["huge_radius_quadratic"] = {path{M,250,250,Q,379.0625,193.75,350,350,}, 274.000000},
    ["huge_radius_quadratic_with_near_mid_vertex"] = {path{M,290.9375,323.75,Q,283.4375,243.75,387.1875,337.08333333333331,}, 122.000000},
    ["inside_cubic_line"] = {path{M,200,200,C,300,250,400,300,500,350}, 20.000000},
    ["inside_quadratic_line"] = {path{M,250,250,Q,300,300,350,350,}, 20.000000},
    ["kink"] = {path{M,409.0625,205.41666666666671,C,234.6875,265.41666666666663,410.3125,205.41666666666666,233.4375,265.41666666666663}, 20.000000},
    ["lazy_loop"] = {path{M,272.1875,134.58333333333331,C,218.4375,95.416666666666629,445.3125,113.75,407.8125,132.91666666666663}, 94.000000},
    ["line_case"] = {path{M,300,300,L,250,250,350,350,}, 20.000000, stroke_style():joined(stroke_join.bevel)},
    ["narrow_quadratic_with_far_far_extrapolating_control_point"] = {path{M,357.8125,242.08333333333331,Q,-4481.304347826087,-7769.8113207547158,381.304347826087,245.28301886792454,}, 18.000000},
    ["narrow_quadratic_with_far_far_extrapolating_control_point,_transitioned_to_hull"] = {path{M,361.84860229492187,247.91666666666666,Q,-4481.304347826087,-7769.8113207547158,381.304347826087,245.28301886792454,}, 18.000000},
    ["nearly_cusp"] = {path{M,250,100,C,450,200,349.6875,199.58333333333331,350,100}, 20.000000},
    ["outside_cubic_line"] = {path{M,400,300,C,200,200,300,250,500,350}, 20.000000},
    ["outside_quadratic_line"] = {path{M,300,300,Q,250,250,350,350,}, 20.000000},
    ["quadratic_line"] = {path{M,250,250,Q,275,275,350,350,}, 20.000000},
    ["quadratic_over_bend"] = {path{M,338.4375,187.91666666666666,Q,208.4375,317.91666666666663,471.5625,35.416666666666629,}, 40.200000},
    ["quadratic_over_bend_with_negative_radius"] = {path{M,338.4375,187.91666666666666,Q,208.4375,317.91666666666663,471.5625,35.416666666666629,}, 40.200000},
    ["quadratic_thats_almost_a_circle"] = {path{M,374.86232207393869,174.82420053647371,C,374.88941303212323,174.85037433620616,374.86402487963846,174.82420584529896,374.86402487963846,174.82420584529896}, 20.000000},
    ["quadratic_with_far_far_extrapolating_control_point_hull_sensitive"] = {path{M,357.1875,170.41666666666666,Q,-4481.304347826087,-7769.8113207547158,381.304347826087,245.28301886792454,}, 110.000000},
    ["ropey_nearly_inside_quadratic_line"] = {path{M,250,250,Q,300,299.99957275390625,350,350,}, 20.000000},
    ["short_quadratic"] = {path{M,375,175,Q,373.4375,173.4375,371.09375,170.3125,}, 20.000000},
    ["small_nick"] = {path{M,369.0625,160.41666666666666,C,369.0625,158.75,434.0625,117.08333333333331,414.0625,137.08333333333331}, 118.000000},
    ["sneaky_lazy_loop"] = {path{M,273.4375,140.41666666666663,C,229.6875,111.25,470.9375,119.58333333333331,434.6875,104.58333333333331}, 94.000000},
    ["tight_narrow_serpentine"] = {path{M,338.4375,187.91666666666666,C,208.4375,317.91666666666663,494.0625,7.0833333333333144,332.8125,177.08333333333331}, 1.000000},
    ["tight_turn_with_inflection"] = {path{M,462.1875,86.25,C,455.3125,81.25,330.9375,242.91666666666666,267.8125,87.916666666666629}, 20.000000},
    ["toes_to_fingers_loop"] = {path{M,345.9375,127.91666666666663,C,225.3125,114.58333333333331,457.1875,104.58333333333331,342.1875,127.91666666666663}, 38.000000},
    ["tricky_bend,_almost_180_degree_end_point_turn"] = {path{M,445.9375,82.916666666666629,C,447.8125,80.416666666666629,317.1875,257.91666666666663,267.8125,87.916666666666629}, 20.000000},
    ["tricky_bend_assertion"] = {path{M,462.1875,86.25,C,467.1875,80.416666666666629,317.1875,257.91666666666663,267.8125,87.916666666666629}, 20.000000},
    ["triple_point_cubic_X-X-X-Y"] = {path{M,250,250,C,250,250,250,250,350,350}, 20.000000},
    ["very_tight_narrow_serpentine"] = {path{M,338.4375,187.91666666666666,C,208.4375,317.91666666666663,494.0625,7.0833333333333144,332.8125,177.08333333333331}, 0.200000},
    ["wide_cubic_serpentine,_almost_wrong"] = {path{M,327.8125,204.58333333333331,C,399.6875,282.08333333333331,293.4375,142.91666666666663,343.4375,208.75}, 58.000000},
    ["wide_cubic_serpentine"] = {path{M,327.8125,204.58333333333331,C,399.0625,279.58333333333331,293.4375,142.91666666666663,343.4375,208.75}, 58.000000},
    ["inner_join_problem"] = {path{M,110.,100.,107.071,92.9289,100.,90.,92.9289,92.9289,90.,100.}, 100},
    ["arc"] = {path{M,0,0,R,42.3496, 109.197, 0.727982,100., 150.,103.247,109.197,0.727982,200,0,Z}, 5, outer_bevel},
    ["arc_flat"] = {path{M,0,0,2.706,6.871,5.529,13.823,8.466,20.836,11.511,27.890,14.659,34.961,17.902,42.027,21.234,49.062,24.645,56.040,28.125,62.934,31.663,69.717,35.249,76.363,38.870,82.843,42.513,89.131,46.165,95.202,49.813,101.031,53.443,106.596,57.042,111.875,60.597,116.850,64.095,121.504,67.523,125.823,70.871,129.795,72.511,131.648,74.126,133.412,75.717,135.085,77.281,136.668,78.817,138.159,80.324,139.559,81.802,140.868,83.250,142.086,84.666,143.213,86.050,144.249,87.402,145.196,88.066,145.636,88.721,146.054,89.367,146.450,90.005,146.824,90.635,147.177,91.256,147.507,91.868,147.817,92.472,148.105,93.067,148.371,93.653,148.617,94.231,148.842,94.799,149.047,95.359,149.231,95.910,149.394,96.452,149.538,96.986,149.662,97.510,149.766,98.026,149.851,98.280,149.886,98.532,149.916,98.783,149.942,99.030,149.963,99.276,149.979,99.519,149.990,99.761,149.997,100,150,100.238,149.997,100.480,149.990,100.723,149.979,100.969,149.963,101.217,149.942,101.467,149.916,101.719,149.886,101.973,149.851,102.489,149.766,103.013,149.662,103.547,149.538,104.089,149.394,104.640,149.231,105.200,149.047,105.768,148.842,106.346,148.617,106.932,148.371,107.527,148.105,108.131,147.817,108.743,147.507,109.364,147.177,109.994,146.824,110.632,146.450,111.278,146.054,111.933,145.636,112.597,145.196,113.949,144.249,115.333,143.213,116.749,142.086,118.197,140.868,119.675,139.559,121.182,138.159,122.719,136.668,124.282,135.085,125.873,133.412,127.488,131.648,129.128,129.795,132.476,125.823,135.904,121.504,139.402,116.850,142.957,111.875,146.556,106.596,150.186,101.031,153.834,95.202,157.486,89.131,161.129,82.843,164.750,76.363,168.336,69.717,171.875,62.934,175.355,56.040,178.765,49.062,182.097,42.027,185.340,34.961,188.488,27.890,191.533,20.836,194.470,13.823,197.293,6.871,200,0,0,0,Z,}, 5, stroke_style():joined(stroke_join.bevel)},
    ["triangle"] = {path{M,0,0,100., 150.,200,0,0,0,Z}, 5},
    ["squiggle"] = {path{M,0,0,50,100,100,0,150,100}, 5},
    ["rotated_triangle"] = {path{M,0,0,100., 150.,200,0,Z}:rotated(0.1), 5, stroke_style():joined(stroke_join.bevel)},
    ["degenerate"] = {path{M,256,256,L,256,256,Z}, 5},
    ["hole"] = {path{M, -6, -3, C, -3.5, 9.5, 0.5, 1, -2, 2}, 6},
    ["curvature"] = {path{M,40,120,C,30,120,30,150,30,160}, 50},
    ["wedge"] = {path{M,0., 0., Q, 37.5, -45, 75, 0.}, 225},
    ["almost_cusp"] = function(v)
        local d = 3
        local a = (20-d)*(1-v) + (20+d)*v
        local b = (80+d)*(1-v) + (80-d)*v
        return {path{M,20,260,C,b,200,a,200,80,260}, 50}
    end,
    ["squishing_cusp"] = function(v)
        local y1 = 260*(1-v) + 200*v
        local y2 = 200*(1-v) + 260*v
        local bbox = {xmin = 20, ymin = 200, xmax = 80, ymax = 260}
        return {path{M,20,y1,C,80,y2,20,y2,80,y1}:rotated(180*v, 50, 230), 50, nil, bbox}
    end,
    ["almost_degenerate_cusp"] = function(v)
        local d = 1
        local x0 = 50+(1-v)*(-2*d)+v*(2*d)
        local x1 = 50+(1-v)*(-d)+v*(d)
        local x2 = 50+(1-v)*(d)+v*(-d)
        local x3 = 50+(1-v)*(2*d)+v*(-2*d)
        local bbox = {xmin = 0, ymin = 0, xmax = 100, ymax = 100}
        return {path{M,x0,0,C,x1,150,x2,-50,x3,100}:rotated(30, 50,50), 15, nil, bbox}
    end,
    ["moving_cusp"] = function(v)
        local a = 0.5*v
        local b = a + 0.5
        local x0,y0, x1,y1, x2,y2, x3,y3 =
            bezier.cut3(a, b, 20,260, 80,200, 20,200, 80,260)
        local bbox = {xmin = 20, ymin = 200, xmax = 80, ymax = 260}
        return {path{M, x0,y0, C, x1,y1, x2,y2, x3,y3}, 50, nil, bbox}
    end,
    ["cusp"] = function(v)
        local a = 0.5*v
        local b = a + 0.5
        local x0,y0, x1,y1, x2,y2, x3,y3 =
            bezier.cut3(a, b, 20,260, 80,200, 20,200, 80,260)
        local r = 25
        local bbox = {xmin = 20-r, ymin = 200-r, xmax = 80+r, ymax = 260+r}
        return {path{M, x0,y0, C, x1,y1, x2,y2, x3,y3}, 2*r, nil, bbox}
    end,
    ["wedge_subdiv"] = function(v)
        local max = 256
        local min = 2
        local n = math.floor(min*(1-v^2)+max*v^2)
        local p = {M}
        for t = 0, n do
            local x,y = bezier.at2(t/n, 0., 0., 37.5, -45, 75, 0.)
            p[#p+1] = x
            p[#p+1] = y
        end
        local bbox = {xmin = 0, ymin = -45, xmax = 75, ymax = 0}
        return {path(p), 225, nil, bbox}
    end,
    ["moving_dashes"] = function(v)
        local d = {4,2,0,2}
        local o = 0
        for _, e in ipairs(d) do
            o = o+e
        end
        o = o*v
        local s = stroke_style():dashed(d):dash_offset(o):capped(stroke_cap.square):joined(stroke_join.round)
        local p = path{M, 0,10, 10*math.sqrt(3)/2, -5, Q, 0,5, -10*math.sqrt(3)/2, -5, Z}
        return {p, 0.5, s}
    end,
    ["error_miter"] = {path{M,459.3990173339844,168.2480163574219,C,428.6883544921875,132.7578887939453,392.2296447753906,98.64330291748047,350.5916442871094,76.43108367919922,L,345.5217590332031,73.72650146484375,}, 1, nil, {xmin = 0, ymin = 0, xmax = 512, ymax = 229}},
    ["miter_clip"] = function(v)
        -- move angle, then move limit
        local u
        if v < 0.5 then
            v = v * 2
            u = 0
        else
            u = (v - 0.5)*2
            v = 1
        end
        local s = stroke_style():joined(stroke_join.miter_clip):miter_limited(9*(1-u)+1*u)
        local x = 9*(1-v)+1*v
        local p = path{M, x,0, 0,10, -x,0}
        return {p, 1, s, {xmin = -10, xmax = 10, ymin = 0, ymax = 20}}
    end,
	["parallel_miter_clip"] = {path{M,0,0,10,0,5,0}, 1, stroke_style():joined(stroke_join.miter_clip):miter_limited(2)},
	["parallel_miter_or_bevel"] = {path{M,0,0,10,0,5,0}, 1, stroke_style():joined(stroke_join.miter_or_bevel):miter_limited(2)},
    ["zigzagp"] = function(v)
        local x0 = 10
        local x = x0
        local dx = 30
        local y0 = 10
        local y1 = 20
        local t = {M, x, y0}
        local n = 12
        for i = 0, n do
            local u = i/n
            local v0 = y0*(1-u)+y1*u
            local v1 = y1*(1-u)+y0*u
            if i %2 == 1 then
                append(t, {C, x+dx/3, y1, x+2*dx/3, v0, x+dx, y0})
            else
                append(t, {L, x+dx, y1})
            end
            x = x + dx
        end
        local sw = 60
        local s = (1-v)*(1.1) + v*(0.525)
        local w = 2*x0+(n+1)*dx
        local h = y1+y0
        return {path(t):scaled(s, 1, w/2, h/2), 60, stroke_style():joined(stroke_join.round) , {xmin = -sw/2, ymin = -sw/2, ymax = h+sw/2, xmax = w+sw/2} }
    end,
    ["zigzag"] = function(v)
        local x = 10
        local dx = 30
        local t = {M, x, 10}
        local n = 12
        local y0 = 10
        local y1 = 20
        for i = 0, n do
            local u = i/n
            local v0 = y0*(1-u)+y1*u
            local v1 = y1*(1-u)+y0*u
            if i %2 == 1 then
                append(t, {C, x+dx/3, y1, x+2*dx/3, v0, x+dx, y0})
            else
                append(t, {L, x+dx, y1})
            end
            x = x + dx
        end
        return {path(t), 60, stroke_style():joined(stroke_join.round) }
    end,
    ["talk0"] = {path"M 28.778009,117.86509 190.92117,341.03638 190.92153,65.18165 28.777787,288.35252 291.13142,203.10934 Z M 540.17648,201.30433 A 83.288646,83.288641 0 0 1 456.88784,284.59298 83.288646,83.288641 0 0 1 373.5992,201.30433 83.288646,83.288641 0 0 1 456.88784,118.01569 83.288646,83.288641 0 0 1 540.17648,201.30433 Z M 581.363,201.30433 A 124.47516,124.47516 0 0 1 456.88784,325.77949 124.47516,124.47516 0 0 1 332.41268,201.30433 124.47516,124.47516 0 0 1 456.88784,76.82918 124.47516,124.47516 0 0 1 581.363,201.30433 Z M 738.07885,329.59452 C 616.8906,330.20413 587.1213,96.62474 661.94069,96.62474 736.76008,96.62475 804.68745,223.95596 737.4281,223.52168 670.16874,223.0874 739.2097,96.62474 810.96325,96.62475 882.71679,96.62476 859.2671,328.98491 738.07885,329.59452 Z", 2, stroke_style(), {xmin = 0, xmax = 892.8, ymin = 0, ymax = 438.72}},
    ["talk1"] = {path"M 28.7845,117.94 291.197,203.239 28.7842,288.537 190.964,65.223 V 341.254 Z M 332.487,201.433 A 124.555,124.503 89.9673 0 0 456.99,325.987 124.503,124.555 0 0 0 581.493,201.433 124.503,124.555 0.0044 0 0 456.99,76.878 124.503,124.555 0.048 0 0 332.487,201.433 Z M 540.297,201.433 A 83.3418,83.3074 89.9098 0 1 456.99,284.775 83.3418,83.3073 89.9976 0 1 373.683,201.433 83.3073,83.3418 0.0316 0 1 456.99,118.091 83.3074,83.3418 0.0512 0 1 540.297,201.433 Z M 738.244,329.805 C 859.46,329.195 882.915,96.686 811.145,96.686 739.375,96.686 670.319,223.23 737.593,223.664 804.868,224.099 736.925,96.686 662.089,96.686 587.253,96.686 617.029,330.415 738.244,329.805 Z ", 2, stroke_style(), {xmin = 0, xmax = 892.8, ymin = 0, ymax = 438.72}},
    --["talk2"] = {path"M 140.637 50.2785 C 136.795 57.0195 131.895 63.0545 125.543 67.4963 119.011 72.1122 110.795 74.8814 102.763 73.624 97.2295 72.7842 92.1065 69.7697 88.9798 65.0384", 28.442710876465, stroke_style(), {xmin = 0, ymin = 0, xmax = 236.22, ymax = 99.75}},
    --["talk3"] = {path"M 140.637 50.2785 C 136.795 57.0195 131.895 63.0545 125.543 67.4963 119.011 72.1122 110.795 74.8814 102.763 73.624 97.2295 72.7842 92.1065 69.7697 88.9798 65.0384", 145.521, stroke_style(), {xmin = 0, ymin = 0, xmax = 236.22, ymax = 99.75}},
    ["talk2"] = {path"M 140.71144,50.207502 C 125.33922,77.122871 98.552892,79.50314 88.979771,65.038416", 28.442710876465, stroke_style(), {xmin = 0, ymin = 0, xmax = 236.22, ymax = 99.75}},
    ["talk3"] = {path"M 140.71144,50.207502 C 125.33922,77.122871 98.552892,79.50314 88.979771,65.038416", 145.521, stroke_style(), {xmin = 0, ymin = 0, xmax = 236.22, ymax = 99.75}},
}

if testname == "list" then
	for i,v in pairs(tests) do
		print(i)
	end
	os.exit()
end

if strokername == "list" then
	for i,v in pairs(strokers) do
        if i ~= "native" and i ~= "arc_length" then
            print(i)
        end
	end
	os.exit()
end

local test = tests[testname]
    or error(string.format("test '%s' not found", tostring(testname)))

if type(test) == "function" then
    test = test(parameter)
end

if arc_length then
    print(strokers.arc_length(test[1])/(test[2]*width_scale))
    os.exit()
end

local stroker = strokers[strokername]
    or error(string.format("stroker '%s' not found", tostring(strokername)))

if strokername == "openvg_ri" then
    if not no_idempotent then
        idempotent = true
    end
    split = true
end

local stroke_color = rgba(1, 0, 0, 0.5)

local generatrix_color = rgb8(136, 49, 0)

if idempotent then
    stroke_color = rgba(1, 0.5, 0.5, 1)
end

local function newbbox()
	return {
		xmin = math.huge, ymin = math.huge,
		xmax = -math.huge, ymax = -math.huge,
		begin_contour = function(self, x0, y0)
			self.xmin = min(self.xmin, x0); self.ymin = min(self.ymin, y0)
			self.xmax = max(self.xmax, x0); self.ymax = max(self.ymax, y0)
		end,
		linear_segment = function(self, x0, y0, x1, y1)
			self.xmin = min(self.xmin, x1); self.ymin = min(self.ymin, y1)
			self.xmax = max(self.xmax, x1); self.ymax = max(self.ymax, y1)
		end,
		quadratic_segment = function(self, x0, y0, x1, y1, x2, y2)
			self.xmin = min(self.xmin, x1); self.ymin = min(self.ymin, y1)
			self.xmax = max(self.xmax, x1); self.ymax = max(self.ymax, y1)
			self.xmin = min(self.xmin, x2); self.ymin = min(self.ymin, y2)
			self.xmax = max(self.xmax, x2); self.ymax = max(self.ymax, y2)
		end,
		rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
			self.xmin = min(self.xmin, x1); self.ymin = min(self.ymin, y1)
			self.xmax = max(self.xmax, x1); self.ymax = max(self.ymax, y1)
			self.xmin = min(self.xmin, x2); self.ymin = min(self.ymin, y2)
			self.xmax = max(self.xmax, x2); self.ymax = max(self.ymax, y2)
		end,
		cubic_segment = function(self, x0, y0, x1, y1, x2, y2, x3, y3)
			self.xmin = min(self.xmin, x1); self.ymin = min(self.ymin, y1)
			self.xmax = max(self.xmax, x1); self.ymax = max(self.ymax, y1)
			self.xmin = min(self.xmin, x2); self.ymin = min(self.ymin, y2)
			self.xmax = max(self.xmax, x2); self.ymax = max(self.ymax, y2)
			self.xmin = min(self.xmin, x3); self.ymin = min(self.ymin, y3)
			self.xmax = max(self.xmax, x3); self.ymax = max(self.ymax, y3)
		end,
		end_open_contour = function(self, x0, y0)
		end,
		end_closed_contour = function(self, x0, y0)
		end,
	}
end

local function adjust_style(style)
    style = style or stroke_style()
    if initial_cap then
        local cap = stroke_cap[initial_cap]
        if not cap then
            error("invalid initial cap")
        end
        style = style:initial_capped(cap)
    end
    if terminal_cap then
        local cap = stroke_cap[terminal_cap]
        if not cap then
            error("invalid terminal cap")
        end
        style = style:terminal_capped(cap)
    end
    if dash_initial_cap then
        local cap = stroke_cap[dash_initial_cap]
        if not cap then
            error("invalid dash initial cap")
        end
        style = style:dash_initial_capped(cap)
    end
    if dash_terminal_cap then
        local cap = stroke_cap[dash_terminal_cap]
        if not cap then
            error("invalid dash terminal cap")
        end
        style = style:dash_terminal_capped(cap)
    end
    if miter_limit then
        style = style:mitered(miter_limit)
    end
    if join then
        local j = stroke_join[join]
        if not j then
            error("invalid join")
        end
        style = style:joined(j)
    end
    if inner_join then
        local j = stroke_join[inner_join]
        if not j then
            error("invalid inner join")
        end
        style = style:inner_joined(j)
    end
    if dash_offset then
        style = style:dash_offset(dash_offset)
    end
    if resets_on_move then
        style = style:reset_on_move(true)
    end
    if dashes then
        local d = {}
        string.gsub(dashes .. ",", "(.-),", function (v)
            v = tonumber(v)
            if not v then error("invalid dashes") end
            d[#d+1] = v
        end)
        style = style:dashed(d)
    end
    return style
end

local test_path = test[1]
local test_stroke_width = test[2]
local test_stroke_style = adjust_style(test[3])
local test_bbox = test[4]

if reverse then
    test_path = reverse_path(test_path)
end

-- acquire bounding box of path
local bbox = newbbox()
test_path:as_path_data():iterate(
    filter.make_input_path_f_xform(test_path:get_xf(),
    filterx.monotonize(bbox)))
-- expand to include stroke
local half_stroke_width = .5*test_stroke_width
bbox.xmin = bbox.xmin-half_stroke_width
bbox.ymin = bbox.ymin-half_stroke_width
bbox.xmax = bbox.xmax+half_stroke_width
bbox.ymax = bbox.ymax+half_stroke_width
-- add a border
local bbox_width = bbox.xmax-bbox.xmin
local bbox_border = viewport_border*bbox_width
bbox.xmin = bbox.xmin-bbox_border
bbox.ymin = bbox.ymin-bbox_border
bbox.xmax = bbox.xmax+bbox_border
bbox.ymax = bbox.ymax+bbox_border
-- allow bbox override
bbox = test_bbox or bbox
-- compute viewport to preserve aspect ratio
bbox_width = bbox.xmax-bbox.xmin
bbox_height = bbox.ymax-bbox.ymin
local viewport_height = math.ceil(viewport_width*bbox_height/bbox_width)
local vp = viewport(0, 0, math.ceil(viewport_width), math.ceil(viewport_height))
-- transform path to viewport coordinates
test_path = test_path:windowviewport(window(bbox.xmin, bbox.ymax, bbox.xmax, bbox.ymin), vp)
test_stroke_width = test_stroke_width*viewport_width/bbox_width
-- compute window
local window_width = viewport_width
local window_height = viewport_height
local wnd = window(0, 0, window_width, window_height)

-- stroke path
local stroked_shape
if precomputed then
    local rvg = assert(assert(loadfile(precomputed, "bt", driver))())
    local ps = {}
    rvg.scene:get_scene_data():iterate{
        painted_shape = function(self, winding_rule, shape, paint)
            ps[#ps+1] = shape
        end
    }
    assert(#ps >= 1, "precomputed rvg should contain at least one painted element")
    if #ps > 1 then
        stderr("precomputed rvg contains multiple painted elements.\nkeeping topmost")
    end
    if precomputed_scale ~= 1 then
        local pd = path_data()
        ps[#ps]:as_path_data():iterate(filter.make_input_path_f_xform(
            scaling(precomputed_scale), pd))
        stroked_shape = path(pd)
    else
        stroked_shape = ps[#ps]
    end
else
    stroked_shape = stroker(test_path, identity(),
        test_stroke_width*width_scale, test_stroke_style)
end

local rvg_stroked_shape

if flip then
    stroked_shape = stroked_shape:translated(0,-viewport_height):scaled(1, -1)
end

-- empty scene
local s = {}

-- add white background
if not no_background then
    s[#s+1] = fill(rect(0, 0, viewport_width, viewport_height), color.white)
end

-- add differences relative to rvg
if differences then
    rvg_stroked_shape = strokers.rvg(test_path, identity(),
        test_stroke_width*width_scale, test_stroke_style)
    if flip then
        rvg_stroked_shape = rvg_stroked_shape:
            translated(0,-viewport_height):scaled(1, -1)
    end
    s[#s+1] = clip(nzpunch(rvg_stroked_shape), fill(stroked_shape, color.red))
    s[#s+1] = clip(nzpunch(stroked_shape), fill(rvg_stroked_shape, color.red))
    s[#s+1] = fill(stroked_shape, stroke_color)
end

if split then
    local pd
    local splitter = setmetatable({
        begin_contour = function(self, x0, y0)
            pd = path_data()
            pd:begin_contour(x0, y0)
        end,
        end_open_contour = function(self, x0, y0)
            pd:end_open_contour(x0, y0)
            local p = path(pd)
            if not no_fill then
                s[#s+1] = fill(p, stroke_color)
            end
            if outline then
                s[#s+1] = fill(p:stroked(outline_width), color.black)
            end
            pd = path_data()
        end,
        end_closed_contour = function(self, x0, y0)
            pd:end_closed_contour(x0, y0)
            local p = path(pd)
            if not no_fill then
                s[#s+1] = fill(p, stroke_color)
            end
            if outline then
                s[#s+1] = fill(p:stroked(outline_width), color.black)
            end
            pd = path_data()
        end
    }, {
        __index = function(self, method)
            local new = function(self, ...)
                pd[method](pd, ...)
            end
            self[method] = new
            return new
        end
    })
    stroked_shape:as_path_data():iterate(splitter)
else
    -- create fill and outline
    if not no_fill then
        s[#s+1] = fill(stroked_shape, stroke_color)
    end
    if outline then
        s[#s+1] = fill(stroked_shape:stroked(outline_width), color.black)
    end
end

-- add interpolating control points in black, and
-- non-interpolating in darkgreen
if inner_points or interpolated_points then
    local r = 2*outline_width
    stroked_shape:as_path_data():iterate(
        filter.make_input_path_f_xform(stroked_shape:get_xf(), {
        begin_contour = function(self, x, y)
            s[#s+1] = fill(circle(x, y, r), color.black)
        end,
        linear_segment = function(self, x0, y0, x1, y1)
            s[#s+1] = fill(circle(x1, y1, r), color.black)
        end,
        quadratic_segment = function(self, x0, y0, x1, y1, x2, y2)
            if inner_points then
                s[#s+1] = fill(circle(x1, y1, r), color.darkgreen)
            end
            s[#s+1] = fill(circle(x2, y2, r), color.black)
        end,
        cubic_segment = function(self, x0, y0, x1, y1, x2, y2, x3, y3)
            if inner_points then
                s[#s+1] = fill(circle(x1, y1, r), color.darkgreen)
                s[#s+1] = fill(circle(x2, y2, r), color.darkgreen)
            end
            s[#s+1] = fill(circle(x3, y3, r), color.black)
        end,
        rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
            if inner_points then
                s[#s+1] = fill(circle(x1/w1, y1/w1, r), color.darkgreen)
            end
            s[#s+1] = fill(circle(x2, y2, r), color.black)
        end,
        end_open_contour = function(self, x, y)
        end,
        end_closed_contour = function(self, x, y)
        end,
    }))
end

-- add interpolating control points in , and
-- non-interpolating in darkgreen
if inner_input_points or interpolated_input_points then
    local r = 2*outline_width
    test_path:as_path_data():iterate(
        filter.make_input_path_f_xform(test_path:get_xf(), {
        begin_contour = function(self, x, y)
            s[#s+1] = fill(circle(x, y, r), generatrix_color)
        end,
        linear_segment = function(self, x0, y0, x1, y1)
            s[#s+1] = fill(circle(x1, y1, r), generatrix_color)
        end,
        quadratic_segment = function(self, x0, y0, x1, y1, x2, y2)
            if inner_points then
                s[#s+1] = fill(circle(x1, y1, r), color.darkred)
            end
            s[#s+1] = fill(circle(x2, y2, r), generatrix_color)
        end,
        cubic_segment = function(self, x0, y0, x1, y1, x2, y2, x3, y3)
            if inner_points then
                s[#s+1] = fill(circle(x1, y1, r), color.darkred)
                s[#s+1] = fill(circle(x2, y2, r), color.darkred)
            end
            s[#s+1] = fill(circle(x3, y3, r), generatrix_color)
        end,
        rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
            if inner_points then
                s[#s+1] = fill(circle(x1/w1, y1/w1, r), color.darkred)
            end
            s[#s+1] = fill(circle(x2, y2, r), generatrix_color)
        end,
        end_open_contour = function(self, x, y)
        end,
        end_closed_contour = function(self, x, y)
        end,
    }))
end


local function save(s, i, fmt)
    local file = io.open(string.format(fmt, i), "wb")
    render(accelerate(scene(s), wnd, vp), wnd, vp, file)
    stderr("%u", i)
    file:close()
end

if animate_outline then
    local segpaths = {}
    local xp, yp
    local xf = stroked_shape:get_xf()
    local splitter = {
        begin_contour = function(self, x0, y0)
        end,
        end_open_contour = function(self, x0, y0)
        end,
        end_closed_contour = function(self, x0, y0)
        end,
        linear_segment = function(self, x0, y0, x1, y1)
            local pd = path_data()
            pd:begin_contour(x0, y0)
            pd:linear_segment(x0, y0, x1, y1)
            pd:end_open_contour(x1, y1)
            segpaths[#segpaths+1] = { path(pd):transformed(xf) }
        end,
        quadratic_segment = function(self, x0, y0, x1, y1, x2, y2)
            local pd = path_data()
            pd:begin_contour(x0, y0)
            pd:quadratic_segment(x0, y0, x1, y1, x2, y2)
            pd:end_open_contour(x2, y2)
            segpaths[#segpaths+1] = { path(pd):transformed(xf) }
        end,
        rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
            local pd = path_data()
            pd:begin_contour(x0, y0)
            pd:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
            pd:end_open_contour(x2, y2)
            segpaths[#segpaths+1] = { path(pd):transformed(xf) }
        end,
        cubic_segment = function(self, x0, y0, x1, y1, x2, y2, x3, y3)
            local pd = path_data()
            pd:begin_contour(x0, y0)
            pd:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
            pd:end_open_contour(x3, y3)
            segpaths[#segpaths+1] = { path(pd):transformed(xf) }
        end,
    }
    stroked_shape:as_path_data():iterate(splitter)
    local total_length = strokers.arc_length(stroked_shape)
    local running_length = 0
    for i, p in ipairs(segpaths) do
        running_length = running_length + strokers.arc_length(p[1])
        p[2] = running_length
    end
    local co =  rgba(0,0,0,0.5)
    local frames = math.ceil(total_length/animate_outline_speed)
    local fmt = "outline-%06u.svg"
    for i = 1, frames do
        local end_length = i*total_length/frames
        local j = 1
        local s = {}
        if not no_background then
            s[#s+1] =
                fill(rect(0, 0, viewport_width, viewport_height), color.white)
        end
        s[#s+1] = fill(stroked_shape, stroke_color)
        local done = 0
        while segpaths[j] and segpaths[j][2] < end_length do
            s[#s+1] = fill(segpaths[j][1]:stroked(outline_width):joined(stroke_join.round), co)
            done = segpaths[j][2]
            j = j + 1
        end
        if segpaths[j] then
            local all = segpaths[j][2]/(outline_width)
            local missing = (end_length-done)/(outline_width)
            s[#s+1] = fill(segpaths[j][1]:stroked(outline_width):joined(stroke_join.round):capped(stroke_cap.round):dashed{0, missing, 2, all}, color.white)
            s[#s+1] = fill(segpaths[j][1]:stroked(outline_width):joined(stroke_join.round):capped(stroke_cap.round):dashed{missing, all}, co)
        end
        if generatrix then
            s[#s+1] = fill(test_path:stroked(outline_width):joined(stroke_join.round), generatrix_color)
        end
        save(s, i, fmt)
    end
end

if animate_fill then
    local pd
    local fills = {}
    local splitter = setmetatable({
        begin_contour = function(self, x0, y0)
            pd = path_data()
            pd:begin_contour(x0, y0)
        end,
        end_open_contour = function(self, x0, y0)
            pd:end_open_contour(x0, y0)
            fills[#fills+1] = path(pd)
            pd = path_data()
        end,
        end_closed_contour = function(self, x0, y0)
            pd:end_closed_contour(x0, y0)
            fills[#fills+1] = path(pd)
            pd = path_data()
        end
    }, {
        __index = function(self, method)
            local new = function(self, ...)
                pd[method](pd, ...) end
            self[method] = new
            return new
        end
    })
    stroked_shape:as_path_data():iterate(splitter)
    local fmt = "fill-%06u.svg"
    for i = 1, #fills do
        local s = {}
        if not no_background then
            s[#s+1] =
                fill(rect(0, 0, viewport_width, viewport_height), color.white)
        end
        for j = 1, i do
            local p = fills[j]
            s[#s+1] = fill(p, stroke_color)
        end
        s[#s+1] = fill(fills[i]:stroked(outline_width):joined(stroke_join.round):capped(stroke_cap.round), color.black)
        if generatrix then
            s[#s+1] = fill(test_path:stroked(outline_width):joined(stroke_join.round), generatrix_color)
        end
        save(s, i, fmt)
    end
    local s = {}
    if not no_background then
        s[#s+1] =
            fill(rect(0, 0, viewport_width, viewport_height), color.white)
    end
    s[#s+1] = fill(stroked_shape, stroke_color)
    local co =  rgba(0,0,0,0.5)
    for i = 1, #fills do
        s[#s+1] = fill(fills[i]:stroked(outline_width):joined(stroke_join.round):capped(stroke_cap.round), co)
    end
   -- s[#s+1] = fill(stroked_shape:stroked(outline_width):
    --    joined(stroke_join.round):capped(stroke_cap.round), color.black)
    if generatrix then
        s[#s+1] = fill(test_path:stroked(outline_width):joined(stroke_join.round), generatrix_color)
    end
    save(s , #fills+1, fmt)
end

-- add input path on top in blue
if generatrix then
    s[#s+1] = fill(test_path:stroked(outline_width):joined(stroke_join.round), generatrix_color)
end

local out = io.stdout
if outputname then
    out = assert(io.open(outputname, "w"))
end

-- render result to stdout
render(accelerate(scene(s), wnd, vp, rejected), wnd, vp, out, rejected)

if outputname then
    out:close()
end
