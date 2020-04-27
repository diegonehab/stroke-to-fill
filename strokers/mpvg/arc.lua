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

local vector = require"strokers.mpvg.vector"
local xform = require"strokers.mpvg.xform"
local svd = require"strokers.mpvg.svd"
local atan2 = math.atan2 or math.atan
local abs = math.abs

local util = require"strokers.mpvg.util"
local negligible = util.negligible
local similar = util.similar

local arc_meta = _M.meta
arc_meta.__index = {}
arc_meta.name = "arc"

local unpack = unpack or table.unpack

function _M.degenerate(x0, y0, rx, ry, rot_deg, fa, fs, x2, y2)
    local dx = x2-x0
    local dy = y2-y0
    return negligible(dx*dx+dy*dy) or negligible(rx) or negligible(ry)
end

local function elevate(x0, y0, x2, y2)
    return .5*(x0+x2), .5*(y0+y2), 1
end

function _M.torational(x0, y0, rx, ry, rot_deg, fa, fs, x2, y2)
    -- convert flags to booleans
    fa = fa == 1 or fa == '1' or fa == true
    fs = fs == 1 or fs == '1' or fs == true
    -- radii are assumed positive
    rx, ry = abs(rx), abs(ry)
    -- if radii are too small, we degenerate to line connecting endpoints
    if negligible(rx) or negligible(ry) then
        -- use degree elevation to represent line segment as quadratic
        return elevate(x0, y0, x2, y2)
    end
    local rot_rad = math.rad(rot_deg)
    local cos_rot = math.cos(rot_rad)
    local sin_rot = math.sin(rot_rad)
    local p0 = vector.vector(x0, y0)
    local p2 = vector.vector(x2, y2)
    local scale = xform.scale(1./rx, 1./ry)
    local rotate = xform.rotate(cos_rot, -sin_rot)
    -- we solve the problem in a new coordinate system
    -- where rx=ry=1 and rot_deg=0, then we move the solution
    -- back to the original coordinate system
    local q0 = scale*(rotate*p0)
    local q2 = scale*(rotate*p2)
    -- direction perpendicular to line connecting endpoints
    local perp = vector.perp(q2-q0)
    -- if transformed endpoints are too close, degenerate to
    -- line segment connecting endpoints
    local len2 = vector.len2(perp) -- perp doesn't change length
    if negligible(len2) then
        return elevate(x0, y0, x2, y2)
    end
    local mq = vector.lerp(q0, q2, .5) -- midpoint between transformed endpoints
    local radius -- circle radius
    local inv_radius -- its reciprocal
    local offset -- distance from midpoint to center
    -- center of circle, endpoint, and midpoint form a right triangle
    -- hypotenuse is the circle radius, which has length 1
    -- it connects the endpoint to the center
    -- the segment connecting the midpoint and endpoint is a cathetus
    -- the segment connecting midpoint and the center is the other
    local len = math.sqrt(len2)
    local inv_len = 1./len
    -- the length of the hypothenuse must be at least
    -- as large as the length of the catheti.
    if len2 > 4. then
        -- otherwise, we grow the circle isotropically until they are equal
        radius = .5*len
        inv_radius = 2.*inv_len
        -- in which case, the midpoint *is* the center
        offset = 0.
    else
        -- circle with radius 1 is large enough
        radius = 1.
        inv_radius = 1.
        -- length of the cathetus connecting the midpoint and the center
        offset = .5*math.sqrt(4.-len2)
    end
    -- there are two possible circles. flags decide which one
    local sign = ((fa ~= fs) and 1.) or -1. -- offset sign
    -- to find circle center in new coordinate system,
    -- simply offset midpoint in the perpendicular direction
    local cq = mq + (sign*offset*inv_len)*perp
    -- middle weight is the cosine of half the sector angle
    local w1 = math.abs(vector.dot(q0-cq, perp)*inv_len*inv_radius)
    -- if center was at the origin, this would be the
    -- intermediate control point for the rational quadratic
    local q1 = vector.vector((-sign*radius*inv_len)*perp, w1)
    -- so we translate it by the center
    q1 = xform.translate(cq[1], cq[2])*q1
    -- move control point back to original coordinate system
    scale = xform.scale(rx, ry)
    rotate = xform.rotate(cos_rot, sin_rot)
    local x1, y1 = unpack(rotate*(scale*q1), 1, 2)
    -- this selects the small arc. to select the large arc,
    -- negate all coordinates of intermediate control point
    -- TODO: this is not a good idea. We should instead
    -- split the arc into pieces that subintend at most 2*Pi/3 radians
    if fa then
        return -x1, -y1, -w1
    else
        return x1, y1, w1
    end
end

local function det(a, b, c, d, e, f, g, h, i)
    return -c*e*g + b*f*g + c*d*h - a*f*h - b*d*i + a*e*i
end

function _M.tosvg(x0, y0, x1, y1, w1, x2, y2)
    -- we start by computing the projective transformation that
    -- maps the unit circle to the ellipse described by the control points
    local s2 = 1.-w1*w1
    if negligible(s2) then
        if similar(x0, x1) and similar(x1, x2) and
           similar(y0, y1) and similar(y1, y2) then
            -- degenerate to a single point?
            return 0, 0, 0, 0, 0
        else
            error("not an ellipse (parabola?)")
        end
    end
    local s = ((s2 < 0.) and -1. or 1.) * math.sqrt(math.abs(s2))
    local a11 = 2.*x1-w1*(x0+x2)
    local a12 = s*(x2-x0)
    local a21 = 2.*y1-w1*(y0+y2)
    local a22 = s*(y2-y0)
    -- from the transformation, we extract the linear part and compute the SVD
    local ca, sa, sx, sy = svd.us(a11, a12, a21, a22)
    -- the sign of the middle weight gives the large/small angle flag
    local fa = (w1 < 0.) and 1 or 0
    -- the sign of the area of the control point triangle gives the orientation
    local fs = (det(x0, y0, 1., x1, y1, w1, x2, y2, 1.) > 0.) and 1 or 0
    -- the rotate and the scale from SVD give the angle and axes
    return sx/(2.*s2), sy/(2.*s2), math.deg(atan2(sa, ca)), fa, fs
end

return _M
