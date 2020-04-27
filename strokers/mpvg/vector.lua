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
local _M = require"strokers.mpvg.delay.vector"
local xform = require"strokers.mpvg.delay.xform"
local overload = require"strokers.mpvg.overload"

local vector_meta = _M.meta
vector_meta.__index = {}
vector_meta.name = "vector"

local unpack = unpack or table.unpack

function vector_meta.__tostring(v)
    if v[3] == 1 then
        return string.format("vector{%g,%g}", unpack(v, 1, 2))
    else
        return string.format("vector{%g,%g,%g}", unpack(v, 1, 3))
    end
end

vector_meta.__add = overload.handler("__add")

overload.register("__add", function(u, v)
    local x1, y1, w1 = u[1], u[2], u[3]
    local x2, y2, w2 = v[1], v[2], v[3]
    if w1 == w2 then
        return setmetatable({x1+x2, y1+y2, w1}, vector_meta)
    else
        return setmetatable({x1*w2+x2*w1, y1*w2+y2*w1, w1*w2}, vector_meta)
    end
end, vector_meta, vector_meta)

vector_meta.__sub = overload.handler("__sub")

overload.register("__sub", function(u, v)
    local x1, y1, w1 = u[1], u[2], u[3]
    local x2, y2, w2 = v[1], v[2], v[3]
    if w1 == w2 then
        return setmetatable({x1-x2, y1-y2, w1}, vector_meta)
    else
        return setmetatable({x1*w2-x2*w1, y1*w2-y2*w1, w1*w2}, vector_meta)
    end
end, vector_meta, vector_meta)

function vector_meta.__unm(v)
    local x1, y1, w1 = v[1], v[2], v[3]
    if w1 == w2 then
        return setmetatable({-x1, -y1, w1}, vector_meta)
    else
        return setmetatable({-x1, -y1, w1}, vector_meta)
    end
end

vector_meta.__mul = overload.handler("__mul")

overload.register("__mul", function(v, s)
    return setmetatable({v[1]*s, v[2]*s, v[3]}, vector_meta)
end, vector_meta, "number")

overload.register("__mul", function(s, v)
    return setmetatable({v[1]*s, v[2]*s, v[3]}, vector_meta)
end, "number", vector_meta)

overload.register("__mul", function(v, A)
    local v = {0., 0., 0.}
    for j = 0,2 do
        for k = 0, 2 do
            local s = s + v[k+1]*A[k*3+j+1]
        end
        v[i] = s
    end
    return setmetatable(v, vector_meta)
end, vector_meta, xform.meta)

function _M.lerp(v, u, a)
    assert(getmetatable(v) == vector_meta and
        getmetatable(u) == vector_meta, "lerp requires vectors")
    assert(type(a) == "number")
    local x1, y1, w1 = u[1], u[2], u[3]
    local x2, y2, w2 = v[1], v[2], v[3]
    local na = 1.-a
    if w1 == w2 then
        return setmetatable({na*x1+a*x2, na*y1+a*y2, w1}, vector_meta)
    else
        return setmetatable({na*x1*w2+a*x2*w1, na*y1*w2+a*y2*w1, w1*w2}, vector_meta)
    end
end

function _M.vector(x_or_v, y_or_w_or_nil, w_or_nil)
    if getmetatable(x_or_v) == vector_meta then
        local vx, vy, vw = unpack(x_or_v, 1, 3)
        assert(not y_or_w_or_nil or type(y_or_w_or_nil) == "number",
            "expected number")
        vw = y_or_w_or_nil or vw -- potentially replace w
        assert(not w_or_nil, "too many arguments")
        return setmetatable({vx, vy, vw}, vector_meta)
    else
        assert(type(x_or_v) == "number", "expected number")
        assert(type(y_or_w_or_nil) == "number", "expected number")
        w_or_nil = w_or_nil or 1.
        assert(type(w_or_nil) == "number", "expected number")
        return setmetatable({x_or_v, y_or_w_or_nil, w_or_nil}, vector_meta)
    end
end

function _M.perp(u)
    return u:perp()
end

function _M.dot(u, v)
    return u:dot(v)
end

function _M.len2(u)
    return u:len2()
end

function _M.len(u)
    return u:len()
end

function vector_meta.__index.project(v)
    local x, y, w = unpack(v, 1, 3)
    local inv_w = 1./w
    return setmetatable({x*inv_w, y*inv_w, 1.}, vector_meta)
end

function vector_meta.__index.dot(u, v)
    local ux, uy, uw = unpack(u, 1, 3)
    local vx, vy, vw = unpack(v, 1, 3)
    return ux*vx + uy*vy, uw*vw
end

function vector_meta.__index.perp(u)
    local x, y, w = unpack(u, 1, 3)
    return setmetatable({-y, x, w}, vector_meta)
end

function vector_meta.__index.len2(u)
    local ux, uy, uw = unpack(u, 1, 3)
    return ux*ux + uy*uy, uw*uw
end

local sqrt = math.sqrt
function vector_meta.__index.len(u)
    local ux, uy, uw = unpack(u, 1, 3)
    return sqrt(ux*ux + uy*uy)/uw
end

function vector_meta.__index.translate(v,...)
    return xform.translate(...) * v
end

function vector_meta.__index.scale(v, ...)
    return xform.scale(...) * v
end

function vector_meta.__index.rotate(v, ...)
    return xform.rotate(...) * v
end

function vector_meta.__index.affine(v, ...)
    return xform.affine(...) * v
end

function vector_meta.__index.linear(v, ...)
    return xform.linear(...) * v
end

function vector_meta.__index.windowviewport(v, ...)
    return xform.windowviewport(...) * v
end

return _M
