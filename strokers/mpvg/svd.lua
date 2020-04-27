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

local abs = math.abs
local min = math.min
local max = math.max
local sqrt = math.sqrt

local util = require"strokers.mpvg.util"
local significant = util.significant
local negligible = util.negligible

-- our own implementation of the C hypot function
local function hypot(x, y)
    x, y = abs(x), abs(y)
    if negligible(x) and negligible(y) then return 0. end
    local t = min(x,y);
    x = max(x,y)
    t = t/x
    return x*sqrt(1.+t*t)
end

-- build an elementary projector from one of the
-- vectors in the nullspace of symmetric matrix
-- {{r, s}, {s,t}}, which is known to be rank defficient
-- returns the cos and the sin of the rotate
local function projector(r, s, t)
    if abs(r) > abs(t) then
        local h = hypot(r, s)
        if significant(h) then
            local inv_h = 1./h
            return s*inv_h, -r*inv_h
        else
            return 1., 0.
        end
    else
        local h = hypot(t, s)
        if significant(h) then
            local inv_h = 1./h
            return t*inv_h, -s*inv_h
        else
            return 1., 0.
        end
    end
end

-- returns the cos and sin of the rotate angle for U,
-- followed by the sx and sy of the scale S,
-- and omits the orthogonal matrix V
function _M.us(a, b, c, d)
    local ac = a*c
    local bd = b*d
    local a2 = a*a
    local b2 = b*b
    local c2 = c*c
    local d2 = d*d
    -- we have expressed things in a way that the
    -- discriminant is certainly non-negative even in the
    -- presence of numerical errors
    local D = hypot(.5*(a2+b2-c2-d2), ac+bd)
    local m = -.5*(a2+b2+c2+d2)
    local p = b2*c2+a2*d2-2.*ac*bd
    local el0, el1
    if m < 0. then
        el0 = -m+D
        el1 = p/el0
    else
        el0 = 0.
        el1 = 0.
    end
    -- so now we have the
    local s0 = sqrt(el0) -- largest singular value
    if significant(s0) then -- at least 1 singular value above threshold
        -- get projector from AAt - el0*I
        local cos, sin = projector(a2+b2-el0, ac+bd, c2+d2-el0)
        -- we will also use the
        local s1 = sqrt(el1) -- smallest singular value
        if significant(s1) then -- both singular values are above threshold
            return cos, sin, s0, s1
        else -- only largest is above threshold
            return cos, sin, s0, 0.
        end
    else  -- zero matrix
        return 1., 0., 0., 0.
    end
end

return _M
