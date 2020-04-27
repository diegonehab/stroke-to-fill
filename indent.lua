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

local indent_meta = { __index = {} }

function indent_meta:__call()
    return self.v
end

function indent_meta.__index:inc()
    local v = self.v
    self.n = self.n + 1
    self.v = "\n" .. self.s:rep(self.n)
    return v
end

function indent_meta.__index:dec()
    self.n = self.n - 1
    self.v = "\n" .. self.s:rep(self.n)
    return self.v
end

function indent_meta.__index:write(file)
    file:write(self.v)
end

function indent_meta.__index:write_inc(file)
    file:write(self:inc())
end

function indent_meta.__index:dec_write(file)
    file:write(self:dec())
end

function _M.indent(n, s)
    return setmetatable({ n = n, s = s, v = "\n" .. s:rep(n) }, indent_meta)
end

return _M
