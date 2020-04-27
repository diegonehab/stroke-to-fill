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
local facade = require"facade"
local image = require"image"
local filter = require"filter"

local unpack = table.unpack
local floor = math.floor

local _M = facade.driver()

function new_path_stats()
    local ps = {
        begin_contour_count = 0,
        end_open_contour_count = 0,
        end_closed_contour_count = 0,
        linear_segment_count = 0,
        quadratic_segment_count = 0,
        rational_quadratic_segment_count = 0,
        cubic_segment_count = 0,
    }
    function ps.begin_contour(self) self.begin_contour_count = self.begin_contour_count + 1 end
    function ps.end_open_contour(self) self.end_open_contour_count = self.end_open_contour_count + 1 end
    function ps.end_closed_contour(self) self.end_closed_contour_count = self.end_closed_contour_count + 1 end
    function ps.linear_segment(self) self.linear_segment_count = self.linear_segment_count + 1 end
    function ps.quadratic_segment(self) self.quadratic_segment_count = self.quadratic_segment_count + 1 end
    function ps.rational_quadratic_segment(self) self.rational_quadratic_segment_count = self.rational_quadratic_segment_count + 1 end
    function ps.cubic_segment(self) self.cubic_segment_count = self.cubic_segment_count + 1 end
    return ps
end

local function new_shape_stats()
    local ss = {}
    for i, v in pairs(_M.shape_type) do
        ss[v] = 0
    end
    return setmetatable(ss, {
        __call = function(self, shape)
            local t = shape:get_type()
            self[t] = self[t] + 1
        end
    })
end

local function new_paint_stats()
    local ss = {}
    for i, v in pairs(_M.paint_type) do
        ss[v] = 0
    end
    return setmetatable(ss, {
        __call = function(self, paint)
            local t = paint:get_type()
            self[t] = self[t] + 1
        end
    })
end

function _M.accelerate(scene, window, viewport, args)
--    scene:get_scene_data():riterate(filter.make_scene_f_spy("scene:"))
    return scene
end

local i = 0

function _M.render(content, window, viewport, file, args)
    local path_stats = new_path_stats()
    local stroke_path_stats = new_path_stats()
    local stencil_path_stats = new_path_stats()
    local stencil_shape_stats = new_shape_stats()
    local shape_stats = new_shape_stats()
    local paint_stats = new_paint_stats()
    local clips = 0
    content:get_scene_data():iterate{
        begin_clip = function()
            clips = clips + 1
        end,
        stencil_shape = function(self, winding_rule, shape)
            shape:as_path_data():iterate(stencil_path_stats)
            stencil_shape_stats(shape)
        end,
        painted_shape = function(self, winding_rule, shape, paint)
            if shape:get_type() == _M.shape_type.stroke then
                shape:get_stroke_data():get_shape():
                    as_path_data():iterate(stroke_path_stats)
            end
            shape:as_path_data():iterate(path_stats)
            shape_stats(shape)
            paint_stats(paint)
        end
    }
    for i,v in pairs(shape_stats) do
        file:write(tostring(i), ": ", v, "\n")
    end
    file:write(
        "begin_contour: ", path_stats.begin_contour_count, "\n",
        "end_open_contour: ", path_stats.end_open_contour_count, "\n",
        "end_closed_contour: ", path_stats.end_closed_contour_count, "\n",
        "linear_segment: ", path_stats.linear_segment_count, "\n",
        "quadratic_segment: ", path_stats.quadratic_segment_count, "\n",
        "rational_quadratic_segment: ", path_stats.rational_quadratic_segment_count, "\n",
        "cubic_segment: ", path_stats.cubic_segment_count, "\n"
    )
    file:write(
        "stroke_begin_contour: ", stroke_path_stats.begin_contour_count, "\n",
        "stroke_end_open_contour: ", stroke_path_stats.end_open_contour_count, "\n",
        "stroke_end_closed_contour: ", stroke_path_stats.end_closed_contour_count, "\n",
        "stroke_linear_segment: ", stroke_path_stats.linear_segment_count, "\n",
        "stroke_quadratic_segment: ", stroke_path_stats.quadratic_segment_count, "\n",
        "stroke_rational_quadratic_segment: ", stroke_path_stats.rational_quadratic_segment_count, "\n",
        "stroke_cubic_segment: ", stroke_path_stats.cubic_segment_count, "\n"
    )
    for i,v in pairs(paint_stats) do
        file:write(tostring(i), ": ", v, "\n")
    end
    file:write("clips: ", clips, "\n")
    file:write(
        "stencil_begin_contour: ", stencil_path_stats.begin_contour_count, "\n",
        "stencil_end_open_contour: ", stencil_path_stats.end_open_contour_count, "\n",
        "stencil_end_closed_contour: ", stencil_path_stats.end_closed_contour_count, "\n",
        "stencil_linear_segment: ", stencil_path_stats.linear_segment_count, "\n",
        "stencil_quadratic_segment: ", stencil_path_stats.quadratic_segment_count, "\n",
        "stencil_rational_quadratic_segment: ", stencil_path_stats.rational_quadratic_segment_count, "\n",
        "stencil_cubic_segment: ", stencil_path_stats.cubic_segment_count, "\n"
    )
    for i,v in pairs(shape_stats) do
        file:write("stencil_", tostring(i), ": ", v, "\n")
    end
end

return _M
