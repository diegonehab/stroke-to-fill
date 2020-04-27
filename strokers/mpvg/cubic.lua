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

local atan2 = math.atan2 or math.atan

-- From "How to Solve a Cubic Equation, Part 5: Back to Numerics",
-- Jim Blinn's Corner, May/Jun 2007
local function cubicroot(x)
    local value
    local neg = false
    local absx = x
    if x < 0. then
        absx = -x
        neg = true
    end
    if absx ~= 0. then
        value = math.exp(math.log(absx)/3.)
    else
        value = 0.
    end
    if neg then return -value
    else return value end
end

local function singleroothelper(tilde_a, bar_c, bar_d, D)
    local sqrt_D = math.sqrt(-D)
    local p
    if bar_d < 0. then
        p = cubicroot(.5*(-bar_d+math.abs(tilde_a)*sqrt_D))
    else
        p = cubicroot(.5*(-bar_d-math.abs(tilde_a)*sqrt_D))
    end
    return p - bar_c/p
end

local sqrt_3 = math.sqrt(3.)

local function tripleroothelper(bar_c, bar_d, sqrt_D, s, r)
    local theta = math.abs(atan2(s*sqrt_D, -bar_d))/3.
    local cos_theta = math.cos(theta)
    local sin_theta = math.sin(theta)
    local sqrt_bar_c = math.sqrt(math.abs(bar_c))
    local x1 = 2.*sqrt_bar_c*cos_theta
    local x3 = -sqrt_bar_c*(cos_theta+sqrt_3*sin_theta)
    if x1+x3 > 2.*r then
        return x1, x3
    else
        return x3, x1
    end
end

-- returns n, t1, s1, .. tn,sn
-- where n is the number of real roots of a*x^3 + b*x^2 + c*x + d == 0
-- and each root i in 1..n is given by ti/si
function _M.cubic(a, b, c, d)
    b = b*(1./3.)
    c = c*(1./3.)
    local d1 = a*c-b*b
    local d2 = a*d-b*c
    local d3 = b*d-c*c
    local D = 4.*d1*d3-d2*d2
    if D <= 0. then
        -- triple root
        if d1 == 0. and d2 == 0. and d3 == 0. then
            if math.abs(a) > math.abs(b) then
                if math.abs(a) > math.abs(c) then
                    return 3, -b, a, -b, a, -b, a
                else
                    return 3, -d, c, -d, c, -d, c
                end
            elseif math.abs(b) > math.abs(c) then
                return 3, -c, b, -c, b, -c, b
            else
                return 3, -d, c, -d, c, -d, c
            end
        end
        if b*b*b*d >= a*c*c*c then
            local bar_c = d1
            local bar_d = -2.*b*d1+a*d2
            local r1 = singleroothelper(a, bar_c, bar_d, D)
            -- double root
            if D == 0. then
                return 3, r1-b, a, -0.5*r1-b, a, -0.5*r1-b, a
            else
                return 1, r1-b, a
            end
        else
            local bar_c = d3
            local bar_d = -d*d2+2.*c*d3
            local r1 = singleroothelper(d, bar_c, bar_d, D)
            -- double root
            if (D == 0.) then
                return 3, -d, r1+c, -d, -0.5*r1+b, -d, -0.5*r1+b
            else
                return 1, -d, r1+c
            end
        end
    else
        local sqrt_D = math.sqrt(D)
        local bar_c_a = d1
        local bar_d_a = -2.*b*d1+a*d2
        local xl, o = tripleroothelper(bar_c_a, bar_d_a, sqrt_D, a, b)
        xl = xl-b
        local wl = a
        local bar_c_d = d3
        local bar_d_d = -d*d2+2.*c*d3
        local o, xs = tripleroothelper(bar_c_d, bar_d_d, sqrt_D, d, c)
        local ws = xs+c
        xs = -d
        local e = wl*ws
        local f = -xl*ws-wl*xs
        local g = xl*xs
        local xm = c*f-b*g
        local wm = -b*f+c*e
        return 3, xs, ws, xm, wm, xl, wl
    end
end

--[[
function printroots(a, b, c, d)
    local r = {_M.cubic(a, b, c, d)}
    io.stderr:write("  ")
    for i = 2, r[1]*2+1, 2 do
        io.stderr:write(string.format("%g ", r[i]/r[i+1]))
    end
    io.stderr:write("\n")
    for i = 2, r[1]*2+1, 2 do
        local rt = r[i]/r[i+1]
        io.stderr:write(string.format("  -> %g\n", a*rt*rt*rt+b*rt*rt+c*rt+d))
    end
end

function test3(a)
    io.stderr:write(string.format("%g %g %g\n", a, a, a))
    printroots(1., -3.*a, 3.*a*a, -a*a*a)
end

function test1b1(a, b, c)
    io.stderr:write(string.format("%g %g+%gi %g+%gi\n", a, b, c, b, -c))
    local a3 = 1.
    local a2 = -(a+2.*b)
    local a1 = b*b+c*c+2.*a*b
    local a0 = -a*(b*b+c*c)
    printroots(a3, a2, a1, a0)
    io.stderr:write(string.format("-> %g\n", a3*a*a*a+a2*a*a+a1*a+a0))
end

function test21(a, b)
    io.stderr:write(string.format("%g %g %g\n", a, a, b))
    local a3 = 1.
    local a2 = -(b+2.*a)
    local a1 = a*a+2.*a*b
    local a0 = -a*a*b
    printroots(a3, a2, a1, a0)
    io.stderr:write(string.format("-> %g %g\n",
        a3*a*a*a+a2*a*a+a1*a+a0,
        a3*b*b*b+a2*b*b+a1*b+a0))
end

function test111(a, b, c)
    io.stderr:write(string.format("%g %g %g\n", a, b, c))
    local a3 = 1.
    local a2 = -(a+b+c)
    local a1 = a*b+b*c+c*a
    local a0 = -a*b*c
    printroots(a3, a2, a1, a0)
    io.stderr:write(string.format("-> %g %g %g\n",
        a3*a*a*a+a2*a*a+a1*a+a0,
        a3*b*b*b+a2*b*b+a1*b+a0,
        a3*c*c*c+a2*c*c+a1*c+a0))
end

test3(1000.)
test3(100.)
test3(10.)
test3(1.)
test3(.1)
test3(.01)
test3(.0001)
test3(0)
test1b1(1, 1, 1)
test1b1(1, 1, .1)
test1b1(1, 1, .01)
test1b1(1, 1, .001)
test1b1(1, 1, .0001)
test1b1(1,  1,    1)
test1b1(1, .1,    1)
test1b1(1, .01,   1)
test1b1(1, .001,  1)
test1b1(1, .0001, 1)
test1b1(1,  1,    1)
test1b1(.1,  1,    1)
test1b1(.01,  1,    1)
test1b1(.001,  1,    1)
test1b1(.0001,  1,    1)
test21(0.9, 1.1)
test21(2, 1)
test21(0, 1)
test111(1, 2, 3)
test111(-1, 0, 0)
]]

return _M
