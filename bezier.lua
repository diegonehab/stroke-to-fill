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

local bernstein = require"bernstein"

local lerp1 = bernstein.lerp1
local lerp2 = bernstein.lerp2
local lerp3 = bernstein.lerp3
local split1 = bernstein.split1
local split2 = bernstein.split2
local split3 = bernstein.split3
local power3 = bernstein.power3
local cut1 = bernstein.cut1
local cut2 = bernstein.cut2
local cut3 = bernstein.cut3

local sqrt = math.sqrt

local util = require"util"
local is_almost_zero = util.is_almost_zero
local det = util.det

function _M.split1(t, x0, y0, x1, y1)
    local u1 = split1(t, x0, x1)
    local v1 = split1(t, y0, y1)
    return u1, v1
end

function _M.split2(t, x0, y0, x1, y1, x2, y2)
    local u1, u2, u3 = split2(t, x0, x1, x2)
    local v1, v2, v3 = split2(t, y0, y1, y2)
    return u1, v1, u2, v2, u3, v3
end

function _M.split3(t, x0, y0, x1, y1, x2, y2, x3, y3)
    local u1, u2, u3, u4, u5 = split3(t, x0, x1, x2, x3)
    local v1, v2, v3, v4, v5 = split3(t, y0, y1, y2, y3)
    return u1, v1, u2, v2, u3, v3, u4, v4, u5, v5
end

function _M.split2r(t, x0, y0, w0, x1, y1, w1, x2, y2, w2)
    local u1, u2, u3 = split2(t, x0, x1, x2)
    local v1, v2, v3 = split2(t, y0, y1, y2)
    local r1, r2, r3 = split2(t, w0, w1, w2)
    return u1, v1, r1, u2, v2, r2, u3, v3, r3
end

-- cut linear segment
function _M.cut1(a, b, x0, y0, x1, y1)
    local u0, u1 = cut1(a, b, x0, x1)
    local v0, v1 = cut1(a, b, y0, y1)
    return u0, v0, u1, v1
end

-- cut quadratic segment
function _M.cut2(a, b, x0, y0, x1, y1, x2, y2)
    local u0, u1, u2 = cut2(a, b, x0, x1, x2)
    local v0, v1, v2 = cut2(a, b, y0, y1, y2)
    return u0, v0, u1, v1, u2, v2
end

-- cut cubic segment
function _M.cut3(a, b, x0, y0, x1, y1, x2, y2, x3, y3)
    local u0, u1, u2, u3 = cut3(a, b, x0, x1, x2, x3)
    local v0, v1, v2, v3 = cut3(a, b, y0, y1, y2, y3)
    return u0, v0, u1, v1, u2, v2, u3, v3
end

function _M.canonic2r(u0, v0, r0, u1, v1, r1, u2, v2, r2)
    assert(r0*r2 > 0 and not is_almost_zero(r0*r2), "unbounded curve")
    local ir0, ir2 = 1./r0, 1./r2
    local ir1 = sqrt(ir0*ir2)
    return u0*ir0, v0*ir0, u1*ir1, v1*ir1, r1*ir1, u2*ir2, v2*ir2
end

-- cut rational quadratic segment
function _M.cut2r(a, b, x0, y0, w0, x1, y1, w1, x2, y2, w2)
    local u0 = lerp2(a, a, x0, x1, x2)
    local v0 = lerp2(a, a, y0, y1, y2)
    local r0 = lerp2(a, a, w0, w1, w2)
    local u1 = lerp2(a, b, x0, x1, x2)
    local v1 = lerp2(a, b, y0, y1, y2)
    local r1 = lerp2(a, b, w0, w1, w2)
    local u2 = lerp2(b, b, x0, x1, x2)
    local v2 = lerp2(b, b, y0, y1, y2)
    local r2 = lerp2(b, b, w0, w1, w2)
    return u0, v0, r0, u1, v1, r1, u2, v2, r2
end

-- cut canonic rational quadratic segment and recanonize
function _M.cut2rc(a, b, x0, y0, x1, y1, w1, x2, y2)
    return _M.canonic2r(_M.cut2r(a, b, x0, y0, 1, x1, y1, w1, x2, y2, 1))
end

function _M.at1(t, x0, y0, x1, y1)
    local x = lerp1(t, x0, x1)
    local y = lerp1(t, y0, y1)
    return x, y
end

function _M.at2(t, x0, y0, x1, y1, x2, y2)
    local x = lerp2(t, t, x0, x1, x2)
    local y = lerp2(t, t, y0, y1, y2)
    return x, y
end

function _M.at3(t, x0, y0, x1, y1, x2, y2, x3, y3)
    local x = lerp3(t, t, t, x0, x1, x2, x3)
    local y = lerp3(t, t, t, y0, y1, y2, y3)
    return x, y
end

function _M.at2r(t, x0, y0, w0, x1, y1, w1, x2, y2, w2)
    local x = lerp2(t, t, x0, x1, x2, x3)
    local y = lerp2(t, t, y0, y1, y2, y3)
    local w = lerp2(t, t, w0, w1, w2, w3)
    return x, y, w
end

function _M.at2rc(t, x0, y0, x1, y1, w1, x2, y2)
    return _M.at2r(t, x0, y0, 1., x1, y1, w1, x2, y2, 1.)
end

local function crosspmatrix(x0, y0, x1, y1, x2, y2, x3, y3)
    local u0, u1, u2, u3 = power3(x0, x1, x2, x3)
    local v0, v1, v2, v3 = power3(y0, y1, y2, y3)
    local d1 = 0.
    local d2 = det(u2, u3, v2, v3)
    local d3 = -det(u1, u3, v1, v3)
    local d4 = det(u1, u2, v1, v2)
    return d1, d2, d3, d4
end

function _M.classify3(x0, y0, x1, y1, x2, y2, x3, y3)
    local d1, d2, d3, d4 = crosspmatrix(x0, y0, x1, y1, x2, y2, x3, y3)
    local d = 3.*d3*d3-4.*d2*d4
    if not is_almost_zero(d2) then
        if d > 0 then
            return "serpentine", d1, d2, d3, d4, d
        elseif d < 0 then
            return "loop", d1, d2, d3, d4, d
        else
            return "cusp with inflection at infinity", d1, d2, d3, d4, d
        end
    elseif not is_almost_zero(d3) then
        return "cusp with cusp at infinity", d1, d2, d3, d4, d
    elseif not is_almost_zero(d4) then
        return "quadratic", d1, d2, d3, d4, d
    else
        return "line or point", d1, d2, d3, d4, d
    end
end

return _M
