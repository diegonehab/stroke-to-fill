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
local mpvg = {
    path = require"strokers.mpvg.path",
    stroke = require"strokers.mpvg.stroke",
    filter = require"strokers.mpvg.filter",
    xform = require"strokers.mpvg.xform",
    flatten = require"strokers.mpvg.flatten"
}

local _M = { }

-- transforms path by xf
function make_input_path_f_xform(xf, forward)
    local px, py -- previous cursor
    local xformer = {}
    function xformer:begin_contour(x0, y0)
        px, py = xf:apply(x0, y0)
        forward:begin_contour(px, py)
    end
    function xformer:end_closed_contour(x0, y0)
        forward:end_closed_contour(px, py)
    end
    function xformer:end_open_contour(x0, y0)
        forward:end_open_contour(px, py)
    end
    function xformer:degenerate_segment(x0, y0, dx, dy, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       dx, dy = xf:apply(dx, dy, 0)
       forward:degenerate_segment(px, py, dx, dy, x1, y1)
       px, py = x1, y1
    end
    function xformer:linear_segment(x0, y0, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       forward:linear_segment(px, py, x1, y1)
       px, py = x1, y1
    end
    function xformer:quadratic_segment(x0, y0, x1, y1, x2, y2)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        forward:quadratic_segment(px, py, x1, y1, x2, y2)
        px, py = x2, y2
    end
    function xformer:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        x1, y1, w1 = xf:apply(x1, y1, w1)
        x2, y2 = xf:apply(x2, y2)
        forward:rational_quadratic_segment(px, py, x1, y1, w1, x2, y2)
        px, py = x2, y2
    end
    function xformer:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        x3, y3 = xf:apply(x3, y3)
        forward:cubic_segment(px, py, x1, y1, x2, y2, x3, y3)
        px, py = x3, y3
    end
    return xformer
end

local to_mpvg_cap = {
    [stroke_cap.butt] = mpvg.stroke.cap.butt,
    [stroke_cap.round] = mpvg.stroke.cap.round,
    [stroke_cap.triangle] = mpvg.stroke.cap.triangle,
    [stroke_cap.square] = mpvg.stroke.cap.square,
    [stroke_cap.fletching] = mpvg.stroke.cap.fletching
}

local to_mpvg_join = {
    [stroke_join.arcs] = mpvg.stroke.join.arcs,
    [stroke_join.miter_clip] = mpvg.stroke.join.miterclip,
    [stroke_join.miter_or_bevel] = mpvg.stroke.join.miter,
    [stroke_join.bevel] = mpvg.stroke.join.bevel,
    [stroke_join.round] = mpvg.stroke.join.round
}

local function do_nothing() end
local flagger_meta = {
    __index = function()
        return do_nothing
    end
}

local function make_closed_flags(flags)
    local flagger = setmetatable({}, flagger_meta)
    function flagger:end_closed_contour(x0, y0)
        flags[#flags+1] = true
    end
    function flagger:end_open_contour(x0, y0)
        flags[#flags+1] = false
    end
    return flagger
end

local function make_input_path_f_to_mpvg_path(flags, forward)
    local filter = {}
    local i = 1
    function filter:begin_contour(x0, y0)
        if flags[i] then
            forward:begin_closed_contour(nil, x0, y0)
        else
            forward:begin_open_contour(nil, x0, y0)
        end
        i = i + 1
    end
    function filter:end_closed_contour(x0, y0)
        forward:end_closed_contour(x0, y0)
    end
    function filter:end_open_contour(x0, y0)
        forward:end_open_contour(x0, y0)
    end
    function filter:degenerate_segment(x0, y0, dx, dy, x1, y1)
       forward:degenerate_segment(x0, y0, dx, dy, dx, dy, x1, y1)
    end
    function filter:linear_segment(x0, y0, x1, y1)
       forward:linear_segment(x0, y0, x1, y1)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        forward:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        forward:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
    end
    return filter
end

local function make_mpvg_path_f_to_input_path(forward)
    local filter = {}
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_contour(x0, y0)
    end
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_contour(x0, y0)
    end
    function filter:end_closed_contour(x0, y0)
        forward:end_closed_contour(x0, y0)
    end
    function filter:end_open_contour(x0, y0)
        forward:end_open_contour(x0, y0)
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
       forward:degenerate_segment(x0, y0, dx0, dy0, x1, y1)
    end
    function filter:linear_segment(x0, y0, x1, y1)
       forward:linear_segment(x0, y0, x1, y1)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        forward:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        forward:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
    end
    return filter
end

local function to_mpvg_xform(xf)
    return mpvg.xform.affine(table.unpack(xf:toxform()))
end

local function from_mpvg_xform(xf)
    return affinity(xf(1,1), xf(1,2), xf(1,3), xf(2,1), xf(2,2), xf(2,3))
end

function _M.stroke(input_shape, screen_xf, width, style)
    local input_path_data = assert(
        input_shape:as_path_data(input_shape:get_xf():transformed(screen_xf)))
    local flags = {}
    input_path_data:iterate(make_closed_flags(flags))
    local mpvg_input_path = mpvg.path.path():
        transform(to_mpvg_xform(input_shape:get_xf()))
    input_path_data:iterate(make_input_path_f_to_mpvg_path(flags,
        mpvg_input_path))
    local dash
    if #style:get_dashes() ~= 0 then
        dash = {}
        dash.array = {}
        for i,d in ipairs(style:get_dashes()) do
            dash.array[i] = d*width
        end
        dash.initial_phase = style:get_dash_offset()*width
        dash.phase_reset = style:get_resets_on_move()
    end
    local mpvg_stroked_path = mpvg_input_path:stroke{
        width = width,
        cap = to_mpvg_cap[style:get_initial_cap()],
        join = to_mpvg_join[style:get_join()],
        miter_limit = style:get_miter_limit()*0.5*width,
        dash = dash
    }:path(to_mpvg_xform(screen_xf))
    local output_path_data = path_data()
    mpvg_stroked_path:iterate(make_mpvg_path_f_to_input_path(output_path_data))
    return path(output_path_data):transformed(
        from_mpvg_xform(mpvg_stroked_path.xf))
end

function _M.flatten(input_shape, screen_xf, width, style)
    local input_path_data = input_shape:as_path_data(input_shape:get_xf():
        transformed(screen_xf))
    local flags = {}
    input_path_data:iterate(make_closed_flags(flags))
    local mpvg_input_path = mpvg.path.path():
        transform(to_mpvg_xform(input_shape:get_xf()))
    input_path_data:iterate(make_input_path_f_to_mpvg_path(flags,
        mpvg_input_path))
    local mpvg_flat_path = mpvg.path.path()
    mpvg_input_path:iterate(mpvg.flatten.flatten(
            mpvg.stroke.ATOL, mpvg.stroke.PTOL,
            to_mpvg_xform(screen_xf),
            mpvg.stroke.MAXDIV, mpvg_flat_path))
    local output_path_data = path_data()
    mpvg_flat_path:iterate(make_mpvg_path_f_to_input_path(output_path_data))
    return path(output_path_data):transformed(
        from_mpvg_xform(mpvg_flat_path.xf))
end

return _M
