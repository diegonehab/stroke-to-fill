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
local _M = require"strokers.mpvg.delay.overload"

-- adds support for a simple kind of argument-type-based overload to Lua
local overloads = {}
local switch = {}
local none = {}
local type = type

local function type_or_meta(value)
    local t = type(value)
    if t == "string" or t == "number" then return t
    else return getmetatable(value) or t end
end

local function signature(...)
    local types = {}
    for j = 1, select("#", ...) do
        local value = select(j, ...)
        local meta = getmetatable(value)
        local name
        if meta then name = tostring(meta.name or meta)
        else name = type(value) end
        types[j] = name
    end
    return "(" .. table.concat(types, ", ") .. ")"
end

local function newoverload(name)
    return function(...)
        local s = switch[name]
        local n = select("#", ...)
        if n > 0 then
            for i = 1, n do
                local t_i = type_or_meta(select(i, ...))
                assert(type(s) == "table",
                    "no overload " .. name .. signature(...))
                s = s[t_i]
            end
        else
            s = s[none]
        end
        assert(type(s) == "function", "no overload " .. name .. signature(...))
        return s(...)
    end
end

function _M.handler(name)
    local overload = overloads[name]
    if not overload then
        overload = newoverload(name)
        overloads[name] = overload
    end
    return overload
end

function _M.register(name, func, ...)
    local s = switch[name]
    if not s then
        s = {}
        switch[name] = s
    end
    local n = select("#", ...)
    if n > 0 then
        for i = 1, n-1 do
            local t_i = assert(select(i, ...))
            local sti = s[t_i]
            if not sti then
                sti = {}
                s[t_i] = sti
            end
            if type(sti) ~= "table" then error("overload redefinition") end
            s = sti
        end
        local t_i = assert(select(n, ...), "type " .. n .. " is nil")
        assert(not s[t_i], "overload redefinition")
        s[t_i] = func
    else
        s[none] = func
    end
end

return _M
