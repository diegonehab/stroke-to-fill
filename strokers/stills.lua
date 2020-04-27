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
local test = arg[1]
local base64 = require"base64"

local strokers = {
    "ours",
    "agg",
    "cairo_polygon",
    "cairo_traps",
    "cairo_tristrip",
    "direct2d",
    "gs",
    "gs_compat",
    "gs_fast",
    "illustrator_path",
    "illustrator_render",
    "livarot_outline",
    "livarot_stroke",
    "mpvg",
    "mupdf",
    "nvpr",
    "openjdk11",
    "openjdk8",
    "openvg_ri",
    "oraclejdk8",
    "qt5",
    "quartz",
    "skia",
}

local rename = {
    cairo_polygon = "cairo polygon",
    cairo_traps = "cairo traps",
    cairo_tristrip = "cairo tristrip",
    gs_compat = "gs compat",
    gs_fast = "gs fast",
    livarot_stroke = "livarot stroke",
    livarot_outline = "livarot outline",
    illustrator_path = "illustrator path",
    illustrator_render = "illustrator render",
    openvg_ri = "openvg-ri",
}

local pad = 15
local fontsize = 24

function writesvg(f)
    local ff = assert(io.open(f, "rb"))
    local s = ff:read("*a")
    s = string.gsub(s, "%<%?.-%?%>", "")
    ff:close()
    io.write(s)
end

function writebase64(f)
    local ff = assert(io.open(f, "rb"))
    local s = ff:read("*a")
    ff:close()
    io.write(base64.encode(s))
end

function get_size(f)
    local p = assert(io.popen("identify -format '%wx%h' " .. f))
    local s = assert(p:read("*a"))
    p:close()
    local W, H = string.match(s, "(%d+)x(%d+)")
    return tonumber(W), tonumber(H)
end

function get_crop(f)
    local p = assert(io.popen("convert -trim -format '%wx%h%O' " .. f .. " info:"))
    local s = assert(p:read("*a"))
    p:close()
    local w, h, x, y = string.match(s, "(%d+)x(%d+)%+(%d+)%+(%d+)")
    return tonumber(w), tonumber(h), tonumber(x), tonumber(y)
end

local W, H = assert(get_size(string.format("ours/bare/%s_ours_bare.png", test)))

local w, h, x, y = get_crop(string.format("ours/bare/%s_ours_bare.png", test))
-- miter_clip
--local w, h, x, y = math.floor(W/2), math.floor(0.575*H), math.floor(W/4), math.floor(H/10)
-- error_mitter
--local w, h, x, y = math.floor(W/2), math.floor(0.35*H), math.floor(W/2), math.floor(H/3)

print([[
<?xml version="1.0" standalone="no"?>
<svg
  xmlns:xlink="http://www.w3.org/1999/xlink"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:cc="http://creativecommons.org/ns#"
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:svg="http://www.w3.org/2000/svg"
  xmlns="http://www.w3.org/2000/svg"
  version="1.1"
  preserveAspectRatio="none"]])
print(string.format('width="%d" height="%d" >', (w+2*pad)*#strokers, (h+3*pad+fontsize)*3))
print([[
<defs>
<style type="text/css">
@font-face {
    font-family: 'Linux Biolinum O'; /* italic */
    src: url('../fonts/LinBiolinum_RI.woff') format('woff');
    font-weight: normal;
    font-style: italic;
}
</style>
</defs>
]])

print(string.format('<rect width="%d" height="%d" fill="white" />', (w+2*pad)*#strokers, (h+3*pad+fontsize)*3))

for i,stroker in ipairs(strokers) do
    local f = string.format("%s/bare/%s_%s_bare", stroker, test, stroker)
    if stroker == "nvpr" or stroker == "illustrator_render" then
        f = f .. ".png"
        print(string.format([[  <svg x="%d" y="%d" width="%d" height="%d">
        <image x="%d" y="%d" width="%d" height="%d" xlink:href="data:image/png;base64,]], (i-1)*(w+2*pad), fontsize, w+2*pad, h+2*pad, -x+pad, -y+pad, W, H))
        writebase64(f)
        print('\n"/></svg>')
    else
        f = f .. ".svg"
        print(string.format([[<svg x="%d" y="%d" width="%d" height="%d">]],
            (i-1)*(w+2*pad), fontsize, w+2*pad, h+2*pad))
        print(string.format([[<g transform="translate(%d,%d)">]],
            -x+pad, -y+pad))
        writesvg(f)
        print("</g></svg>")
    end
end

for i,stroker in ipairs(strokers) do
    local f = string.format("%s/compare/%s_%s_compare.png", stroker, test,
        stroker)

        print(string.format([[  <svg x="%d" y="%d" width="%d" height="%d">
        <image x="%d" y="%d" width="%d" height="%d" xlink:href="data:image/png;base64,]], (i-1)*(w+2*pad), h+2*pad+fontsize+fontsize, w+2*pad, h+2*pad, -x+pad, -y+pad, W, H))
        writebase64(f)
        print('\n"/></svg>')
end

for i,stroker in ipairs(strokers) do
    local f = string.format("%s/outline/%s_%s_outline", stroker, test, stroker)
    if stroker == "nvpr" or stroker == "illustrator_render" then
        f = f .. ".png"
        print(string.format([[  <svg x="%d" y="%d" width="%d" height="%d">
        <image x="%d" y="%d" width="%d" height="%d" xlink:href="data:image/png;base64,]], (i-1)*(w+2*pad), 2*(h+2*pad+fontsize)+fontsize, w+2*pad, h+2*pad, -x+pad, -y+pad, W, H))
        writebase64(f)
        print('\n"/></svg>')
    else
        f = f .. ".svg"
        print(string.format([[<svg x="%d" y="%d" width="%d" height="%d">]],
            (i-1)*(w+2*pad), 2*(h+2*pad+fontsize)+fontsize, w+2*pad, h+2*pad))
        print(string.format([[<g transform="translate(%d,%d)">]],
            -x+pad, -y+pad))
        writesvg(f)
        print("</g></svg>")
    end
end

for i,stroker in ipairs(strokers) do
    print(string.format('<text x="%d" font-family="Linux Biolinum O" font-style="italic" font-size="%d" text-anchor="middle" y="%d">%s (bare)</text>', math.floor((i-1+0.5)*(w+2*pad)), fontsize, fontsize, rename[stroker] or stroker))
end

for i,stroker in ipairs(strokers) do
    print(string.format('<text x="%d" font-family="Linux Biolinum O" font-style="italic" font-size="%d" text-anchor="middle" y="%d">%s (compare)</text>', math.floor((i-1+0.5)*(w+2*pad)), fontsize, h+2*pad+fontsize+fontsize, rename[stroker] or stroker))
end

for i,stroker in ipairs(strokers) do
    print(string.format('<text x="%d" font-family="Linux Biolinum O" font-style="italic" font-size="%d" text-anchor="middle" y="%d">%s (outline)</text>', math.floor((i-1+0.5)*(w+2*pad)), fontsize, 2*(h+2*pad+fontsize)+fontsize, rename[stroker] or stroker))
end

print("</svg>")
