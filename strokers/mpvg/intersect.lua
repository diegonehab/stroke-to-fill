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

local abs = math.abs
local max = math.max
local sqrt = math.sqrt

local util = require"strokers.mpvg.util"

local dot2 = util.dot2
local norm2 = util.norm2
local sign = util.sign
local det2 = util.det2
local adj2 = util.adj2
local apply2 = util.apply2
local negligible = util.negligible
local significant = util.significant
local similar = util.similar

-- queries if segments intersect
-- returns false if parallel, even if coindicent
function _M.segmentsq(p0x, p0y, p1x, p1y, q0x, q0y, q1x, q1y)
    local dpx, dpy = p1x-p0x, p1y-p0y
    local dqx, dqy = q1x-q0x, q1y-q0y
    local dqpx, dqpy = q0x-p0x, q0y-p0y
    local a, b, c, d = adj2(dpx, -dqx, dpy, -dqy)
    local det = det2(a, b, c, d)
    local sdet = sign(det)
    local adet = abs(det)
    local ssdet, tsdet = apply2(a, b, c, d, sdet*dqpx, sdet*dqpy)
    return 0 <= ssdet and ssdet <= adet and 0 <= tsdet and tsdet <= adet
end

-- returns line intersection in homogeneous coordinates (x, y, w)
-- if lines are paralell, returns w = 0
-- if, in addition, lines are colinear, returns x = 0 and y = 0 as well
function _M.linesi(p0x, p0y, p1x, p1y, q0x, q0y, q1x, q1y)
    local dpx, dpy = p1x-p0x, p1y-p0y
    local dqx, dqy = q1x-q0x, q1y-q0y
    local dqpx, dqpy = q0x-p0x, q0y-p0y
    local a, b, c, d = adj2(dpx, -dqx, dpy, -dqy) -- inverse times det
    local det = det2(a, b, c, d)
    local s, t = apply2(a, b, c, d, dqpx, dqpy)
    return p0x*det+s*dpx, p0y*det+s*dpy, det
end

local function sq(s)
    return s*s
end

-- returns intersections between two circles
function _M.circlesi(cx0, cy0, r0, cx1, cy1, r1)
    local dcx, dcy = cx1-cx0, cy1-cy0
    local d = sqrt(norm2(dcx, dcy))
    if significant(d) then
        local ypn2 = 4*d*d*r0*r0 - sq(d*d + r0*r0 - r1*r1)
        if ypn2 > 0 then
            local inv_d = 1./d
            local yp, xp = .5*inv_d*sqrt(ypn2), 0.5*inv_d*(d*d+r0*r0-r1*r1)
            local s, c = inv_d*dcy, inv_d*dcx
            local i0x, i0y = apply2(c, -s, s, c, xp, yp)
            local i1x, i1y = apply2(c, -s, s, c, xp, -yp)
            return "intersect", i0x+cx0, i0y+cy0, i1x+cx0, i1y+cy0
        else
            return "disjoint"
        end
    else
        if similar(r0,r1) then return "coincident"
        else return "disjoint" end
    end
end

-- returns intersections between a circle and a line
-- assumes dx,dy has unit length
function _M.circlelinei(cx, cy, r, x, y, dx, dy)
    local nx, ny = -dy, dx
    local d = dot2(x-cx,y-cy, nx,ny)
    if abs(d) <= r then
        local s, c = ny, nx
        local h = sqrt(r*r-d*d)
        local i0x, i0y = apply2(c, -s, s, c, d, h)
        local i1x, i1y = apply2(c, -s, s, c, d, -h)
        return "intersect", i0x+cx, i0y+cy, i1x+cx, i1y+cy
    else
        return "disjoint"
    end
end

return _M
