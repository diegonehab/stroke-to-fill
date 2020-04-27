#!/usr/local/bin/luapp
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

local chronos = require"chronos"
local total = chronos.chronos()
local util = require"util"

local quiet = false
local stroker = nil

local function stderr(...)
    if not quiet then
        io.stderr:write(string.format(...))
    end
end

-- print help and exit
local function help()
    io.stderr:write([=[
Usage:
  lua process.lua [options] <driver> [<input.rvg> [<output-name>]]
where options are:
  -profile:<output>        write profiling info to <output>
  -stroker:<method>        transform strokes to fills using <method>
  -stroker-repeats:<n>     repeat stroking <n> times
  -accelerate-repeats:<n>  repeat acceleration <n> times
  -render-repeats:<n>      repeat rendering <n> times
  -width:<number>          set viewport width (and height proportionally if not set)
  -height:<number>         set viewport height (and width proportionally if not set)
]=])
    os.exit()
end

local width, height
local drivername, inputname, outputname, profilename
local strokerrepeats = 1
local renderrepeats = 1
local acceleraterepeats = 1

-- list of supported options
-- in each option,
--   first entry is the pattern to match
--   second entry is a callback
--     if callback returns true, the option is accepted.
--     if callback returns false, the option is rejected.
local options = {
    { "^%-help$", function(w)
        if w then
            help()
            return true
        else
            return false
        end
    end },
    { "^%-quiet$", function(d)
        if not d then return false end
        quiet = true;
        return true
    end },
    { "^%-stroker%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        strokername = o
        return true
    end },
    { "^%-profile%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        profilename = o
        return true
    end },
    { "^(%-width%:(%d*)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        width = math.floor(n)
        return true
    end },
    { "^(%-stroker%-repeats%:(%d*)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        strokerrepeats = math.floor(n)
        return true
    end },
    { "^(%-render%-repeats%:(%d*)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        renderrepeats = math.floor(n)
        return true
    end },
    { "^(%-accel%-repeats%:(%d*)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        acceleraterepeats = math.floor(n)
        return true
    end },
    { "^(%-height%:(%d+)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        height = math.floor(n)
        return true
    end },
}

-- rejected options are passed to driver
local rejected = {}
local nrejected = 0
-- value do not start with -
local values = {}
local nvalues = 0

-- go over command-line arguments
-- processes recognized options
-- collect unrecognized ones into rejected list,
-- collect values into another list
for i, argument in ipairs({...}) do
    if argument:sub(1,1) == "-" then
        local recognized = false
        for j, option in ipairs(options) do
            if option[2](argument:match(option[1])) then
                recognized = true
                break
            end
        end
        if not recognized then
            nrejected = nrejected + 1
            rejected[nrejected] = argument
        end
    else
        nvalues = nvalues + 1
        values[nvalues] = argument
    end
end

drivername = values[1]
inputname = values[2]
outputname = values[3]

-- load driver
assert(drivername, "missing <driver> argument")
if not package.searchpath(drivername, package.cpath) and
   not package.searchpath(drivername, package.path) then
    drivername = "driver." .. drivername
end
local driver = require(drivername)
assert(type(driver) == "table", "invalid driver")

if strokername == "list" then
    for i,v in pairs(driver.stroke_method) do
        if i ~= "native" then
            print(i)
        end
    end
    os.exit()
end

-- load and run the Lua program that defines the scene, window, and viewport
-- the only globals visible are the ones exported by the
stderr("processing %s\n", inputname)
local time = chronos.chronos()
local input
if _VERSION == "Lua 5.1" then
    input = assert(setfenv(assert(loadfile(inputname)), driver)())
else
    input = assert(assert(loadfile(inputname, "bt", driver))())
end
stderr("loaded in %gs\n", time:elapsed())

-- by default, dump to stadard out
local output = io.stdout
-- if another argument was given, replace with the open file
if outputname then
    output = assert(io.open(outputname, "wb"))
end

-- print options that will be passed down to driver
if #rejected > 0 then
    stderr("options passed down to driver\n")
    for i,v in ipairs(rejected) do
        stderr("  %s\n", v)
    end
end

-- update viewport if width or height were given
local viewport = input.viewport
local vxmin, vymin, vxmax, vymax = table.unpack(viewport)
local vwidth = vxmax-vxmin
local vheight = vymax-vymin
if width or height then
    vxmin = 0
    vymin = 0
end
if width and not height then
    assert(vwidth > 0, "empty viewport")
    vheight = math.floor(vheight*width/vwidth+0.5)
    assert(vheight > 0, "empty viewport")
    vwidth = width
end
if height and not width then
    assert(vheight > 0, "empty viewport")
    vwidth = math.floor(vwidth*height/vheight+0.5)
    assert(vwidth > 0, "empty viewport")
    vheight = height
end
if height and width then
    vwidth = width
    vheight = height
end
viewport = driver.viewport(vxmin, vymin, vxmin+vwidth, vymin+vheight)

if strokername and strokername ~= "native" then
    local filter = require"filter"
    local method = driver.stroke_method[strokername]
    assert(method, "stroker not available")
    if profilename then
        stderr("profiling stroker...\n")
        util.profiler_start("stroker-" .. profilename)
    end
    local stroked
    time:reset()
    for i = 1, strokerrepeats do
        stderr("mock stroker pass %d\n", i)
        stroked = driver.scene_data()
        input.scene:get_scene_data():iterate(filter.make_scene_f_stroke(method, true, stroked))
    end
    local mocktime = time:elapsed()/strokerrepeats
    time:reset()
    for i = 1, strokerrepeats do
        stderr("stroker pass %d\n", i)
        stroked = driver.scene_data()
        input.scene:get_scene_data():iterate(filter.make_scene_f_stroke(method, stroked))
    end
    stderr("stroke in %gs\n", time:elapsed()/strokerrepeats - mocktime)
    if profilename then
        util.profiler_stop()
    end
    input.scene = driver.scene(stroked):transformed(input.scene:get_xf())
end

-- invoke driver-defined accelerate() function on scene
-- pass rejected options as last argument
if profilename then
    stderr("profiling accelerate...\n")
    util.profiler_start("accelerate-" .. profilename)
end
time:reset()
local accel
for i = 1, acceleraterepeats do
    stderr("accelerate pass %d\n", i)
    accel = driver.accelerate(input.scene, input.window, viewport, rejected)
end
stderr("accelerate in %gs\n", time:elapsed()/acceleraterepeats)
if profilename then
    util.profiler_stop()
end

-- invoke driver-defined render() on result of accelerate()
-- pass rejected options as last argument
if profilename then
    stderr("profiling render...\n")
    util.profiler_start("render-" .. profilename)
end
time:reset()
for i = 1, renderrepeats do
    stderr("render pass %d\n", i)
    driver.render(accel, input.window, viewport, output, rejected)
end
stderr("render in %gs\n", time:elapsed()/renderrepeats)
if profilename then
    util.profiler_stop()
end

-- close output file if we created it
if outputname then output:close() end
stderr("done in %gs\n", total:elapsed())
