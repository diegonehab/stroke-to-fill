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

-- TODO: change the names of all internal tables with
-- filter methods to "filter"
local bezier = require"strokers.mpvg.bezier"
local cut2 = bezier.cut2
local cut2rc = bezier.cut2rc
local cut3 = bezier.cut3
local classify3 = bezier.classify3

local util = require"strokers.mpvg.util"
local distinct = util.distinct
local significant = util.significant
local negligible = util.negligible

local unpack = unpack or table.unpack
local sqrt = math.sqrt
local quadratic = require"strokers.mpvg.quadratic"
local solvequadratic = quadratic.quadratic
local sort = table.sort

local initialcurvature = bezier.initialcurvature
local finalcurvature = bezier.finalcurvature
local initialtangent3 = bezier.initialtangent3
local initialtangent2 = bezier.initialtangent2
local initialtangent2rc = bezier.initialtangent2rc
local finaltangent3 = bezier.finaltangent3
local finaltangent2 = bezier.finaltangent2
local finaltangent2rc = bezier.finaltangent2rc

local function uniq(t)
    sort(t)
    local n = #t
    local i, j = 1, 2
    while j <= n do
        if t[i] == t[j] then
            j = j + 1
        else
            i = i + 1
            t[i] = t[j]
            j = j + 1
        end
    end
    i = i + 1
    while i <= n do
        t[i] = nil
        i = i + 1
    end
    return t
end

-- prints whatever is forwarded
function _M.spy(name, forward)
    return setmetatable({}, {
        __index = function(self, method)
            local new = function(self, ...)
                io.stderr:write(name, ":", method, "(")
                local n = select("#", ...)
                for i = 1, n-1 do
                    io.stderr:write(tostring(select(i, ...)), ",")
                end
                if n > 0 then
                    io.stderr:write(tostring(select(n, ...)))
                end
                io.stderr:write(")\n")
                if forward and forward[method] then
                    forward[method](forward, ...)
                end
            end
            self[method] = new
            return new
        end
    })
end

-- turns all open contours into closed contours
function _M.close(all, forward)
    local filter = {}
    local fx, fy -- first contour cursor
    local px, py -- previous cursor

    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
        fx, fy, px, py = x0, y0, x0, y0
    end

    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
        fx, fy, px, py = x0, y0, x0, y0
    end

    function filter:begin_segment(s, t, x0, y0)
    end

    function filter:end_segment(s, t, x0, y0)
    end

    if all then filter.begin_open_contour = filter.begin_closed_contour end

    function filter:end_closed_contour(x0, y0, len)
        if fx then -- and fy and px and py
            if px ~= fx or py ~= fy then
                forward:linear_segment(px, py, fx, fy)
                x0, y0 = fx, fy
            end
        end
        forward:end_closed_contour(x0, y0, len)
        fx, fy, px, py = nil, nil, nil, nil
    end

    function filter:end_open_contour(x0, y0, len)
        forward:end_open_contour(x0, y0, len)
        fx, fy, px, py = nil, nil, nil, nil
    end

    if all then filter.end_open_contour = filter.end_closed_contour end

    function filter:linear_segment(x0, y0, x1, y1)
        forward:linear_segment(px, py, x1, y1)
        px, py = x1, y1
    end

    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        forward:linear_segment_with_length(px, py, len, x1, y1)
        px, py = x1, y1
    end

    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(px, py, dx0, dy0, dx1, dy1, x1, y1)
        px, py = x1, y1
    end

    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        forward:quadratic_segment(px, py, x1, y1, x2, y2)
        px, py = x2, y2
    end

    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        forward:rational_quadratic_segment(px, py, x1, y1, w1, x2, y2)
        px, py = x2, y2
    end

    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        forward:cubic_segment(px, py, x1, y1, x2, y2, x3, y3)
        px, py = x3, y3
    end

    return filter
end

local function dist(x0, y0, x1, y1)
    x0 = x0 - x1
    y0 = y0 - y1
    return sqrt(x0*x0 + y0*y0)
end

-- transforms path by xf
function _M.xform(xf, forward)
    local px, py -- previous cursor
    local xformer = {}
    function xformer:begin_closed_contour(len, x0, y0)
        px, py = xf:apply(x0, y0)
        forward:begin_closed_contour(len, px, py)
    end
    function xformer:begin_open_contour(len, x0, y0)
        px, py = xf:apply(x0, y0)
        forward:begin_open_contour(len, px, py)
    end
    function xformer:end_closed_contour(x0, y0, len)
        forward:end_closed_contour(px, py, len)
    end
    function xformer:end_open_contour(x0, y0, len)
        forward:end_open_contour(px, py, len)
    end
    function xformer:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       dx0, dy0 = xf:apply(dx0, dy0, 0)
       dx1, dy1 = xf:apply(dx1, dy1, 0)
       forward:degenerate_segment(px, py, dx0, dy0, dx1, dy1, x1, y1)
       px, py = x1, y1
    end
    function xformer:linear_segment(x0, y0, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       forward:linear_segment(px, py, x1, y1)
       px, py = x1, y1
    end
    function xformer:linear_segment_with_length(x0, y0, len, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       forward:linear_segment_with_length(px, py, dist(px, py, x1, y1), x1, y1)
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

-- transforms path by xf inplace
function _M.xforminplace(xf)
    local xformer = {}
    function xformer:begin_closed_contour(len, x0, y0)
        x0, y0 = xf:apply(x0, y0)
        return nil, len, x0, y0
    end
    function xformer:begin_open_contour(len, x0, y0)
        x0, y0 = xf:apply(x0, y0)
        return nil, len, x0, y0
    end
    function xformer:end_closed_contour(x0, y0, len)
        -- x0, y0 already xformed by previous command
        return nil, x0, y0, len
    end
    function xformer:end_open_contour(x0, y0, len)
        -- x0, y0 already xformed by previous command
        return nil, x0, y0, len
    end
    function xformer:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       dx0, dy0 = xf:apply(dx0, dy0, 0)
       dx1, dy1 = xf:apply(dx1, dy1, 0)
        -- x0, y0 already xformed by previous command
       return nil, x0, y0, dx0, dy0, dx1, dy1, x1, y1
    end
    function xformer:linear_segment(x0, y0, x1, y1)
       x1, y1 = xf:apply(x1, y1)
        -- x0, y0 already xformed by previous command
       return nil, x0, y0, x1, y1
    end
    function xformer:quadratic_segment(x0, y0, x1, y1, x2, y2)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        -- x0, y0 already xformed by previous command
        return nil, x0, y0, x1, y1, x2, y2
    end
    function xformer:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        x1, y1, w1 = xf:apply(x1, y1, w1)
        x2, y2 = xf:apply(x2, y2)
        -- x0, y0 already xformed by previous command
        return nil, x0, y0, x1, y1, w1, x2, y2
    end
    function xformer:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        x3, y3 = xf:apply(x3, y3)
        -- x0, y0 already xformed by previous command
        return nil, x0, y0, x1, y1, x2, y2, x3, y3
    end
    return xformer
end

local function det2(a, b, c, d)
    return a*d-b*c
end

local function sign(v)
    if v < 0. then return -1.
    elseif v > 0. then return 1.
    else return 0. end
end

local function filterroot(ts, t, s)
    local ss = sign(s)
    if 0. < ss*t and ss*t < ss*s then ts[#ts+1] = t/s end
end

local function intersect(x0, y0, x1, y1, x2, y2, x3, y3)
    -- translate x0,y0 to 0,0
    x1, y1 = x1-x0, y1-y0
    x2, y2 = x2-x0, y2-y0
    x3, y3 = x3-x0, y3-y0
    -- find intersection of tangents
    local x10, y10 = x1, y1
    local x32, y32 = x3-x2, y3-y2
    local x30, y30 = x3, y3
    local xp, yp
    -- if tangent at starting point is zero
    if negligible(x10) and negligible(y10) then
        -- if tangent at end point is zero
        if negligible(x32) and negligible(y32) then
            -- cubic degenerates to line
            error("degenerate to line!")
        else
            -- use other control point of other tangent
            return x2+x0, y2+y0
        end
    -- if tangent at end point zero
    elseif negligible(x32) and negligible(y32) then
        -- use other control point of other tangent
        return x1+x0, y1+y0
    -- actually compute intersection
    else
        local t = det2(x30,x32,y30,y32)/det2(x10,x32,y10,y32)
        return t*x10+x0, t*y10+y0
    end
end

-- splits non-monotonic segments into their monotonic components
function _M.monotonize(forward)
    local filter = {}
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
    end
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
    end
    function filter:end_closed_contour(x0, y0, len)
        forward:end_closed_contour(x0, y0, nil)
    end
    function filter:end_open_contour(x0, y0, len)
        forward:end_open_contour(x0, y0, nil)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        forward:linear_segment(x0, y0, x1, y1)
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        -- go on with quadratic
        local ts = { 0. }
        -- monotonize in x
        local t = x0 - x1
        local s = x0 - 2.*x1 + x2
        filterroot(ts, t, s)
        -- monotonize in y
        t = y0 - y1
        s = y0 - 2.*y1 + y2
        filterroot(ts, t, s)
        ts[#ts+1] = 1.
        uniq(ts)
        local up, vp = x0, y0
        -- I am pretty sure the cut2s(a, 1, p0, p1, p2) returns p2
        -- as the last control point
        for i = 2, #ts do
            local u0, v0, u1, v1, u2, v2 =
                cut2(ts[i-1], ts[i], x0, y0, x1, y1, x2, y2)
            forward:quadratic_segment(up, vp, u1, v1, u2, v2)
            up, vp = u2, v2
        end
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        -- go on with rational quadratic
        local ts = { 0. }
        -- monotonize in x
        local a = (x0 - x2) * (w1 - 1.)
        local b = x0 - x2 + 2.*(x1 - w1*x0)
        local c = w1*x0 - x1
        local n, t1, s1, t2, s2 = solvequadratic(a, b, c)
        if n > 0 then filterroot(ts, t1, s1) end
        if n > 1 then filterroot(ts, t2, s2) end
        -- monotonize in y
        a = (y0 - y2) * (w1 - 1.)
        b = y0 - y2 + 2.*(y1 - w1*y0)
        c = w1*y0 - y1
        n, t1, s1, t2, s2 = solvequadratic(a, b, c)
        if n > 0 then filterroot(ts, t1, s1) end
        if n > 1 then filterroot(ts, t2, s2) end
        ts[#ts+1] = 1.
        uniq(ts)
        -- I am pretty sure the cutr2s(a, 1, p0, p1w1, p2) returns p2
        -- as the last control point
        local up, vp = x0, y0
        for i = 2, #ts do
            local u0, v0, u1, v1, r1, u2, v2 =
                cut2rc(ts[i-1], ts[i], x0, y0, x1, y1, w1, x2, y2)
            forward:rational_quadratic_segment(up, vp, u1, v1, r1, u2, v2)
            up, vp = u2, v2
        end
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        local class, d1, d2, d3, d4, d =
            classify3(x0, y0, x1, y1, x2, y2, x3, y3)
        -- go on with cubic
        local ts = { 0. }
        -- monotonize in x
        local a = -x0 + 3.*(x1 - x2) + x3
        local b = 2.*(x0 - 2.*x1 + x2)
        local c = x1 - x0
        local n, t1, s1, t2, s2 = solvequadratic(a, b, c)
        if n > 0 then filterroot(ts, t1, s1) end
        if n > 1 then filterroot(ts, t2, s2) end
        -- monotonize in y
        a = -y0 + 3.*(y1 - y2) + y3
        b = 2.*(y0 - 2.*y1 + y2)
        c = y1 - y0
        n, t1, s1, t2, s2 = solvequadratic(a, b, c)
        if n > 0 then filterroot(ts, t1, s1) end
        if n > 1 then filterroot(ts, t2, s2) end
        -- break at double point
        if class == "loop" then
            n, t1, s1, t2, s2
                = solvequadratic(d2*d2, -d2*d3, d3*d3-d2*d4, -.25*d2*d2*d)
        -- break at inflections
        else
            n, t1, s1, t2, s2 = solvequadratic(-3.*d2, 3.*d3, -d4, .25*3.*d)
        end
        if n > 0 then filterroot(ts, t1, s1) end
        if n > 1 then filterroot(ts, t2, s2) end
        ts[#ts+1] = 1.
        uniq(ts)
        local up, vp = x0, y0
        for i = 2, #ts do
            local u0, v0, u1, v1, u2, v2, u3, v3 =
                cut3(ts[i-1], ts[i], x0, y0, x1, y1, x2, y2, x3, y3)
            forward:cubic_segment(up, vp, u1, v1, u2, v2, u3, v3)
            up, vp = u3, v3
        end
    end
    function filter:begin_segment(s, t, x, y)
        forward:begin_segment(s, t, x, y)
    end
    function filter:end_segment(s, t, x, y)
        forward:end_segment(s, t, x, y)
    end

    return filter
end

-- downgrade degenerate segments
-- assumes monotonic segments
function _M.downgrade(forward)
    local filter = {}
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
    end
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        if distinct(x0, x1) or distinct(y0, y1) then
            forward:linear_segment(x0, y0, x1, y1)
        else
            local dx, dy = x1-x0, y1-y0
            forward:degenerate_segment(x0, y0, dx, dy, dx, dy, x1, y1)
        end
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        local a = det2(x0,x1,y0,y1)-det2(x0,x2,y0,y2)+det2(x1,x2,y1,y2)
        -- not degenerate to line
        if significant(a) then
            forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
        -- not degenerate to point
        elseif distinct(x0, x2) or distinct(y0, y2) then
            forward:linear_segment(x0, y0, x2, y2)
        -- degenerate to point
        else
            local dx, dy = x2-x0, y2-y0
            forward:degenerate_segment(x0,y0, dx,dy, dx,dy, x2,y2)
        end
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        local a = det2(x0,x1,y0,y1)-w1*det2(x0,x2,y0,y2)+det2(x1,x2,y1,y2)
        -- not degenerate to line
        if significant(a) then
            if distinct(w1, 1) then
                forward:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
            else
                forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
            end
        -- not degenerate to point
        elseif distinct(x0, x2) or distinct(y0, y2) then
            forward:linear_segment(x0, y0, x2, y2)
        else
            local dx, dy = x2-x0, y2-y0
            forward:degenerate_segment(x0,y0, dx,dy, dx,dy, x2,y2)
        end
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        local class = classify3(x0, y0, x1, y1, x2, y2, x3, y3)
        if class == "quadratic" then
            local u1, v1 = intersect(x0, y0, x1, y1, x2, y2, x3, y3)
            return forward:quadratic_segment(x0, y0, u1, v1, x3, y3)
        elseif class == "line or point" then
            if distinct(x0, y0) or distinct(x3, y3) then
                return forward:linear_segment(x0, y0, x3, y3)
            else
                local dx, dy = x3-x0, y3-y0
                forward:degenerate_segment(x0,y0, dx,dy, dx,dy, x3,y3)
            end
        else
            forward:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        end
    end
    function filter:end_closed_contour(x0, y0, len)
        forward:end_closed_contour(x0, y0, nil)
    end
    function filter:end_open_contour(x0, y0, len)
        forward:end_open_contour(x0, y0, nil)
    end
    return filter
end

function _M.initialtangent()
    local tx, ty = 0, 0
    local filter = {}
    function filter:get()
        return tx, ty
    end
    function filter:begin_open_contour(len, x0, y0)
        -- clear orientation
        tx, ty = 0, 0
    end
    function filter:begin_closed_contour(len, x0, y0)
        tx, ty = 0, 0
    end
    function filter:end_closed_contour(x0, y0, len)
    end
    function filter:end_open_contour(x0, y0, len)
    end
    function filter:begin_segment(s, t, x0, y0)
    end
    function filter:end_segment(s, t, x0, y0)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        tx, ty = x1-x0, y1-y0
        return true -- abort
    end
    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        tx, ty = x1-x0, y1-y0
        return true -- abort
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        -- ignore degenerates
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        tx, ty = bezier.initialtangent2(x0, y0, x1, y1, x2, y2)
        return true -- abort
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        tx, ty = bezier.initialtangent2rc(x0, y0, x1, y1, w1, x2, y2)
        return true -- abort
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        tx, ty = bezier.initialtangent3(x0, y0, x1, y1, x2, y2, x3, y3)
        return true -- abort
    end
    function filter:begin_segment(s, t, x, y)
    end
    function filter:end_segment(s, t, x, y)
    end
    function filter:begin_dash(len, x0, y0)
    end
    function filter:end_dash(x0, y0, len)
    end
    return filter
end

function _M.delimit(forward)
    local s, t
    local fx, fy
    local tx, ty
    local filter = {}
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
        fx, fy = nil, nil
        s, t = nil, nil
        tx, ty = nil, nil
    end
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
        fx, fy = nil, nil
        s, t = nil, nil
        tx, ty = nil, nil
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        if s then forward:end_segment(s, t, fx, fy) end
        forward:begin_segment(0, 1, x0, y0)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        s, t = 0, 1
        fx, fy = x1, y1
        tx, ty = nil, nil
    end
    function filter:linear_segment(x0, y0, x1, y1)
        if s then forward:end_segment(s, t, fx, fy) end
        forward:begin_segment(0, 1, x0, y0)
        forward:linear_segment(x0, y0, x1, y1)
        s, t = 0, 1
        fx, fy = x1, y1
        tx, ty = nil, nil
    end
    function filter:linear_segment_with_length(x0, y0, x1, y1)
        if s then forward:end_segment(s, t, fx, fy) end
        forward:begin_segment(0, 1, x0, y0)
        forward:linear_segment_with_length(x0, y0, len, x1, y1)
        s, t = 0, 1
        fx, fy = x1, y1
        tx, ty = nil, nil
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        if s then forward:end_segment(s, t, fx, fy) end
        local s0, t0 = initialcurvature(2, x0, y0, 1, x1, y1, 1, x2, y2, 1)
        forward:begin_segment(s0, t0, x0, y0)
        if not s then
            local tx0, ty0 = initialtangent2(x0, y0, x1, y1, x2, y2)
            forward:degenerate_segment(x0, y0, tx0, ty0, tx0, ty0, x0, y0)
        end
        forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
        s, t = finalcurvature(2, x0, y0, 1, x1, y1, 1, x2, y2, 1)
        tx, ty = finaltangent2(x0, y0, x1, y1, x2, y2)
        fx, fy = x2 ,y2
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        if s then forward:end_segment(s, t, fx, fy) end
        local s0, t0 = initialcurvature(2, x0, y0, 1, x1, y1, w1, x2, y2, 1)
        forward:begin_segment(s0, t0, x0, y0)
        if not s then
            local tx0, ty0 = initialtangent2rc(x0, y0, x1, y1, w1, x2, y2)
            forward:degenerate_segment(x0, y0, tx0, ty0, tx0, ty0, x0, y0)
        end
        forward:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        s, t = finalcurvature(2, x0, y0, 1, x1, y1, w1, x2, y2, 1)
        tx, ty = finaltangent2rc(x0, y0, x1, y1, w1, x2, y2)
        fx, fy = x2, y2
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        if s then forward:end_segment(s, t, fx, fy) end
        local s0, t0 = initialcurvature(3, x0, y0, 1, x1, y1, 1, x2, y2, 1)
        forward:begin_segment(s0, t0, x0, y0)
        if not s then
            local tx0, ty0 = initialtangent3(x0, y0, x1, y1, x2, y2, x3, y3)
            forward:degenerate_segment(x0, y0, tx0, ty0, tx0, ty0, x0, y0)
        end
        forward:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        s, t = finalcurvature(3, x0, y0, 1, x1, y1, 1, x2, y2, 1)
        tx, ty = finaltangent3(x0, y0, x1, y1, x2, y2, x3, y3)
        fx, fy = x3, y3
    end
    function filter:end_open_contour(x0, y0, len)
        if s then
            if tx then
                forward:degenerate_segment(fx, fy, tx, ty, tx, ty, fx, fy)
            end
            forward:end_segment(s, t, fx, fy)
        end
        forward:end_open_contour(x0, y0, nil)
    end
    function filter:end_closed_contour(x0, y0, len)
        if s then
            if tx then
                forward:degenerate_segment(fx, fy, tx, ty, tx, ty, fx, fy)
            end
            forward:end_segment(s, t, fx, fy)
        end
        forward:end_closed_contour(x0, y0, nil)
    end
    return filter
end




return _M
