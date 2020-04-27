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
local _M = { meta = {} }

local stroke_meta = _M.meta
stroke_meta.__index = {}
stroke_meta.name = "stroke"

local path = require"strokers.mpvg.delay.path"
local xform = require"strokers.mpvg.xform"
local filter = require"strokers.mpvg.filter"
local offset = require"strokers.mpvg.offset"
local flatten = require"strokers.mpvg.flatten"
local dash = require"strokers.mpvg.dash"

_M.ATOL = 3
_M.PTOL = .1
_M.MAXDIV = 10

_M.cap = {
    butt = "butt",
    round = "round",
    square = "square",
    triangle = "triangle",
    fletching = "fletching"
}

_M.join = {
    arcs = "arcs",
    miter = "miter",
    miterclip = "miterclip",
    round = "round",
    bevel = "bevel"
}

function copystyle(style)
    if type(style) == "table" then
        local copy = {}
        if style.dash then
            local dash = style.dash
            assert(type(dash) == "table", "invalid dash specification")
            if dash.array then
                local phase_reset = dash.phase_reset
                assert(not phase_reset or
                    type(phase_reset) == "boolean" or
                    type(phase_reset) == "number" and
                        (phase_reset == 0 or phase_reset == 1),
                    "invalid phase_reset")
                local initial_phase = dash.initial_phase
                assert(not initial_phase or
                    type(initial_phase) == "number", "invalid initial_phase")
                copy.dash = {
                    initial_phase = initial_phase or 0,
                    phase_reset = (phase_reset == 1) or (phase_reset == true)
                }
                local array = dash.array
                assert(type(array) == "table", "invalid dash array")
                local n = #array
                local s = 0
                for i = 1, n do
                    assert(type(array[i]) == "number", "invalid dash array")
                    assert(array[i] >= 0, "invalid dash array")
                    s = s + array[i]
                end
                if s > 0 then
                    copy.dash.array = {}
                    for i = 1, n do
                        copy.dash.array[i] = array[i]
                    end
                    -- double size if not even
                    if (n % 2) ~= 0 then
                        for i = 1, n do
                            copy.dash.array[i+n] = array[i]
                        end
                    end
                end
            end
        end
        assert(type(style.width) == "number" and style.width >= 0,
            "invalid stroke width")
        copy.width = style.width
        assert(not style.cap or _M.cap[style.cap], "invalid cap")
        copy.cap = style.cap or _M.cap.butt
        assert(not style.join or _M.join[style.join], "invalid join")
        copy.join = style.join or _M.join.bevel
        if style.join == _M.join.miter or style.join == _M.join.miterclip then
            assert(not style.miter_limit or
                type(style.miter_limit) == "number" and
                    style.miter_limit >= 0.5*style.width,
                "invalid miter_limit")
            copy.miter_limit = style.miter_limit or 2*style.width
        end
        return copy
    else
        assert(type(style) == "number", "invalid stroke style")
        assert(style >= 0, "invalid stroke width")
        return {
            width = style,
            cap = _M.cap.butt,
            join = _M.join.bevel,
         }
    end
end

function _M.stroke(shape, style, option)
    return setmetatable({
        type = "stroke",
        xf = xform.identity(),
        shape = shape,
        style = copystyle(style),
        option = option
    }, stroke_meta)
end

function _M.setmethods(index)
    function index.stroke(obj, style)
        return _M.stroke(obj, style)
    end

    function index.delayedstroke(obj, style)
        return _M.stroke(obj, style, "delayed")
    end
end

function stroke_meta.__index.path(stroke, post_xf)
    local xf = (post_xf or xform.identity()) * stroke.xf
    -- write orientation of degenerate segments
    local p = stroke.shape:path(xf)
    -- xform, flatten, dash, and save results
    local q = path.path()
    p:iterate(
        filter.close(false,
            filter.xform(p.xf,
                filter.delimit(
                    filter.monotonize(
                        flatten.flatten(_M.ATOL, _M.PTOL, xf, _M.MAXDIV,
                            dash.dash(stroke.style,
                                q)))))))
    q:orient()
    -- offset results of dashing process
    return offset.offset(q, stroke.style,
        _M.ATOL, path.path()):transform(stroke.xf)
end

xform.setmethods(stroke_meta.__index)
_M.setmethods(stroke_meta.__index)

return _M
