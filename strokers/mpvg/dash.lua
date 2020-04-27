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
local _M = {}

local bezier = require"strokers.mpvg.bezier"
local at1 = bezier.at1

local sqrt = math.sqrt
local abs = math.abs
local min = math.min

local filter = require"strokers.mpvg.filter"

local util = require"strokers.mpvg.util"
local similar = util.similar
local negligible = util.negligible
local significant = util.significant

local function dist(x0, y0, x1, y1)
    x0 = x0 - x1
    y0 = y0 - y1
    return sqrt(x0*x0 + y0*y0)
end

local function snapat1(used, len, inv_len, x0, y0, x1, y1)
    if used == len then return x1, y1
    else return at1(used*inv_len, x0, y0, x1, y1) end
end

function _M.lengther(forward)
    local filter = {}
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
    end
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        local len = dist(x0, y0, x1, y1)
        if significant(len) then
            forward:linear_segment_with_length(x0, y0, len, x1, y1)
        else
            local dx, dy = x1-x0, y1-y0
            forward:degenerate_segment(x0, y0, dx, dy, dx, dy, x1, y1)
        end
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    end
    function filter:end_open_contour(x0, y0, len)
        forward:end_open_contour(x0, y0, nil)
    end
    function filter:end_closed_contour(x0, y0, len)
        forward:end_closed_contour(x0, y0, nil)
    end
    function filter:begin_segment(s, t, x0, y0)
        forward:begin_segment(s, t, x0, y0)
    end
    function filter:end_segment(s, t, x0, y0)
        forward:end_segment(s, t, x0, y0)
    end
    return filter
end

function nodashes(forward)
    local filter = {}

    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
        forward:begin_dash(nil, x0, y0)
    end

    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
        forward:begin_dash(nil, x0, y0)
    end

    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    end

    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        forward:linear_segment_with_length(x0, y0, len, x1, y1)
    end

    function filter:begin_segment(s, t, x0, y0)
        forward:begin_segment(s, t, x0, y0)
    end

    function filter:end_segment(s, t, x1, y1)
        forward:end_segment(s, t, x1, y1)
    end

    function filter:end_closed_contour(x0, y0, len)
        forward:end_dash(x0, y0, nil)
        forward:end_closed_contour(x0, y0, nil)
    end

    function filter:end_open_contour(x0, y0, len)
        forward:end_dash(x0, y0, nil)
        forward:end_open_contour(x0, y0, nil)
    end

    return filter
end

function dashes(style, forward)
    -- compute initial phase, on state, and dash index
    local array = style.dash.array
    local pattern_length = 0.
    for i,dash in ipairs(array) do
        assert(dash >= 0., "invalid (negative) entry in dash array")
        pattern_length = pattern_length + dash
    end
    -- if only zeros, no dashing
    if pattern_length == 0. then
        return forward
    end
    local initial_phase = style.dash.initial_phase % pattern_length
    local initial_dash = 1
    local initial_on = true
    -- advance dashes until we are within the dash of the initial phase
    while initial_phase >= array[initial_dash] do
        initial_phase = initial_phase - array[initial_dash]
        initial_dash = initial_dash + 1
        if initial_dash > #array then initial_dash = 1 end
        initial_on = not initial_on
    end
    -- how much is left of the initial dash?
    local initial_needed = array[initial_dash] - initial_phase
    -- initial phase, on state, and dash index computed
    -- previous cursor
    local px, py
    -- current phase, on state, and dash index
    local current_needed = initial_needed
    local current_on = initial_on
    local current_dash = initial_dash

    local contour_begun = false
    local contour_closed

    local filter = {}

    function filter:begin_closed_contour(len, x0, y0)
        if style.dash.phase_reset then
            current_needed = initial_needed
            current_dash = initial_dash
            current_on = initial_on
        end
        if current_on then
            forward:begin_closed_contour(nil, x0, y0)
            forward:begin_dash(nil, x0, y0)
            px, py = x0, y0
        else
            px, py = nil, nil
        end
        contour_closed = current_on
        contour_begun = current_on
    end

    function filter:begin_open_contour(len, x0, y0)
        if style.dash.phase_reset then
            current_needed = initial_needed
            current_dash = initial_dash
            current_on = initial_on
        end
        if current_on then
            forward:begin_open_contour(nil, x0, y0)
            forward:begin_dash(nil, x0, y0)
            px, py = x0, y0
        else
            px, py = nil, nil
        end
        contour_closed = false
        contour_begun = current_on
    end

    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        if current_on then
            forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
            px, py = x1, y1
        end
    end

    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        -- consume entire length of line segment outputing dashes
        assert(significant(len), "degenerate segment not marked as such")
        local inv_len = 1./len
        local used = 0
        -- until segment is fully consumed
        while used < len do
            -- how much of the current_needed can we do with current segment?
            local now = min(len-used, current_needed)
            -- consume part of segment
            used = used + now
            -- snap to endpoint if too close
            if similar(used, len) then used = len end
            -- consume part of dash
            current_needed = current_needed - now
            -- snap to dash boundary if almost done
            if negligible(current_needed) then current_needed = 0 end
            -- if dash is on, output what we can
            if current_on then
                -- output part we can
                local u1, v1 = snapat1(used, len, inv_len, x0, y0, x1, y1)
                if significant(now) then
                    forward:linear_segment_with_length(px, py, now, u1, v1)
                else
                    local dx, dy = x1-x0, y1-y0
                    forward:degenerate_segment(px, py, dx, dy, dx, dy, u1, v1)
                end
                px, py = u1, v1
            end
            -- if dash (or space) is done, advance
            if current_needed <= 0. and (current_on or used < len) then
                current_dash = current_dash + 1
                if current_dash > #array then current_dash = 1 end
                current_needed = array[current_dash]
                current_on = not current_on
                -- if we completed a dash
                if current_on then
                    px, py = snapat1(used, len, inv_len, x0, y0, x1, y1)
                    if not contour_begun then
                        forward:begin_open_contour(nil, px, py)
                        contour_begun = true
                    end
                    forward:begin_dash(nil, px, py)
                else
                    forward:end_dash(px, py, nil)
                    px, py = nil, nil
                end
            end
        end
    end

    function filter:begin_segment(s, t, x0, y0)
        if current_on then
            forward:begin_segment(s, t, x0, y0)
        end
    end

    function filter:end_segment(s, t, x1, y1)
        if current_on then
            forward:end_segment(s, t, x1, y1)
        end
    end

    function filter:end_closed_contour(x0, y0, len)
        if current_on then
            forward:end_dash(nil)
        end
        if current_on and contour_closed then
            forward:end_closed_contour(px, py, nil)
        else
            forward:end_open_contour(px, py, nil)
        end
    end

    function filter:end_open_contour(x0, y0, len)
        if current_on then
            forward:end_dash(nil)
        end
        forward:end_open_contour(px, py, nil)
    end

    return filter
end

function _M.dash(style, forward)
    -- if there is no dash array, forward everything
    if type(style) ~= "table"
        or not style.dash or not style.dash.array or #style.dash.array == 0 then
        return _M.lengther(nodashes(forward))
    else -- otherwise, proceed with dashing
        return _M.lengther(dashes(style, forward))
    end
end

return _M
