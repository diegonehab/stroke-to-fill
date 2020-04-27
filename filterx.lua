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

local bezier = require"bezier"
local cut2 = bezier.cut2
local cut2rc = bezier.cut2rc
local cut3 = bezier.cut3
local classify3 = bezier.classify3
local quadratic = require"quadratic"
local solvequadratic = quadratic.quadratic
local sort = table.sort
local util = require"util"
local sgn = util.sgn

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

-- transforms path by xf
function _M.xform(xf, forward)
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

local function filterroot(ts, t, s)
    local ss = sgn(s)
    if 0. < ss*t and ss*t < ss*s then ts[#ts+1] = t/s end
end

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

-- splits non-monotonic segments into their monotonic components
function _M.monotonize(forward)
    local filter = {}
    function filter:begin_contour(x0, y0)
        forward:begin_contour(x0, y0)
    end
    function filter:end_closed_contour(x0, y0)
        forward:end_closed_contour(x0, y0)
    end
    function filter:end_open_contour(x0, y0)
        forward:end_open_contour(x0, y0)
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
    return filter
end

return _M
