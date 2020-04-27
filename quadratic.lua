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
-- From "How to Solve a Quadratic Equation"
-- Jim Blinn's Corner, Nov/Dec 2005
local _M = {}

local abs = math.abs

-- returns n, t1, s1, .. tn,sn
-- where n is the number of real roots of a*x^2 + b*x + c == 0
-- and each root i in 1..n is given by ti/si
-- if delta = 1/4 * b^2 - a c is given, use it instead of computing it
function _M.quadratic(a, b, c, delta)
    b = b*.5
    delta = delta or b*b-a*c
    if delta >= 0. then
        local d = math.sqrt(delta)
        if b > 0. then
            local e = b+d
            return 2, -c, e, e, -a
        elseif b < 0. then
            local e = -b+d
            return 2, e, a, c, e
        elseif abs(a) > abs(c) then
            return 2, d, a, -d, a
        else
            return 2, -c, d, c, d
        end
    else
        return 0
    end
end

return _M
