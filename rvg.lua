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
-- ??D we should optionally identify repeated paths
-- (based on some kind of hash) to define them only once
-- and reuse them many times.
local facade = require"facade"
local indent = require"indent"
local base64 = require"base64"

local util = require"util"

local is_almost_zero = util.is_almost_zero
local is_almost_one = util.is_almost_one
local is_almost_equal = util.is_almost_equal

local math = require"math"
local image = require"image"

local atan2 = math.atan2
local unpack = table.unpack

local _M = facade.driver()

local function torotationstring(c, s)
    if (not is_almost_one(c) or not is_almost_zero(s)) then
    	return string.format("rotated(%g)", math.deg(atan2(s, c)))
	else
		return ""
	end
end

local function toscalingstring(sx, sy)
    if (is_almost_equal(sx, sy)) then
        local avg = 0.5*(sx+sy)
        if (not is_almost_one(avg)) then
            return string.format("scaled(%g)", avg)
        else
			return ""
		end
    else
		return string.format("scaled(%g, %g)", sx,  sy)
    end
end

local function totranslationstring(tx, ty)
    if (not is_almost_zero(tx) or not is_almost_zero(ty)) then
        return string.format("translated(%g, %g)", tx, ty)
	else
		return ""
	end
end

local function tolinearstring(a, b, c, d)
    if (is_almost_zero(a*b+c*d) and is_almost_one(a*d-b*c) and
        is_almost_one(a*a+c*c) and is_almost_one(b*b+d*d)) then
        return torotationstring(a, c)
    elseif (is_almost_zero(b) and is_almost_zero(c)) then
        return toscalingstring(a, d)
    else
        return string.format("linear(%g, %g, %g, %g)", a, b, c, d)
    end
end

local function toaffinitystring(a, b, tx, c, d, ty)
    if (is_almost_zero(tx) and is_almost_zero(ty)) then
        return tolinearstring(a, b, c, d)
    elseif (is_almost_one(a) and is_almost_zero(b) and
		is_almost_zero(c) and is_almost_one(d)) then
		return totranslationstring(tx, ty);
	else
		return string.format("affine(%g, %g, %g, %g, %g, %g)",
			a, b, tx, c, d, ty)
	end
end

local function toxformablestring(xf)
    return toaffinitystring(unpack(xf))
end

local function toxformstring(xf)
	local s = toxformablestring(xf)
	s = string.gsub(s, "^rotated", "rotation")
	s = string.gsub(s, "^scaled", "scaling")
	s = string.gsub(s, "^translated", "translation")
	s = string.gsub(s, "^affine", "affinity")
	s = string.gsub(s, "^linear", "linearity")
	return s
end

local print_shape = {}

-- here is yet an example showcasing how you can write
-- much less code if you learn a bit about Lua. you can also
-- write code that is harder to understand, and perhaps slower. :)
local function newcommandprinter(command, start, stop, file)
    -- returns a new function that receives the callback table
    -- and all arguments passed by the iterator (eg, self, len, x0, y0)
    return function(self, ...)
        -- if the previous was different, output current command
        if self.previous ~= command and (self.previous ~= "M" or
            command ~= "L")  then
            file:write(command, ",")
        end
        -- output arguments start through stop
        for i = start, stop do
            file:write(select(i, ...), ",")
        end
        -- current is new previous
        self.previous = command
    end
end

function make_path_f_command_printer(file)
    return {
        -- store previous command
        previous = nil,
        -- example: begin_contour receives self, x0, y0
        -- we should output "M,x0,y0". so we pass "M" as the
        -- command, and select 1 through 2 as the arguments to be output
        begin_contour = newcommandprinter("M", 1, 2, file),
        -- rest is similar
        linear_segment = newcommandprinter("L", 3, 4, file),
        degenerate_segment = newcommandprinter("L", 7, 8, file),
        quadratic_segment = newcommandprinter("Q", 3, 6, file),
        rational_quadratic_segment = newcommandprinter("R", 3, 7, file),
        cubic_segment = newcommandprinter("C", 3, 8, file),
        end_closed_contour = newcommandprinter("Z", 2, 1, file),
        end_open_contour = function(self) self.previous = nil end,
    }
end

print_shape[_M.shape_type.path] = function(shape, file)
    local path = shape:get_path_data()
    file:write("path{")
    path:iterate(make_path_f_command_printer(file))
    file:write("}")
end

print_shape[_M.shape_type.polygon] = function(shape, file)
    local polygon = shape:get_polygon_data()
    file:write("polygon{", table.concat(polygon:get_coordinates(), ","), "}")
end

print_shape[_M.shape_type.triangle] = function(shape, file)
    local triangle = shape:get_triangle_data()
    file:write("triangle(",
        triangle:get_x1(), ",", triangle:get_y1(), ",",
        triangle:get_x2(), ",", triangle:get_y2(), ",",
        triangle:get_x3(), ",", triangle:get_y3(), ")")
end

print_shape[_M.shape_type.circle] = function(shape, file)
    local circle = shape:get_circle_data()
    file:write("circle(",
        circle:get_cx(), ",", circle:get_cy(), ",", circle:get_r(), ")")
end

print_shape[_M.shape_type.rect] = function(shape, file)
    local rect = shape:get_rect_data()
    file:write("rect(",
        rect:get_x(), ",", rect:get_y(), ",", rect:get_width(), ",",
        rect:get_height(), ")")
end

local function print_style(width, style, file)
    file:write(':stroked(', width, ')')
    local wrong_join = (style:get_join() ~= style.default_join)
    if style:get_miter_limit() ~= style.default_miter_limit then
        file:write(':miter_limited(', style:get_miter_limit(), ')')
    end
    if wrong_join then
        file:write(':joined(', tostring(style:get_join()), ')')
    end
    if style:get_inner_join() ~= style.default_inner_join then
        file:write(':inner_joined(', tostring(style:get_inner_join()), ')')
    end
    if #style:get_dashes() > 0 then
        file:write(':dashed{',table.concat(style:get_dashes(), ","), '}')
    end
    if style:get_initial_cap() == style:get_terminal_cap() and
        style:get_initial_cap() == style:get_dash_initial_cap() and
        style:get_initial_cap() == style:get_dash_terminal_cap() then
        if style:get_initial_cap() ~= style.default_cap then
			file:write(':capped(', tostring(style:get_initial_cap()), ')')
        end
    else
        if style:get_initial_cap() ~= style.default_cap then
			file:write(':initial_capped(', tostring(style:get_initial_cap()), ')')
        end
        if style:get_terminal_cap() ~= style.default_cap  then
			file:write(':terminal_capped(', tostring(style:get_terminal_cap()), ')')
        end
        if style:get_dash_initial_cap() ~= style.default_cap then
			file:write(':dash_initial_capped(', tostring(
                style:get_dash_initial_cap()), ')')
        end
        if style:get_dash_terminal_cap() ~= style.default_cap then
			file:write(':dash_terminal_capped(', tostring(
                style:get_dash_terminal_cap()), ')')
        end
    end
    if style:get_resets_on_move() ~= style.default_resets_on_move then
        file:write(':reset_on_move(', tostring(style:get_resets_on_move()), ')')
    end
    if style:get_dash_offset() ~= style.default_dash_offset then
        file:write(':dash_offset(', style:get_dash_offset(), ')')
    end
end

print_shape[_M.shape_type.stroke] = function(shape, file)
    local stroke = shape:get_stroke_data()
    local stroke_shape = stroke:get_shape()
    print_shape[stroke_shape:get_type()](stroke_shape, file)
    local s = toxformablestring(stroke_shape:get_xf())
    if s ~= "" then file:write(":", s) end
    print_style(stroke:get_width(), stroke:get_style(), file)
end

local function unpack8_helper(rgba, i)
    local v = rgba[i]
    if v then
        return math.floor(v*255. + .5), unpack8_helper(rgba, i+1)
    end
end

local function unpack8(rgba)
    return unpack8_helper(rgba, 1)
end

local function print_rgba8(c, file)
    local s = _M.color_name(c)
    if not s then
        local r, g, b, a = unpack8(c)
        if a ~= 255 then
            file:write("rgba8(", r, ",", g, ",", b, ",", a, ")")
        else
            file:write("rgb8(", r, ",", g, ",", b, ")")
        end
    else
        file:write("color.", s)
    end
end

local function print_ramp(ramp, file)
    file:write('color_ramp(', tostring(ramp:get_spread()), ',{')
    local stops = ramp:get_color_stops()
    for i, s in ipairs(stops) do
        file:write("{", s:get_offset(), ",")
        print_rgba8(s:get_color(), file)
        file:write("},")
    end
    file:write('})')
end

local print_paint = {}

print_paint[_M.paint_type.radial_gradient] = function(paint, file)
    local radial_gradient = paint:get_radial_gradient_data()
    file:write("radial_gradient(")
    print_ramp(radial_gradient:get_color_ramp(), file)
    file:write(",", radial_gradient:get_cx())
    file:write(",", radial_gradient:get_cy())
    file:write(",", radial_gradient:get_fx())
    file:write(",", radial_gradient:get_fy())
    file:write(",", radial_gradient:get_r())
    if paint:get_opacity() ~= 1 then
        file:write(",", paint:get_opacity())
    end
    file:write(")")
end

print_paint[_M.paint_type.linear_gradient] = function(paint, file)
    local linear_gradient = paint:get_linear_gradient_data()
    file:write("linear_gradient(")
    print_ramp(linear_gradient:get_color_ramp(), file)
    file:write(",", linear_gradient:get_x1())
    file:write(",", linear_gradient:get_y1())
    file:write(",", linear_gradient:get_x2())
    file:write(",", linear_gradient:get_y2())
    if paint:get_opacity() ~= 1 then
        file:write(",", paint:get_opacity())
    end
    file:write(")")
end

print_paint[_M.paint_type.solid_color] = function(paint, file)
    if paint:get_opacity() ~= 1. then
        file:write("solid_color(")
        print_rgba8(paint:get_solid_color(), file)
        file:write(",", paint:get_opacity(), ")")
    else
        print_rgba8(paint:get_solid_color(), file)
    end
end

local function print_image(img, file)
    file:write("image.png.load(base64.decode[[\n")
    if img:get_channel_type() == "uint8_t" then
        file:write(base64.encode(image.png.string8(img)), "]])")
    else
        file:write(base64.encode(image.png.string16(img)), "]])")
    end
end

print_paint[_M.paint_type.texture] = function(paint, file)
    local texture = paint:get_texture_data()
    file:write("texture(", tostring(texture:get_spread()), ",")
    print_image(texture:get_image(), file)
    if paint:get_opacity() ~= 1 then
        file:write(",", paint:get_opacity())
    end
    file:write(")")
end

local winding_rule_prefix = {
    [_M.winding_rule.non_zero] = "",
    [_M.winding_rule.zero] = "z",
    [_M.winding_rule.even] = "e",
    [_M.winding_rule.odd] = "eo"
}

local function make_scene_f_print_rvg(file)
    local sceneprinter = { }
    local nl = indent.indent(1, "  ")

    function sceneprinter.painted_shape(self, winding_rule, shape, paint)
        local s = toxformablestring(shape:get_xf())
        local p = toxformablestring(paint:get_xf())
        file:write(nl:inc(), winding_rule_prefix[winding_rule], "fill(")
        print_shape[shape:get_type()](shape, file)
        if s ~= "" and s ~= p then file:write(":", s) end
        file:write(", ")
        print_paint[paint:get_type()](paint, file)
        if p ~= "" and p ~= s then file:write(":", p) end
        if s ~= "" and s == p then file:write("):", s, ",")
        else file:write("),") end
        nl:dec()
    end

    function sceneprinter.stencil_shape(self, winding_rule, shape)
        local s = toxformablestring(shape:get_xf())
        file:write(nl:inc(), winding_rule_prefix[winding_rule], "punch(")
        print_shape[shape:get_type()](shape, file)
        if s ~= "" then file:write(":", s) end
        file:write("),")
        nl:dec()
    end

    function sceneprinter.begin_transform(self, depth, xf)
        file:write(nl:inc(), "transform(", toxformstring(xf), ", {")
    end

    function sceneprinter.end_transform(self, depth, xf)
        file:write(nl:dec(), "}),")
    end

    function sceneprinter.begin_blur(self, depth, radius)
        file:write(nl:inc(), "blur(", radius, ", {")
    end

    function sceneprinter.end_blur(self, depth, radius)
        file:write(nl:dec(), "}),")
    end

    function sceneprinter.begin_fade(self, depth, opacity)
        file:write(nl:inc(), "fade(", opacity, ", {")
    end

    function sceneprinter.end_fade(self, depth, opacity)
        file:write(nl:dec(), "}),")
    end

    function sceneprinter.begin_clip(self, depth)
        file:write(nl:inc(), "clip({")
    end

    function sceneprinter.activate_clip(self, depth)
        file:write(nl:dec(), "},{")
        nl:inc()
    end

    function sceneprinter.end_clip(self, depth)
        file:write(nl:dec(), "}),")
    end

    return sceneprinter
end

-- define acceleration function
function _M.accelerate(content, window, viewport)
    return content
end

-- define rendering function
function _M.render(content, window, viewport, file)
    file:write("local rvg = {}\n\n")
    file:write("rvg.window = window(", table.concat(window, ","), ")\n\n")
    file:write("rvg.viewport = viewport(", table.concat(viewport, ","), ")\n\n")
    file:write("rvg.scene = scene({")
    content:get_scene_data():iterate(make_scene_f_print_rvg(file))
    file:write("\n})")
    local xfs = toxformablestring(content:get_xf())
    if xfs ~= "" then file:write(":", xfs) end
    file:write("\n\n")
    file:write("return rvg\n")
end

return _M
