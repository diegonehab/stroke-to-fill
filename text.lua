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

local freetype = require"freetype"
local harfbuzz = require"harfbuzz"
local min = math.min
local max = math.max
local huge = math.huge

local filter = require"filter"
local fonts = require"fonts"

local glyphs = {}

local function loadglyph(freetypeface, glyphs, codepoint)
    local g = glyphs[codepoint] -- load glyph from cache
    if g then return g.p, g.m end
    local p = assert(freetypeface:get_glyph_outline(codepoint),
        "no glyph for codepoint")
    local m = assert(freetypeface:get_glyph_metrics(codepoint),
        "no metrics for codepoint")
    glyphs[codepoint] = { p = p, m = m }
    return p, m
end

-- glyph cache
local glyphs = {}

-- returns a path containing the string of text
function _M.text(driver, string, facename_or_filename, features)
    -- get face from cache or load from file
    local f = fonts.loadface(facename_or_filename or fonts.files[1])
    glyphs[f] = glyphs[f] or {}
    local xcursor, ycursor = 0, 0
    local ymin = huge
    local ymax = -huge
    local xmin = huge
    local xmax = -huge
    -- path starts empty
    local p = driver.path_data()
    local buffer = harfbuzz.buffer()
    --buffer:setdirection("ttb")
    buffer:add(string)
    buffer:shape(f.harfbuzzfont, features and harfbuzz.features(features))
    --buffer:normalizeglyphs()
    local infos, n = buffer:get_glyph_infos()
    local positions = buffer:get_glyph_positions()
    -- append each codepoint to path
    for i = 1, n do
        local g, m = loadglyph(f.freetypeface, glyphs[f], infos[i].codepoint)
        -- update bounding box
        xmin = min(xmin, xcursor+m.horiz_bearing_x)
        xmax = max(xmax, xcursor+m.horiz_bearing_x+m.width)
        ymin = min(ymin, ycursor+m.horiz_bearing_y-m.height)
        ymax = max(ymax, ycursor+m.horiz_bearing_y)
        -- add glyph to path, translated by cursor
        g:iterate(
          filter.make_input_path_f_xform(
            driver.translation(
                xcursor + positions[i].x_offset,
                ycursor + positions[i].y_offset
            ), p))
        xcursor = xcursor + positions[i].x_advance
        ycursor = ycursor + positions[i].y_advance
    end
    return p, f, xmin, ymin, xmax, ymax
end

return _M
