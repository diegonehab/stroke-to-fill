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

local image = require"image"

local atan2 = math.atan2
local unpack = table.unpack
local spack = string.pack
local abs = math.abs
local concat = table.concat
local asvd = util.asvd

local function stderr(...)
    io.stderr:write(string.format(...))
end

local _M = facade.driver()

local function factorxf(xf)
    local a, b, tx, c, d, ty = unpack(xf)
    local cos_U, sin_U, sx, sy, va, vb, vc, vd = asvd(a, b, c, d)
    local ang_U = math.deg(atan2(sin_U, cos_U))
    local cos_V = va
    local sin_V = vc
    local ang_V = math.deg(atan2(sin_V, cos_V))
    if va*vd-vb*vc < 0 then
        sy = -sy
        ang_V = -ang_V
    end
    if is_almost_equal(abs(ang_U), 180) and is_almost_equal(abs(ang_V), 180) then
        ang_U = 0
        ang_V = 0
    end
    if is_almost_equal(abs(ang_U), 90) and is_almost_equal(abs(ang_V), 90)
        and ang_U * ang_V < 0 then
        sx, sy = sy, sx
        ang_U = 0
        ang_V = 0
    end

    local nxf = (_M.rotation(ang_U) * _M.scaling(sx, sy) * _M.rotation(ang_V)):toxform()
    local ra, rb, rc, rd
    ra, rb = unpack(nxf, 1, 2)
    rc, rd = unpack(nxf, 4, 5)
    if abs(a-ra) > 0.01 or
       abs(b-rb) > 0.01 or
       abs(c-rc) > 0.01 or
       abs(d-rd) > 0.01 then
        stderr("%g %g %g %g\n", a, b, c, d)
        stderr("%g %g %g %g\n", ra, rb, rc, rd)
        os.exit(1)
    end

    return tx, ty, ang_U, sx, sy, ang_V
end

local function toxformablestring(xf)
	local tx, ty, ang_U, sx, sy, ang_V = factorxf(xf)
	local t = ""
	if not is_almost_zero(ang_V) then
        t = string.format("rotated(%g)", ang_V)
	end
    if is_almost_equal(sx, sy) then
        if not is_almost_one(sx) then
            t = t ~= "" and t .. ":"  or t
            t = t .. string.format("scaled(%g)", sx)
        end
    else
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("scaled(%g,%g)", sx, sy)
    end
    if not is_almost_zero(ang_U) then
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("rotated(%g)", ang_U)
    end
    if not is_almost_zero(tx) or not is_almost_zero(ty) then
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("translated(%g,%g)", tx, ty)
    end
	return t
end

local function toxformstring(xf)
	local s = toxformablestring(xf)
	s = string.gsub(s, "^rotated", "rotation")
	s = string.gsub(s, "^scaled", "scaling")
	s = string.gsub(s, "^translated", "translation")
	s = string.gsub(s, "^affine", "affinity")
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
            file:write(command, ", ")
        end
        -- output arguments start through stop
        for i = start, stop do
            file:write(string.format("%g, ", select(i, ...)))
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

print_shape[_M.shape_type.path] = function(v, file)
    local path = v.shape:get_path_data()
    file:write("path{")
    path:iterate(make_path_f_command_printer(file))
    file:write("}")
end

print_shape[_M.shape_type.polygon] = function(v, file)
    local polygon = v.shape:get_polygon_data()
    file:write("polygon{", table.concat(polygon:get_coordinates(), ","), "}")
end

print_shape[_M.shape_type.triangle] = function(v, file)
    local triangle = v.shape:get_triangle_data()
    file:write("triangle(",
        triangle:get_x1(), ",", triangle:get_y1(), ",",
        triangle:get_x2(), ",", triangle:get_y2(), ",",
        triangle:get_x3(), ",", triangle:get_y3(), ")")
end

print_shape[_M.shape_type.circle] = function(v, file)
    local circle = v.shape:get_circle_data()
    file:write("circle(",
        circle:get_cx(), ",", circle:get_cy(), ",", circle:get_r(), ")")
end

print_shape[_M.shape_type.rect] = function(v, file)
    local rect = v.shape:get_rect_data()
    file:write("rect(",
        rect:get_x(), ",", rect:get_y(), ",", rect:get_width(), ",",
        rect:get_height(), ")")
end

local function print_dashes(d, file)
    file:write('{',table.concat(d, ","), '}')
end

local function print_style(v, file)
    local style = v.style
    file:write('stroke_style()')
    if v.dashes_index then
        file:write(':dashed(dashes[',v.dashes_index,'])')
    end
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

print_shape[_M.shape_type.stroke] = function(v, file)
    file:write("shapes[", v.shape_index, "]")
    local s = toxformablestring(v.shape_xf)
    if s ~= "" then file:write(":", s) end
    file:write(":stroked(", v.stroke_width, ", styles[",
        v.stroke_style_index, "])")
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

local function print_ramp(v, file)
    file:write('color_ramp(', tostring(v.spread), ',{')
    for i, s in ipairs(v.stops) do
        file:write("{", string.format("%g", s[1]), ", colors[", s[2], "]}, ")
    end
    file:write('})')
end

local print_paint = {}

print_paint[_M.paint_type.radial_gradient] = function(v, file)
    file:write("radial_gradient(ramps[", v.ramp_index, "],")
    file:write(string.format("%g, %g, %g, %g, %g", v.cx, v.cy, v.fx, v.fy, v.r))
    if v.opacity ~= 1 then
        file:write(string.format(", %g", v.opacity))
    end
    file:write(")")
end

print_paint[_M.paint_type.linear_gradient] = function(v, file)
    file:write("linear_gradient(ramps[", v.ramp_index, "],")
    file:write(string.format("%g, %g, %g, %g", v.x1, v.y1, v.x2, v.y2))
    if v.opacity ~= 1 then
        file:write(string.format(", %g", v.opacity))
    end
    file:write(")")
end

print_paint[_M.paint_type.solid_color] = function(v, file)
    if v.opacity ~= 1. then
        file:write("solid_color(colors[", v.rgba_index, "]")
        file:write(",", v.opacity, ")")
    else
        file:write("colors[", v.rgba_index, "]")
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

print_paint[_M.paint_type.texture] = function(v, file)
    file:write("texture(", tostring(v.spread), ", images[", v.image_index, "]")
    if v.opacity ~= 1 then
        file:write(string.format(", %g", v.opacity))
    end
    file:write(")")
end

local winding_rule_prefix = {
    [_M.winding_rule.non_zero] = "",
    [_M.winding_rule.zero] = "z",
    [_M.winding_rule.even] = "e",
    [_M.winding_rule.odd] = "eo"
}

-- define acceleration function
function _M.accelerate(scene, window, viewport)
    return scene
end

local function make_input_path_f_to_table(t)
    local n = 1
    t[n] = "path:"
    return {
        begin_contour = function(self, x, y)
            n = n + 1
            t[n] = spack("=i8dd",1,x,y)
        end,
        end_open_contour = function(self, x, y)
        end,
        end_closed_contour = function(self, x, y)
            n = n + 1
            t[n] = spack("=i8", 2)
        end,
        linear_segment = function(self, x0, y0, x1, y1)
            n = n + 1
            t[n] = spack("=i8dd",3,x1,y1)
        end,
        quadratic_segment = function(self, x0, y0, x1, y1, x2, y2)
            n = n + 1
            t[n] = spack("=i8dddd",3,x1,y1,x2,y2)
        end,
        rational_quadratic_segment = function(self, x0, y0, x1, y1, w1, x2, y2)
            n = n + 1
            t[n] = spack("=i8ddddd",4,x1,y1,w1,x2,y2)
        end,
        cubic_segment = function(self, x0, y0, x1, y1, x2, y2, x3, y3)
            n = n + 1
            t[n] = spack("=i8dddddd",5,x1,y1,x2,y2,x3,y3)
        end,
    }
end

local stroke_join_id = {
	[_M.stroke_join.miter_clip] = 1,
	[_M.stroke_join.round] = 2,
	[_M.stroke_join.bevel] = 3,
	[_M.stroke_join.miter_or_bevel] = 4,
	[_M.stroke_join.arcs] = 5,
}

local stroke_cap_id = {
	[_M.stroke_cap.butt] = 1,
	[_M.stroke_cap.round] = 2,
	[_M.stroke_cap.square] = 3,
	[_M.stroke_cap.triangle] = 4,
	[_M.stroke_cap.fletching] = 5
}

local print_element = { }

print_element.punch = function(v, nesting, file, indent)
    file:write(indent, winding_rule_prefix[v.winding_rule],
        "punch(shapes[", v.shape_index, "])")
end

print_element.stencil = function(v, nesting, file, indent)
    file:write(indent, "stencils[", v.stencil_index, "]")
    local s = toxformablestring(v.xf or _M.identity())
    if s ~= "" then file:write(":", s) end
end

local print_element_list

local function ancestor_list(cursor)
    local t = {}
    while cursor do
        t[#t+1] = cursor
        cursor = cursor.parent
    end
    return t
end

local function find_common_ancestor(from_list, to_list)
    local to_i = #to_list
    local from_i = #from_list
    while from_list[from_i] and to_list[to_i] and
          from_list[from_i].k == to_list[to_i].k  do
        from_i = from_i-1
        to_i = to_i-1
    end
    return from_i+1
end

local function close_nesting_down(from, to, file, indent)
    -- find common ancestor
    local from_list = ancestor_list(from)
    local to_list = ancestor_list(to)
    local from_i = find_common_ancestor(from_list, to_list)
    for i = 1, from_i-1 do
        if i < from_i-1 then
            file:write(indent, string.rep("  ", from_i-i-1), "})\n")
        else
            file:write(indent, string.rep("  ", from_i-i-1), "}),\n")
        end
    end
    return from_list[from_i], from_i-1
end

local function open_nesting_up(from, to, file, indent)
    if not from or not to or from.k == to.k then return 0 end
    local i = open_nesting_up(from, to.parent, file, indent)
    if to.type == "clip" then
        file:write(indent, string.rep("  ", i), "clip({\n")
        print_element_list(to.list, to.parent, file, indent .. string.rep("  ", i+1))
        file:write(indent, string.rep("  ", i), "}, {\n")
    elseif to.type == "xf" then
        file:write(indent, string.rep("  ", i), "transform(", toxformstring(to.xf), ", {\n")
    elseif to.type == "blur" then
        file:write(indent, string.rep("  ", i), "blur(", to.radius, ", {\n")
    end
    return i+1
end

print_element.painted = function(v, nesting, file, indent)
    local s = toxformablestring(v.shape_xf)
    local p = toxformablestring(v.paint_xf)
    file:write(indent, winding_rule_prefix[v.winding_rule], "fill(shapes[",
        v.shape_index, "]")
    if s ~= "" and s ~= p then file:write(":", s) end
    file:write(", ")
    file:write("paints[", v.paint_index, "]")
    if p ~= "" and p ~= s then file:write(":", p) end
    if s ~= "" and s == p then file:write("):", s)
    else file:write(")") end
end

function print_element_list(list, nesting, file, indent)
    local previous_nesting = nesting
    local ni = 0
    for i, v in ipairs(list) do
        local parent, nm = close_nesting_down(previous_nesting, v.nesting, file, indent)
        local np = open_nesting_up(parent, v.nesting, file, indent)
        ni = ni + np - nm
        previous_nesting = v.nesting
        print_element[v.type](v, previous_nesting, file, indent .. string.rep("  ", ni))
        file:write(",\n")
    end
    close_nesting_down(previous_nesting, nesting, file, indent)
end

print_element.clip = function(clip, nesting, file, indent)
    local iindent = indent .. "  "
    file:write(indent, "clip({\n")
    print_element_list(clip.clipper, nesting, file, iindent)
    file:write(indent, "}, {\n")
    print_element_list(clip.clippee, nesting, file, iindent)
    file:write(indent, "})")
end

local function make_scene_f_compress()
    local compressed = {}
    local colors = {}
    local ncolors = 0
    local shapes = {}
    local nshapes = 0
    local ramps = {}
    local nramps = 0
    local paints = {}
    local npaints = 0
    local styles = {}
    local nstyles = 0
    local dashes = {}
    local ndashes = 0
    local stencils = {}
    local nstencils = 0
    local images = {}
    local nimages = 0

    local function xfk(xf)
        if #xf == 6 then
            return "xform:" .. spack("=dddddd", unpack(xf))
        else
            return "xform:" .. spack("=ddddddddd", unpack(xf))
        end
    end

    local function unique_color(rgba)
        local k = spack("=dddd", unpack(rgba))
        local i = colors[k]
        if not i then
            ncolors = ncolors+1
            colors[ncolors] = rgba
            colors[k] = ncolors
            i = ncolors
        end
        return i, k
    end

    local function unique_ramp(ramp)
        local stops = ramp:get_color_stops()
        local t = { "ramp:", tostring(ramp:get_spread()) }
        for i,s in ipairs(stops) do
            t[#t+1] = spack("=d", s:get_offset())
            t[#t+1] = unique_color(s:get_color())
        end
        local k = concat(t)
        local i = ramps[k]
        if not i then
            nramps = nramps+1
            local st = {}
            for i,s in ipairs(stops) do
                st[#st+1] = { s:get_offset(), (unique_color(s:get_color())) }
            end
            ramps[nramps] = {
                type = t,
                stops = st,
                spread = ramp:get_spread()
            }
            ramps[k] = nramps
            i = nramps
        end
        return i, k
    end

    local function unique_image(img)
        local k
        if img:get_channel_type() == "uint8_t" then
            k = image.png.string8(img)
        else
            k = image.png.string16(img)
        end
        local i = images[k]
        if not i then
            nimages = nimages+1
            images[nimages] = img
            images[k] = nimages
            i = nimages
        end
        return i, k
    end

    local function unique_paint(paint)
        local t = paint:get_type()
        if t == _M.paint_type.solid_color then
            local rgba_index, rgba_k = unique_color(paint:get_solid_color())
            local o = paint:get_opacity()
            local k = "solid_color:" .. rgba_k .. spack("=d", o)
            local i = paints[k]
            if not i then
                npaints = npaints+1
                paints[npaints] = {
                    type = t,
                    rgba_index = rgba_index,
                    opacity = o
                }
                paints[k] = npaints
                i = npaints
            end
            return i, k
        elseif t == _M.paint_type.linear_gradient then
            local linear_gradient = paint:get_linear_gradient_data()
            local ramp_index, ramp_k = unique_ramp(
                linear_gradient:get_color_ramp())
            local o = paint:get_opacity()
            local x1 = linear_gradient:get_x1()
            local x2 = linear_gradient:get_x2()
            local y1 = linear_gradient:get_y1()
            local y2 = linear_gradient:get_y2()
            local k = "linear_gradient:" ..
                spack("=dddd", x1, y1, x2, y2) ..  ramp_k .. spack("=d", o)
            local i = paints[k]
            if not i then
                npaints = npaints+1
                paints[npaints] = {
                    type = t,
                    x1 = x1,
                    y1 = y1,
                    x2 = x2,
                    y2 = y2,
                    ramp_index = ramp_index,
                    opacity = o
                }
                paints[k] = npaints
                i = npaints
            end
            return i, k
        elseif t == _M.paint_type.radial_gradient then
            local radial_gradient = paint:get_radial_gradient_data()
            local ramp_index, ramp_k = unique_ramp(
                radial_gradient:get_color_ramp())
            local o = paint:get_opacity()
            local cx = radial_gradient:get_cx()
            local cy = radial_gradient:get_cy()
            local fx = radial_gradient:get_fx()
            local fy = radial_gradient:get_fy()
            local r = radial_gradient:get_r()
            local k = "radial_gradient:" ..
                spack("=ddddd", cx, cy, fx, fy, r) ..  ramp_k .. spack("=d", o)
            local i = paints[k]
            if not i then
                npaints = npaints+1
                paints[npaints] = {
                    type = t,
                    cx = cx,
                    cy = cy,
                    fx = fx,
                    fy = fy,
                    r = r,
                    ramp_index = ramp_index,
                    opacity = o
                }
                paints[k] = npaints
                i = npaints
            end
            return i, k
        elseif t == _M.paint_type.texture then
            local o = paint:get_opacity()
            local texture = paint:get_texture_data()
            local image_index = unique_image(texture:get_image())
            local k = "texture:" .. tostring(texture:get_spread()) ..
                spack("=d", o) .. image_index
            local i = paints[k]
            if not i then
                npaints = npaints+1
                paints[npaints] = {
                    type = t,
                    image_index = image_index,
                    opacity = o,
                    spread = texture:get_spread()
                }
                paints[k] = npaints
                i = npaints
            end
            return i, k
        end
    end

    local function unique_stroke_style(style)
        local t = { "style:", spack("=dd", style:get_miter_limit(),
                style:get_dash_offset()) }
        local udashes_index
        if #style:get_dashes() > 0 then
            local td = { "dash:" }
            for i,d in ipairs(style:get_dashes()) do
                td[#td+1] = spack("=d", d)
            end
            local kd = concat(td)
            udashes_index = dashes[kd]
            if not udashes_index then
                ndashes = ndashes+1
                dashes[ndashes] = style:get_dashes()
                dashes[kd] = ndashes
                udashes_index = ndashes
            end
            t[#t+1] = kd
        end
        t[#t+1] = spack("=i8", stroke_join_id[style:get_join()])
        t[#t+1] = spack("=i8", stroke_cap_id[style:get_initial_cap()])
        t[#t+1] = spack("=i8", stroke_cap_id[style:get_terminal_cap()])
        t[#t+1] = spack("=i8", stroke_cap_id[style:get_dash_initial_cap()])
        t[#t+1] = spack("=i8", stroke_cap_id[style:get_dash_terminal_cap()])
        local k = concat(t)
        local i = styles[k]
        if not i then
            nstyles = nstyles+1
            styles[nstyles] = {
                style = style,
                dashes_index = udashes_index,
            }
            styles[k] = nstyles
            i = nstyles
        end
        return i, k
    end

    local function unique_shape(shape)
        if shape:get_type() == _M.shape_type.stroke then
            local stroke_data = shape:get_stroke_data()
            local stroke_shape = stroke_data:get_shape()
            local xf = stroke_shape:get_xf()
            local stroke_style = stroke_data:get_style()
            local width = stroke_data:get_width()
            local ushape_index, kshape = unique_shape(stroke_shape)
            local ustyle_index, kstyle =
                unique_stroke_style(stroke_data:get_style())
            local k = "shape:" .. ushape_index .. xfk(xf) .. "stroke:" ..
                spack("=d", width) .. ustyle_index
            local i = shapes[k]
            if not i then
                nshapes = nshapes + 1
                shapes[nshapes] = {
                    type = shape:get_type(),
                    shape_index = ushape_index,
                    shape_xf = xf,
                    stroke_width = width,
                    stroke_style_index = ustyle_index
                }
                shapes[k] = nshapes
                i = nshapes
                return i, k
            end
            return i, k, true
        else
            local t = {}
            local p = shape:as_path_data()
            p:iterate(make_input_path_f_to_table(t))
            local k = concat(t)
            local i = shapes[k]
            if not i then
                nshapes = nshapes + 1
                shapes[nshapes] = {
                    type = shape:get_type(),
                    shape = shape:without_xf()
                }
                shapes[k] = nshapes
                i = nshapes
                return i, k
            end
            return i, k, true
        end
    end

    local nesting = { k = "" }
    local list = {}

    compressed.colors = colors
    compressed.ramps = ramps
    compressed.paints = paints
    compressed.shapes = shapes
    compressed.styles = styles
    compressed.dashes = dashes
    compressed.stencils = stencils
    compressed.images = images
    compressed.painted = list
    local begins = 0
    local activates = 0
    local nblurs = 0

    local function list_key(list)
        local t = {}
        for i,v in ipairs(list) do
            t[#t+1] = v.k
        end
        return concat(t)
    end

    return {
        begin_transform = function(self, depth, xf)
            local k = nesting.k ..  "xform:" .. xfk(xf)
            nesting = {
                type = "xf",
                parent = nesting,
                xf = xf,
                k = k,
            }
        end,

        end_transform = function(self, depth, xf)
            nesting = nesting.parent
        end,

        begin_blur = function(self, depth, radius)
            assert(begins == activates, "expected stencil")
            -- blurs are never merged
            nblurs = nblurs+1
            local k = nesting.k .. "blur:" .. nblurs .. spack("=d", radius)
            nesting = {
                type = "blur",
                parent = nesting,
                radius = radius,
                k = k,
            }
        end,

        end_blur = function(self, depth, radius)
            nesting = nesting.parent
        end,

        painted_shape = function(self, winding_rule, shape, paint)
            if begins ~= activates then stderr("%s begins %s ends\n",
                tostring(begins), tostring(activates)) end
            assert(begins == activates, "expected stencil")
            local shape_index = unique_shape(shape)
            local paint_index = unique_paint(paint)
            list[#list+1] = {
                type = "painted",
                nesting = nesting,
                winding_rule = winding_rule,
                shape_index = shape_index,
                shape_xf = shape:get_xf(),
                paint_index = paint_index,
                paint_xf = paint:get_xf(),
            }
        end,

        begin_clip = function(self, depth)
            begins = begins + 1
            list = { parent = list }
        end,

        activate_clip = function(self, depth)
            activates = activates + 1
            local lk = list_key(list)
            list.k = lk
            local k = nesting.k .. "clip:" .. lk .. ":activate"
            nesting = {
                type = "clip",
                parent = nesting,
                k = k,
                list = list
            }
            list = list.parent
            if activates < begins then
                list = { parent = list }
            end
        end,

        end_clip = function(self, depth)
            activates = activates - 1
            begins = begins - 1
            if activates < begins then
                local lk = list_key(list)
                list.k = lk
                local k = "clip:" .. nesting.list.k .. ":activate" .. lk .. ":end"
                local i = stencils[k]
                if not i then
                    nstencils = nstencils+1
                    stencils[nstencils] = {
                        type = "clip",
                        clipper = nesting.list,
                        clippee = list
                    }
                    stencils[k] = nstencils
                    i = nstencils
                end
                list = list.parent
                list[#list+1] = {
                    type = "stencil",
                    stencil_index = i,
                    nesting = nesting.parent,
                    k = k
                }
            end
            nesting = nesting.parent
        end,

        stencil_shape = function(self, winding_rule, shape)
            assert(begins > activates, "expected painted")
            local ushape_index = unique_shape(shape)
            local k = string.format("%spunch%d", winding_rule_prefix[winding_rule],
                ushape_index)
            local i = stencils[k]
            if not i then
                nstencils = nstencils+1
                stencils[nstencils] = {
                    type = "punch",
                    winding_rule = winding_rule,
                    shape_index = ushape_index,
                }
                stencils[k] = nstencils
                i = nstencils
            end
            local xf = shape:get_xf()
            list[#list+1] = {
                type = "stencil",
                stencil_index = i,
                nesting = nesting,
                xf = xf,
                k = "stencil:" .. i .. xfk(xf)
            }
        end,
    }, compressed

end

-- define rendering function
function _M.render(scene, window, viewport, file)
    file:write("local rvg = {}\n\n")
    file:write("rvg.window = window(", table.concat(window, ","), ")\n\n")
    file:write("rvg.viewport = viewport(", table.concat(viewport, ","), ")\n\n")
    local compressor, compressed = make_scene_f_compress()
    scene:get_scene_data():iterate(compressor)

    if #compressed.images > 0 then
        file:write("local images = { }\n")
        for i,img in ipairs(compressed.images) do
            file:write("images[", i, "] = ")
            print_image(img, file)
            file:write("\n")
        end
        file:write("\n")
    end

    if #compressed.colors > 0 then
        file:write("local colors = {\n")
        for i,c in ipairs(compressed.colors) do
            file:write("  ")
            print_rgba8(c, file)
            file:write(",\n")
        end
        file:write("}\n\n")
    end

    if #compressed.ramps > 0 then
        file:write("local ramps = {\n")
        for i,c in ipairs(compressed.ramps) do
            file:write("  ")
            print_ramp(c, file)
            file:write(",\n")
        end
        file:write("}\n\n")
    end

    if #compressed.paints > 0 then
        file:write("local paints = {\n")
        for i,c in ipairs(compressed.paints) do
            file:write("  ")
            print_paint[c.type](c, file)
            file:write(",\n")
        end
        file:write("}\n\n")
    end

    if #compressed.dashes > 0 then
        file:write("local dashes = {\n")
        for i,d in ipairs(compressed.dashes) do
            file:write("  ")
            print_dashes(d, file)
            file:write(",\n")
        end
        file:write("}\n\n")
    end

    if #compressed.styles > 0 then
        file:write("local styles = {\n")
        for i,v in ipairs(compressed.styles) do
            file:write("  ")
            print_style(v, file)
            file:write(",\n")
        end
        file:write("}\n\n")
    end

    file:write("local shapes = { }\n")
    for i,v in ipairs(compressed.shapes) do
        file:write("shapes[", i, "] = ")
        print_shape[v.type](v, file)
        file:write("\n")
    end
    file:write("\n")

    local root = { k = "" }

    if #compressed.stencils > 0 then
        file:write("local stencils = { }\n")
        for i,v in ipairs(compressed.stencils) do
            file:write("stencils[", i, "] = ")
            print_element[v.type](v, root, file, "")
            file:write("\n")
        end
        file:write("\n")
    end

    file:write("rvg.scene = scene({\n")
    print_element_list(compressed.painted, root, file, "  ")
    file:write("})")
    local xfs = toxformablestring(scene:get_xf())
    if xfs ~= "" then file:write(":", xfs) end
    file:write("\n\n")
    file:write("return rvg\n")
end

return _M
