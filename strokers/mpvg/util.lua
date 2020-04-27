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
local sqrt = math.sqrt
local FLT_MIN = 1e-6

_M.FLT_MIN = FLT_MIN

function _M.stderr(...)
    io.stderr:write(string.format(...))
end

function _M.negligible(a)
    return abs(a) < FLT_MIN
end

function _M.significant(a)
    return abs(a) > FLT_MIN
end

function _M.significantp(s, t)
    return abs(s) > FLT_MIN*abs(t)
end

function _M.negligiblep(s, t)
    return abs(s) < FLT_MIN*abs(t)
end

function _M.similar(a, b)
    return abs(a-b) < FLT_MIN
end

function _M.distinct(a, b)
    return abs(a-b) > FLT_MIN
end

function _M.det2(a, b, c, d)
    return a*d-b*c
end

-- adjugate matrix
function _M.adj2(a, b, c, d)
    return d, -b, -c, a
end

function _M.apply2(a, b, c, d, x, y)
    return a*x+b*y, c*x+d*y
end

function _M.dot2(x0, y0, x1, y1)
    return x0*x1 + y0*y1
end

function _M.norm2(x, y)
    return x*x + y*y
end

function _M.sign(v)
    if v < 0. then return -1.
    elseif v > 0. then return 1.
    else return 0. end
end

return _M
