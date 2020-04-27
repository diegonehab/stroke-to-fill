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

local window_meta = _M.meta
window_meta.__index = {}
window_meta.name = "window"

local unpack = unpack or table.unpack

function window_meta.__tostring(self)
    return string.format("window{%g,%g,%g,%g}", unpack(self, 1, 4))
end

function _M.window(xmin, ymin, xmax, ymax)
    return setmetatable({xmin, ymin, xmax, ymax}, window_meta)
end

return _M
