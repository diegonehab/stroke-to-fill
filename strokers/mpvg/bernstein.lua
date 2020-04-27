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

-- this guarantees that,
--   if x0 = x1, then the result will be x0 = x1 regardless of a
--   if a = 0, then the result is x0, regardles of x1
--   if a = 1, the result is x1, regardless of x0
function _M.lerp1(a, x0, x1)
    if a == 1. then return x1
    else return x0+a*(x1-x0) end
end
local lerp1 = _M.lerp1

function _M.lerp2(a, b, x0, x1, x2)
    local x00 = lerp1(a, x0, x1)
    local x01 = lerp1(a, x1, x2)
    return lerp1(b, x00, x01)
end
local lerp2 = _M.lerp2

function _M.lerp3(a, b, c, x0, x1, x2, x3)
    local x00 = lerp1(a, x0, x1)
    local x01 = lerp1(a, x1, x2)
    local x02 = lerp1(a, x2, x3)
    local x10 = lerp1(b, x00, x01)
    local x11 = lerp1(b, x01, x02)
    return lerp1(c, x10, x11)
end
local lerp3 = _M.lerp3

_M.split1 = lerp1

-- TODO: this can be made more efficient by sharing computation
-- between the values ui
function _M.split2(a, x0, x1, x2)
    local u1 = lerp2(0., a, x0, x1, x2)
    local u2 = lerp2(a,  a, x0, x1, x2)
    local u3 = lerp2(a, 1., x0, x1, x2)
    return u1, u2, u3
end

-- TODO: this can be made more efficient by sharing computation
-- between the values ui
function _M.split3(a, x0, x1, x2, x3)
    local u1 = lerp3(0., 0., a, x0, x1, x2, x3)
    local u2 = lerp3(0., a,  a, x0, x1, x2, x3)
    local u3 = lerp3(a,  a,  a, x0, x1, x2, x3)
    local u4 = lerp3(a,  a, 1., x0, x1, x2, x3)
    local u5 = lerp3(a, 1., 1., x0, x1, x2, x3)
    return u1, u2, u3, u4, u5
end

function _M.cut1(a, b, x0, x1)
    local u0 = lerp1(a, x0, x1)
    local u1 = lerp1(b, x0, x1)
    return u0, u1
end

-- TODO: this can be made more efficient by sharing computation
-- between the values ui
function _M.cut2(a, b, x0, x1, x2)
    local u0 = lerp2(a, a, x0, x1, x2)
    local u1 = lerp2(a, b, x0, x1, x2)
    local u2 = lerp2(b, b, x0, x1, x2)
    return u0, u1, u2
end

-- TODO: this can be made more efficient by sharing computation
-- between the values ui
function _M.cut3(a, b, x0, x1, x2, x3)
    local u0 = lerp3(a, a, a, x0, x1, x2, x3)
    local u1 = lerp3(a, a, b, x0, x1, x2, x3)
    local u2 = lerp3(a, b, b, x0, x1, x2, x3)
    local u3 = lerp3(b, b, b, x0, x1, x2, x3)
    return u0, u1, u2, u3
end

function _M.power2(x0, x1, x2)
    return x0, 2.*(x1-x0), (x2-x1)-(x1-x0)
end

function _M.power3(x0, x1, x2, x3)
    return x0, 3.*(x1-x0), 3.*(x2-2.*x1+x0), x3-x0-3.*(x2-x1)
end

return _M
