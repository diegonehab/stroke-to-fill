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
local filter = require"strokers.mpvg.filter"

local split2 = bezier.split2
local split2r = bezier.split2r
local split3 = bezier.split3

local max = math.max
local abs = math.abs

local util = require"strokers.mpvg.util"
local significant = util.significant
local negligible = util.negligible
local dot2 = util.dot2
local norm2 = util.norm2
local det2 = util.det2

local function sq(x)
    return x*x
end

local function subdiv2(x0, y0, x1, y1, x2, y2, ctol2, ptol2, xf)
    local x10, y10 = xf:apply(x1-x0, y1-y0, 0)
    local x21, y21 = xf:apply(x2-x1, y2-y1, 0)
    local x20, y20 = xf:apply(x2-x0, y2-y0, 0)
    local n20 = norm2(x20, y20)
    return sq(dot2(x10, y10, x20, y20)) < ctol2*norm2(x10, y10)*n20
        or sq(dot2(x21, y21, x20, y20)) < ctol2*norm2(x21, y21)*n20
        or sq(det2(x20, y20, x10, y10)) > ptol2*n20
        or negligible(n20) and norm2(x10, y10) > ptol2
end

local function flatten2(x0, y0, x1, y1, x2, y2,
        ctol2, ptol2, xf, maxdiv, div, forward)
    if div <= maxdiv
        and subdiv2(x0,y0, x1,y1, x2,y2, ctol2, ptol2, xf) then
        local u1,v1, u2,v2, u3,v3 = split2(.5, x0,y0, x1,y1, x2,y2)
        flatten2(x0,y0, u1,v1, u2,v2, ctol2, ptol2, xf, maxdiv, div+1, forward)
        flatten2(u2,v2, u3,v3, x2,y2, ctol2, ptol2, xf, maxdiv, div+1, forward)
    else
        forward:linear_segment(x0,y0, x2,y2)
    end
end

local function subdiv2r(x0,y0,w0, x1,y1,w1, x2,y2,w2, ctol2, ptol2, xf)
    local x10, y10 = xf:apply(w2*det2(w0,x0,w1,x1), w2*det2(w0,y0,w1,y1), 0)
    local x21, y21 = xf:apply(w0*det2(w1,x1,w2,x2), w0*det2(w1,y1,w2,y2), 0)
    local x20, y20 = xf:apply(w1*det2(w0,x0,w2,x2), w1*det2(w0,y0,w2,y2), 0)
    local n20 = norm2(x20, y20)
    return sq(dot2(x10, y10, x20, y20)) < ctol2*norm2(x10, y10)*n20
        or sq(dot2(x21, y21, x20, y20)) < ctol2*norm2(x21, y21)*n20
        or sq(det2(x20, y20, x10, y10)) > ptol2*sq(w0*w1*w2)*n20
        or negligible(n20) and norm2(x10, y10) > ptol2*sq(w0*w1)
end

local function flatten2r(x0, y0, w0, x1, y1, w1, x2, y2, w2,
        ctol2, ptol2, xf, maxdiv, div, forward)
    if div <= maxdiv
        and subdiv2r(x0, y0, w0, x1, y1, w1, x2, y2, w2, ctol2, ptol2, xf) then
        local u1, v1, r1, u2, v2, r2, u3, v3, r3 =
            split2r(.5, x0, y0, w0, x1, y1, w1, x2, y2, w2)
        flatten2r(x0, y0, w0, u1, v1, r1, u2, v2, r2,
            ctol2, ptol2, xf, maxdiv, div+1, forward)
        flatten2r(u2, v2, r2, u3, v3, r3, x2, y2, w2,
            ctol2, ptol2, xf, maxdiv, div+1, forward)
    else
        assert(significant(w0) and significant(w2),
            "unbounded rational quadratic")
        local iw0 = 1./w0
        local iw2 = 1./w2
        forward:linear_segment(iw0*x0, iw0*y0, iw2*x2, iw2*y2)
    end
end

local function subdiv3(x0, y0, x1, y1, x2, y2, x3, y3, ctol2, ptol2, xf)
    local x30, y30 = xf:apply(x3-x0, y3-y0, 0)
    local x10, y10 = xf:apply(x1-x0, y1-y0, 0)
    local x32, y32 = xf:apply(x3-x2, y3-y2, 0)
    local n30 = norm2(x30, y30)
    return sq(dot2(x10, y10, x30, y30)) < ctol2*norm2(x10, y10)*n30
        or sq(dot2(x32, y32, x30, y30)) < ctol2*norm2(x32, y32)*n30
        or max(sq(det2(x30, y30, x10, y10)),
            sq(det2(x30, y30, x32, y32))) > ptol2*n30
        or negligible(n30) and max(norm2(x10, y10), norm2(x32, y32)) > ptol2
end

local function flatten3(x0, y0, x1, y1, x2, y2, x3, y3,
        ctol2, ptol2, xf, maxdiv, div, forward)
    if div <= maxdiv
        and subdiv3(x0, y0, x1, y1, x2, y2, x3, y3, ctol2, ptol2, xf) then
        local u1,v1, u2,v2, u3,v3, u4,v4, u5,v5 =
            split3(.5, x0,y0, x1,y1, x2,y2, x3,y3)
        flatten3(x0,y0, u1,v1, u2,v2, u3,v3,
            ctol2, ptol2, xf, maxdiv, div+1, forward)
        flatten3(u3,v3, u4,v4, u5,v5, x3,y3,
            ctol2, ptol2, xf, maxdiv, div+1, forward)
    else
        forward:linear_segment(x0, y0, x3, y3)
    end
end

-- flatten the entire path
-- mark begining and end of each original segment with the curvature
-- in addition, in the begining and end of each contour, add a degenerate
-- segment just to make sure we capture the correct tangent direction
function _M.flatten(atol, ptol, xf, maxdiv, forward)
    local ctol2 = sq(math.cos(math.rad(atol)))
    local ptol2 = sq(ptol)
    local s, t
    local fx, fy
    local tx, ty
    local filter = {}
    function filter:begin_closed_contour(len, x0, y0)
        forward:begin_closed_contour(nil, x0, y0)
    end
    function filter:begin_open_contour(len, x0, y0)
        forward:begin_open_contour(nil, x0, y0)
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        forward:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        forward:linear_segment(x0, y0, x1, y1)
    end
    function filter:linear_segment_with_length(x0, y0, x1, y1)
        forward:linear_segment_with_length(x0, y0, len, x1, y1)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        flatten2(x0, y0, x1, y1, x2, y2, ctol2, ptol2, xf, maxdiv, 1, forward)
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        flatten2r(x0, y0, 1., x1, y1, w1, x2, y2, 1.,
            ctol2, ptol2, xf, maxdiv, 1, forward)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        flatten3(x0, y0, x1, y1, x2, y2, x3, y3,
            ctol2, ptol2, xf, maxdiv, 1, forward)
    end
    function filter:end_open_contour(x0, y0, len)
        forward:end_open_contour(x0, y0, nil)
    end
    function filter:end_closed_contour(x0, y0, len)
        forward:end_closed_contour(x0, y0, nil)
    end
    function filter:begin_segment(s, t, x, y)
        forward:begin_segment(s, t, x, y)
    end
    function filter:end_segment(s, t, x, y)
        forward:end_segment(s, t, x, y)
    end

    return filter
end

return _M
