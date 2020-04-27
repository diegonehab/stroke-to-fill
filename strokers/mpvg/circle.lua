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

local circle_meta = _M.meta
circle_meta.__index = {}
circle_meta.name = "circle"

local xform = require"strokers.mpvg.xform"
local stroke = require"strokers.mpvg.stroke"

local path = require"strokers.mpvg.path"
local command = require"strokers.mpvg.command"
local M = command.longname.M
local R = command.longname.R
local Z = command.longname.Z

function _M.circle(cx, cy, r)
    return setmetatable({
        type = "circle",
        cx = cx,
        cy = cy,
        r = r,
        xf = xform.identity()
    }, circle_meta)
end

function circle_meta.__index.path(circle)
    -- transform unit circle to given center and radius and xf
    local xf = circle.xf * xform.translate(circle.cx, circle.cy) *
            xform.scale(circle.r)
    -- we start with a unit circle centered at the origin
    -- it is formed by 3 arcs covering each third of the unit circle
    local s = 0.5           -- sin(pi/6)
    local c = 0.86602540378 -- cos(pi/6)
    local w = s
    return path.path{
        M,  0.,  1.,
        R, -c,  s,  w, -c, -s,
        R,  0., -1.,  w,  c, -s,
        R,  c,  s,  w,  0.,  1.,
        Z
    }:transform(xf)
end

xform.setmethods(circle_meta.__index)
stroke.setmethods(circle_meta.__index)

return _M
