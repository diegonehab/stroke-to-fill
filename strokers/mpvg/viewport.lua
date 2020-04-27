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

local viewport_meta = _M.meta
viewport_meta.__index = {}
viewport_meta.name = "viewport"

local unpack = unpack or table.unpack

function viewport_meta.__tostring(self)
    return string.format("viewport{%d,%d,%d,%d}", unpack(self, 1, 4))
end

function _M.viewport(xmin, ymin, xmax, ymax)
    xmin = math.floor(xmin+.5)
    ymin = math.floor(ymin+.5)
    xmax = math.floor(xmax+.5)
    ymax = math.floor(ymax+.5)
    return setmetatable({xmin, ymin, xmax, ymax}, viewport_meta)
end

return _M
