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
local indent = require"indent"
local base64 = require"base64"
local indent = require"indent"

local util = require"util"

local format = string.format

local is_almost_zero = util.is_almost_zero
local is_almost_one = util.is_almost_one
local is_almost_equal = util.is_almost_equal
local sgn = util.sgn
local asvd = util.asvd

local math = require"math"
local image = require"image"

local abs = math.abs
local max = math.max
local min = math.min
local deg = math.deg
local sqrt = math.sqrt
local unpack = table.unpack
local atan2 = math.atan2

local _M = facade.driver()

local function det3(x0, y0, x1, y1, w1, x2, y2)
	return -x1*y0+w1*x2*y0+x0*y1-x2*y1-w1*x0*y2+x1*y2;
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

local function empty(t)
    return #t == 0
end

local function back(t)
    return t[#t]
end

local function push_back(t, v)
    t[#t+1] = v
end

local function pop_back(t)
    local n = #t
    local b = t[n]
    t[n] = nil
    return b
end

local function front(t)
    return t[1]
end

-- define acceleration function
function _M.accelerate(content, window, viewport)
    return content
end

local winding_rule_name = {
    [_M.winding_rule.non_zero] = "nonzero",
    [_M.winding_rule.zero] = "zero",
    [_M.winding_rule.even] = "even",
    [_M.winding_rule.odd] = "evenodd",
}

local spread_name = {
    [_M.spread.clamp] = "pad",
    [_M.spread.mirror] = "reflect",
    [_M.spread.wrap] = "repeat",
    [_M.spread.transparent] = "transparent",
}

local stroke_join_name = {
	[_M.stroke_join.miter_clip] = "miter-clip",
	[_M.stroke_join.round] = "round",
	[_M.stroke_join.bevel] = "bevel",
	[_M.stroke_join.miter_or_bevel] = "miter",
	[_M.stroke_join.arcs] = "arcs"
}

local stroke_cap_name = {
	[_M.stroke_cap.butt] = "butt",
	[_M.stroke_cap.round] = "round",
	[_M.stroke_cap.square] = "square",
	[_M.stroke_cap.triangle] = "triangle",
	[_M.stroke_cap.fletching] = "fletching"
}

local function print_rotation(c, s, title, out)
    if not is_almost_one(c) or not is_almost_zero(s) then
        out:write(title, format("=\"rotate(%g)\"", deg(atan2(s, c))))
    end
end

local function print_scaling(sx, sy, title, out)
    if is_almost_equal(sx, sy) then
        local avg = 0.5*(sx+sy)
        if not is_almost_one(avg) then
            out:write(title, format("=\"scale(%g)\"", avg))
        end
    else
        out:write(title, format("=\"scale(%g,%g)\"", sx, sy))
    end
end

local function print_translation(tx, ty, title, out)
    if not is_almost_zero(tx) or not is_almost_zero(ty) then
        out:write(title, format("=\"translate(%g,%g)\"", tx, ty))
    end
end

local function print_linear(a, b, c, d, title, out)
    if is_almost_zero(a*b+c*d) and is_almost_one(a*d-b*c) and
       is_almost_one(a*a+c*c) and is_almost_one(b*b+d*d) then
        print_rotation(a, c, title, out)
    elseif is_almost_zero(b) and is_almost_zero(c) then
        print_scaling(a, d, title, out)
    else
        out:write(title, format("=\"matrix(%g,%g,%g,%g,0,0)\"", a, c, b ,d))
    end
end

local function print_affinity(a, b, tx, c, d, ty, title, out)
    if is_almost_zero(tx) and is_almost_zero(ty) then
        print_linear(a, b, c, d, title, out)
    elseif is_almost_one(a) and is_almost_zero(b) and
        is_almost_zero(c) and is_almost_one(d) then
        print_translation(tx, ty, title, out)
    else
        out:write(title, format("=\"matrix(%g,%g,%g,%g,%g,%g)\"",
            a, c, b, d, tx, ty))
    end
end

local function print_xform(xf, title, out)
    local a, b, tx, c, d, ty = unpack(xf, 1, 6)
    print_affinity(a, b, tx, c, d, ty, title, out)
end

-- transforms path by xf
function make_path_f_xform(xf, forward)
    local px, py -- previous cursor
    local xformer = {}
    function xformer:begin_contour(x0, y0)
        px, py = xf:apply(x0, y0)
        forward:begin_contour(px, py)
    end
    function xformer:end_closed_contour(x0, y0)
        forward:end_closed_contour(px, py)
    end
    function xformer:end_open_contour(x0, y0)
        forward:end_open_contour(px, py)
    end
    function xformer:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       dx0, dy0 = xf:apply(dx0, dy0, 0)
       dx1, dy1 = xf:apply(dx1, dy1, 0)
       forward:degenerate_segment(px, py, dx0, dy0, dx1, dy1, x1, y1)
       px, py = x1, y1
    end
    function xformer:linear_segment(x0, y0, x1, y1)
       x1, y1 = xf:apply(x1, y1)
       forward:linear_segment(px, py, x1, y1)
       px, py = x1, y1
    end
    function xformer:quadratic_segment(x0, y0, x1, y1, x2, y2)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        forward:quadratic_segment(px, py, x1, y1, x2, y2)
        px, py = x2, y2
    end
    function xformer:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        x1, y1, w1 = xf:apply(x1, y1, w1)
        x2, y2 = xf:apply(x2, y2)
        forward:rational_quadratic_segment(px, py, x1, y1, w1, x2, y2)
        px, py = x2, y2
    end
    function xformer:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        x1, y1 = xf:apply(x1, y1)
        x2, y2 = xf:apply(x2, y2)
        x3, y3 = xf:apply(x3, y3)
        forward:cubic_segment(px, py, x1, y1, x2, y2, x3, y3)
        px, py = x3, y3
    end
    return xformer
end

-- here is yet an example showcasing how you can write
-- much less code if you learn a bit about Lua. you can also
-- write code that is harder to understand, and perhaps slower. :)
local function newcommandprinter(command, start, stop, file)
    -- returns a new function that receives the callback table
    -- and all arguments passed by the iterator (eg, self, len, x0, y0)
    return function(self, ...)
        -- if the previous was different, output current command
        if self.previous ~= command then
            file:write(command, " ")
        end
        -- output arguments start through stop
        for i = start, stop do
            file:write(format("%g ", select(i, ...)))
        end
        -- current is new previous
        self.previous = command
    end
end

local function make_path_f_strip_rational_quadratics(forward)
	return setmetatable({
		rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
			local s2 = 1-w1*w1
			if is_almost_zero(s2) then
				if is_almost_equal(x0, x1) and is_almost_equal(x1, x2)
				   and is_almost_equal(y0, y1) and is_almost_equal(y1, y2) then
					return forward:linear_segment(x0, y0, x2, y2)
				else
					return forward:quadratic_segment(x0, y0, x1, y1, x2, y2)
				end
			end
			local s = sgn(s2)*sqrt(abs(s2))
			-- we get the SVD of the linear part of the transformation
			local cos, sin, sx, sy = asvd(2*x1-w1*(x0+x2), s*(x2-x0),
				2*y1-w1*(y0+y2), s*(y2-y0))
            if not is_almost_zero(sx) or not is_almost_zero(sy) then
                -- the sign of the middle weight gives the large/small angle flag
                local fa = w1 < 0 and 1 or 0
                -- the sign of the area of the control point triangle gives the orientation
                local d = det3(x0, y0, x1, y1, w1, x2, y2)
                local fs = d > 0 and 1 or 0
                -- the rotation and the scaling parts from SVD give the angle and axes
                return forward:svg_arc(x0, y0, sx/(2*s2), sy/(2*s2),
                    deg(atan2(sin, cos)), fa, fs, x2, y2);
            else
                return forward:linear_segment(x0, y0, x2, y2)
            end
		end
    }, {
		__index = function(self, index)
            local f = function(self, ...)
                return forward[index](forward, ...)
            end
            rawset(self, index, f)
            return f
		end
	})
end

function make_path_f_command_printer(file)
    return {
        -- store previous command
        previous = nil,
        -- example: begin_contour receives self, x0, y0
        -- we should output "M,x0,y0". so we pass "M" as the
        -- command, and select 1 through 2 as the arguments to be output
        begin_contour = newcommandprinter("M", 1, 2, file),
        -- remaining are similar
        linear_segment = newcommandprinter("L", 3, 4, file),
        degenerate_segment = newcommandprinter("L", 7, 8, file),
        quadratic_segment = newcommandprinter("Q", 3, 6, file),
        svg_arc = newcommandprinter("A", 3, 9, file),
        cubic_segment = newcommandprinter("C", 3, 8, file),
        end_closed_contour = newcommandprinter("Z", 2, 1, file),
        end_open_contour = function(self) self.previous = nil end,
    }
end

local function print_xformed_path_data(path_data, pre_xf, out)
    out:write(" d=\"")
    path_data:iterate(
        make_path_f_xform(pre_xf,
			make_path_f_strip_rational_quadratics(
				make_path_f_command_printer(out))))
    out:write("\"")
end

local function print_path_data(path_data, out)
    out:write(" d=\"")
    path_data:iterate(
		make_path_f_strip_rational_quadratics(
			make_path_f_command_printer(out)));
    out:write("\"")
end

local scene_f_print_painted_meta = { __index = { } }

function scene_f_print_painted_meta.__index:
    print_paint(mode, shape, paint)
    self.out:write(' ', mode, "=\"")
    local type = paint:get_type()

    if type == _M.paint_type.solid_color then
        local c = paint:get_solid_color()
        local n = _M.color_name(c)
        if not n then
            local r, g, b = unpack8(c)
            self.out:write("rgb(", r, ',', g, ',', b, ')')
        else
            self.out:write(n)
        end
        local o = paint:get_opacity()
        if c[4] < 1 or paint:get_opacity() < 1 then
            self.out:write("\" ", mode, "-opacity=\"", c[4] * o)
        end
        self.out:write('"')

    elseif type == _M.paint_type.linear_gradient or
        type == _M.paint_type.radial_gradient then
        local key = paint:get_key()
        local found = self.map[key]
        if found then
             self.out:write("url(#gradient", found, ")")
             local o = paint:get_opacity()
             if o < 1 then
                 self.out:write("\" ", mode, "-opacity=\"", o)
             end
        end
        self.out:write('"')

    elseif type == _M.paint_type.texture then
        local key = paint:get_key()
        local found = self.map[key]
        if found then
             self.out:write("url(#texture", found, ")")
             local o = paint:get_opacity()
             if  o < 1 then
                 self.out:write("\" ", mode, "-opacity=\"", o)
             end
         end
         self.out:write('"')
    end
end

function scene_f_print_painted_meta.__index:
    print_stroke_style(width, st)
    self.out:write(" stroke-width=\"", width, '"')
    self.out:write(" stroke-linejoin=\"", stroke_join_name[st:get_join()], '"')
    self.out:write(" stroke-miterlimit=\"", st:get_miter_limit(), '"')
    if st:get_initial_cap() == st:get_terminal_cap() and
       st:get_initial_cap() == st:get_dash_initial_cap() and
       st:get_initial_cap() == st:get_dash_terminal_cap() then
        self.out:write(" stroke-linecap=\"",
            stroke_cap_name[st:get_initial_cap()], '"')
    end
    if not empty(st:get_dashes()) then
        self.out:write(" stroke-dasharray=\"")
        local dashes = st:get_dashes()
        for i, d in ipairs(dashes) do
            self.out:write(d*width, " ")
        end
        self.out:write('"')
    end
    self.out:write(" stroke-dashoffset=\"", st:get_dash_offset()*width, '"')
end

function scene_f_print_painted_meta.__index:
    painted_shape(winding_rule, shape, paint)
    local mode, style, data, pre_xf
    local width = 0
    local post_xf = shape:get_xf()
    if (shape:is_stroke()) then
        mode = "stroke";
        style = shape:get_stroke_data():get_style();
        width = shape:get_stroke_data():get_width();
        pre_xf = shape:get_stroke_data():get_shape():get_xf();
        -- Convert shape to be stroked into a path
        data = shape:get_stroke_data():get_shape():as_path_data(
            shape:get_xf():transformed(self.screen_xf))
    else
        mode = "fill";
        data = shape:as_path_data(self.screen_xf)
    end
    self.out:write(self.nl(), "<path id=\"shape", self.shape_id, '"')
    self.out:write(" fill-rule=\"", winding_rule_name[winding_rule], '"')
    if style then
        self.out:write(" fill=\"none\"")
        self:print_stroke_style(width, style)
    end
    print_xform(post_xf, " transform", self.out)
    self:print_paint(mode, shape, paint);
    if pre_xf then
        print_xformed_path_data(data, pre_xf, self.out)
    else
        print_path_data(data, self.out)
    end
    self.out:write("/>")
    self.shape_id = self.shape_id + 1
end

function scene_f_print_painted_meta.__index:begin_clip(depth)
    push_back(self.not_yet_active_clips, self.clippath_id)
    self.clippath_id = self.clippath_id + 1
end

function scene_f_print_painted_meta.__index:activate_clip(depth)
    push_back(self.active_clips, pop_back(self.not_yet_active_clips))
    if empty(self.not_yet_active_clips) then
        self.out:write(self.nl:inc(), "<g clip-path=\"url(#clip",
            back(self.active_clips), ")\">")
    end
end

function scene_f_print_painted_meta.__index:end_clip(depth)
    pop_back(self.active_clips)
    if empty(self.not_yet_active_clips) then
        self.out:write(self.nl:dec(), "</g>")
    end
end

function scene_f_print_painted_meta.__index:stencil_shape(winding_rule, shape)
    -- All stencil elements are in inside the <defs> section
end

function scene_f_print_painted_meta.__index:begin_fade(depth, opacity)
    if opacity < 1 then
        self.out:write(self.nl:inc(), "<g opacity=\"",
            opacity, "\">")
    end
end

function scene_f_print_painted_meta.__index:end_fade(depth, opacity)
    if opacity < 1 then
        self.out:write(self.nl:dec(), "</g>")
    end
end

function scene_f_print_painted_meta.__index:begin_blur(depth, radius)
    if not is_almost_zero(radius) then
        local key = "blur" .. radius
        local found = assert(self.map[key])
        self.out:write(self.nl:inc(), "<g filter=\"url(#blur",
            tostring(found), ")\">")
    end
end

function scene_f_print_painted_meta.__index:end_blur(depth, radius)
    if not is_almost_zero(radius) then
        self.out:write(self.nl:dec(), "</g>")
    end
end

function scene_f_print_painted_meta.__index:begin_transform(depth, xf)
    if empty(self.not_yet_active_clips) then
        self.out:write(self.nl:inc(), "<g")
        print_xform(xf, " transform", self.out)
        self.out:write('>')
    end
end

function scene_f_print_painted_meta.__index:end_transform(depth, xf)
    if empty(self.not_yet_active_clips) then
        self.out:write(self.nl:dec(), "</g>")
    end
end

local function make_scene_f_print_painted(nl, map, screen_xf, out)
    return setmetatable({
        nl = nl,
        map = map,
        screen_xf = screen_xf,
        out = out,
        shape_id = 0,
        clippath_id = 0,
        active_clips = {},
        not_yet_active_clips = {},
    }, scene_f_print_painted_meta)
end

local scene_f_print_paint_and_stencil_meta = { __index = { } }

function scene_f_print_paint_and_stencil_meta.__index:
    print_ramp(ramp)
    for i,stop in ipairs(ramp:get_color_stops()) do
        self.out:write(self.nl(), "<stop offset=\"", stop:get_offset(),
            "\" stop-color=\"")
        local c = stop:get_color()
        local n = _M.color_name(c)
        if not n then
            local r, g, b = unpack8(c)
            self.out:write("rgb(", r, ',', g, ',', b, ')')
        else
            self.out:write(n)
        end
        if c[4] < 1 then
            self.out:write("\" stop-opacity=\"", c[4])
        end
        self.out:write("\"/>")
    end
end

function scene_f_print_paint_and_stencil_meta.__index:
    print_linear_gradient(shape, paint)
    local key = paint:get_key()
    local found = self.map[key]
    if not found then
        self.map[key] = self.gradient_id
        local linear_gradient = paint:get_linear_gradient_data()
        self.out:write(self.nl(), "<linearGradient id=\"gradient",
            self.gradient_id, "\" gradientUnits=\"userSpaceOnUse\" x1=\"",
            linear_gradient:get_x1(), "\" y1=\"", linear_gradient:get_y1(),
            "\" x2=\"", linear_gradient:get_x2(), "\" y2=\"",
            linear_gradient:get_y2(), "\" spreadMethod=\"",
            spread_name[linear_gradient:get_color_ramp():get_spread()],
            "\"")
        print_xform(paint:get_xf():transformed(shape:get_xf():inverse()),
            " gradientTransform", self.out);
        self.out:write(">")
        self.nl:inc()
        self:print_ramp(linear_gradient:get_color_ramp())
        self.nl:dec()
        self.out:write(self.nl(), "</linearGradient>")
        self.gradient_id = self.gradient_id + 1
    end
end

function scene_f_print_paint_and_stencil_meta.__index:
    print_radial_gradient(shape, paint)
    local key = paint:get_key()
    local found = self.map[key]
    if not found then
        self.map[key] = self.gradient_id
        local radial_gradient = paint:get_radial_gradient_data()
        self.out:write(self.nl(), "<radialGradient id=\"gradient",
            self.gradient_id, "\" gradientUnits=\"userSpaceOnUse\" cx=\"",
            radial_gradient:get_cx(), "\" cy=\"" ,
            radial_gradient:get_cy(), "\" fx=\"" ,
            radial_gradient:get_fx(), "\" fy=\"" ,
            radial_gradient:get_fy(), "\" r=\"" ,
            radial_gradient:get_r(), "\" spreadMethod=\"" ,
            spread_name[radial_gradient:get_color_ramp():get_spread()], "\"")
        print_xform(paint:get_xf():transformed(shape:get_xf():inverse()),
            " gradientTransform", self.out);
        self.out:write(">")
        self.nl:inc()
        self:print_ramp(radial_gradient:get_color_ramp());
        self.nl:dec()
        self.out:write(self.nl(), "</radialGradient>")
        self.gradient_id = self.gradient_id + 1
    end
end

function scene_f_print_paint_and_stencil_meta.__index:
    print_texture(shape, paint)
    local key = paint:get_key()
    local found = self.map[key]
    if not found then
        self.map[key] = self.texture_id
        local texture = paint:get_texture_data();
        self.out:write(self.nl:inc(), "<pattern id=\"texture", self.texture_id,
            "\" patternUnits=\"userSpaceOnUse\" width=\"1\"",
            " height=\"1\" preserveAspectRatio=\"none\" ")
        print_xform(paint:get_xf():transformed(shape:get_xf():inverse()),
            " patternTransform", self.out)
        self.out:write(">")
        self.out:write(self.nl(), "<image id=\"image", self.texture_id,
            "\" width=\"1\" height=\"1\" preserveAspectRatio=\"none\"",
            " transform=\"scale(1,-1) translate(0,-1)\" xlink:href=\"",
            " data:image/png;base64,\n")
        local img = texture:get_image()
        if img:get_channel_type() == "uint8_t" then
            self.out:write(
                base64.encode(
                    image.png.string8(img)))
        else
            self.out:write(
                base64.encode(
                    image.png.string16(img)))
        end
        self.out:write("\"/>", self.nl:dec(), "</pattern>")
        self.texture_id = self.texture_id + 1
    end
end

function scene_f_print_paint_and_stencil_meta.__index:
    painted_shape(winding_rule, shape, paint)
    local type = paint:get_type()
    if type == _M.paint_type.linear_gradient then
        self:print_linear_gradient(shape, paint)
    elseif type == _M.paint_type.radial_gradient then
        self:print_radial_gradient(shape, paint)
    elseif type == _M.paint_type.texture then
        self:print_texture(shape, paint)
    end
end

function scene_f_print_paint_and_stencil_meta.__index:
    stencil_shape(winding_rule, shape)
    self.out:write(self.nl(), "<path id=\"stencil", self.stencil_id, '"',
        " clip-rule=\"", winding_rule_name[winding_rule], '"')
    local path_data = shape:as_path_data(self.screen_xf)
    print_path_data(path_data, self.out);
    print_xform(shape:get_xf():transformed(self.stencil_xf),
        " transform", self.out)
    self.out:write("/>")
    self.stencil_id = self.stencil_id + 1
end

function scene_f_print_paint_and_stencil_meta.__index:begin_clip(depth)
    push_back(self.not_yet_active_clips, self.clippath_id)
    self.clippath_id = self.clippath_id + 1
end

function scene_f_print_paint_and_stencil_meta.__index:activate_clip(depth)
    push_back(self.active_clips, pop_back(self.not_yet_active_clips))
end

function scene_f_print_paint_and_stencil_meta.__index:end_clip(depth)
    pop_back(self.active_clips)
end

function scene_f_print_paint_and_stencil_meta.__index:begin_fade(depth, opacity)
end

function scene_f_print_paint_and_stencil_meta.__index:end_fade(depth, opacity)
end

function scene_f_print_paint_and_stencil_meta.__index:begin_blur(depth, radius)
    if not is_almost_zero(radius) then
        local key = "blur" .. radius
        local found = self.map[key]
        if not found then
            self.map[key] = self.blur_id
            self.out:write(self.nl:inc(), "<filter id=\"blur", self.blur_id,
                "\">")
            self.out:write(self.nl(), "<feGaussianBlur stdDeviation=\"",
                radius, "\"/>")
            self.out:write(self.nl:dec(), "</filter>")
            self.blur_id = self.blur_id + 1
        end
    end
end

function scene_f_print_paint_and_stencil_meta.__index:end_blur(depth, radius)
end

function scene_f_print_paint_and_stencil_meta.__index:begin_transform(depth, xf)
    -- If the transformation happened within a stencil
    -- definition, we accumulate it
    if not empty(self.not_yet_active_clips) then
        push_back(self.stencil_xf_stack, self.stencil_xf)
        self.stencil_xf = xf:transformed(self.stencil_xf)
    end
end

function scene_f_print_paint_and_stencil_meta.__index:end_transform(depth, xf)
    if not empty(self.not_yet_active_clips) then
        self.stencil_xf = pop_back(self.stencil_xf_stack)
    end
end

local function make_scene_f_print_paint_and_stencil(nl, map, screen_xf, out)
    return setmetatable({
        nl = nl,
        map = map,
        screen_xf = screen_xf,
        out = out,
        blur_id = 0,
        gradient_id = 0,
        texture_id = 0,
        stencil_id = 0,
        clippath_id = 0,
        active_clips = {},
        not_yet_active_clips = {},
        stencil_xf_stack = {},
        stencil_xf = _M.identity()
    }, scene_f_print_paint_and_stencil_meta)
end

local scene_f_print_clip_meta = { __index = { } }

function scene_f_print_clip_meta.__index:
    painted_shape(winding_rule, shape, paint)
end

function scene_f_print_clip_meta.__index:
    stencil_shape(winding_rule, shape)
    self.out:write(self.nl(), "<use xlink:href=\"#stencil",
        self.stencil_id, '"')
    if not empty(self.nested_clips) then
        self.out:write(" clip-path=\"url(#clip", back(self.nested_clips), ")\"")
    end
    self.out:write("/>")
    self.stencil_id = self.stencil_id + 1;
end

function scene_f_print_clip_meta.__index:
    begin_clip(depth)
    self.out:write(self.nl:inc(), "<clipPath id=\"clip",
        self.clippath_id, "\">")
    push_back(self.not_yet_active_clips, self.clippath_id)
    self.clippath_id = self.clippath_id + 1
end

function scene_f_print_clip_meta.__index:
    activate_clip(depth)
    self.out:write(self.nl:dec(), "</clipPath>")
    push_back(self.active_clips, pop_back(self.not_yet_active_clips))
    if not empty(self.not_yet_active_clips) then
        push_back(self.nested_clips, back(self.active_clips))
    end
end

function scene_f_print_clip_meta.__index:
    end_clip(depth)
    if (not empty(self.nested_clips) and
        back(self.nested_clips) == back(self.active_clips)) then
        pop_back(self.nested_clips)
    end
    pop_back(self.active_clips)
end

function scene_f_print_clip_meta.__index:begin_fade(depth, opacity)
end

function scene_f_print_clip_meta.__index:end_fade(depth, opacity)
end

function scene_f_print_clip_meta.__index:begin_blur(depth, radius)
end

function scene_f_print_clip_meta.__index:end_blur(depth, radius)
end

function scene_f_print_clip_meta.__index:begin_transform(depth, xf)
end

function scene_f_print_clip_meta.__index:end_transform(depth, xf)
end

local function make_scene_f_print_clip(nl, map, screen_xf, out)
    return setmetatable({
        nl = nl,
        map = map,
        screen_xf = screen_xf,
        out = out,
        stencil_id = 0,
        clippath_id = 0,
        active_clips = {},
        not_yet_active_clips = {},
        nested_clips = {}
    }, scene_f_print_clip_meta)
end

function _M.render(content, window, viewport, out)
    local vxl, vyb, vxr, vyt = unpack(viewport)
    local wxl, wyb, wxr, wyt = unpack(window)
    local map = {}
    local nl = indent.indent(1, "  ")
    local br, bg, bb, ba = unpack8(content:get_background_color())
    ba = ba/255
    out:write("<?xml version=\"1.0\" standalone=\"no\"?>\n",
       "<svg\n",
       "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n",
       "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n",
       "  xmlns:cc=\"http://creativecommons.org/ns#\"\n",
       "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n",
       "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n",
       "  xmlns=\"http://www.w3.org/2000/svg\"\n",
       "  version=\"1.1\"\n",
       "  style=\"background-color: rgba(", br, ",", bg, ",", bb, ",", ba,");\"\n",
       "  preserveAspectRatio=\"none\"\n",
       "  width=\"", abs(vxr-vxl),
         "\" height=\"", abs(vyt-vyb), "\"\n",
           "  viewBox=\"", min(wxl,wxr), ' ', min(wyt,wyb), ' ',
            abs(wxl-wxr), ' ', abs(wyt-wyb), "\"", ">")
    local flip = _M.identity()
    if wxl > wxr then
        flip = flip:translated(0.,-wxl):scaled(-1,1):translated(0,wxr)
    end
    if wyt > wyb then
        flip = flip:translated(0.,-wyb):scaled(1,-1):translated(0,wyt)
    end
    flip = flip:toxform()
    local screen_xf = content:get_xf():
        windowviewport(window, viewport):
            transformed(flip):toxform();
    out:write(nl:inc(), "<defs>")
    -- Write stencil shape, gradient paints, and textures
    content:get_scene_data():iterate(
        make_scene_f_print_paint_and_stencil(nl, map, screen_xf, out))
    -- Write clip-paths
    content:get_scene_data():iterate(
        make_scene_f_print_clip(nl, map, screen_xf, out))
    out:write(nl:dec(), "</defs>")
    out:write(nl:inc(), "<g")
    print_xform(flip, " transform", out);
    out:write("> <!-- invert y -->")
    out:write(nl:inc(), "<g")
    print_xform(content:get_xf(), " transform", out);
    out:write("> <!-- content transformation-->")
    -- Write painted shapes
    content:get_scene_data():iterate(
        make_scene_f_print_painted(nl, map, screen_xf, out))
    out:write(nl:dec(), "</g>")
    out:write(nl:dec(), "</g>\n</svg>\n")
end

return _M
