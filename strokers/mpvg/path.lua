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
local _M = require"strokers.mpvg.delay.path"

local xform = require"strokers.mpvg.xform"
local stroke = require"strokers.mpvg.stroke"
local command = require"strokers.mpvg.command"
local arc = require"strokers.mpvg.arc"
local util = require"strokers.mpvg.util"
local negligible = util.negligible
local bezier = require"strokers.mpvg.bezier"
local filter = require"strokers.mpvg.filter"

local overload = require"strokers.mpvg.overload"

local unpack = unpack or table.unpack

local path_meta = _M.meta
path_meta.__index = {}
path_meta.name = "path"

-- Our internal representation of paths consists of an array
-- with instructions, an array of offsets, and an array with
-- data.  Each instruction has a corresponding offset entry
-- pointing into the data array.  There are two interfaces
-- to add information to the path.  The traditional
-- interface is based on move_to, line_to, close_path etc
-- commands. These are converted to our internal
-- representation in a way that guarantees consistency.  The
-- internal representation can be used to directly add
-- instructions and associated data, without much in the way
-- of consistency checks.  The internal and traditional
-- interfaces should not be mixed when adding information to
-- a path, since they depend on some internal state.
--
-- Contours are bracketed by a begin/end pair of
-- instructions.  The pair can be either open or closed.
-- (For example, depending on whether there was a close_path
-- command or not). During the dashing and offsetting
-- process, each original segment is bracketed by a
-- begin/end instruction pair, and each dash is also
-- bracketed by a begin/end instrution pair. Bracketing
-- instructions carry a length data that represents the
-- number to be added to the open instruction index to reach
-- the close instruction index.
--
-- The offset of each instruction points to
-- the start of the instruction's data, so that all
-- instructions can be processed in parallel if need be.
-- Many instructions share common data. In the table below,
-- the data that each instruction needs when being added to
-- a path is marked with '+'. The data to which the
-- instruction's offset points is marked with a '^'
--
-- BOC ^+len +x0 +y0                      ; begin_open_contour
-- EOC ^x0 y0 +len                        ; end_open_contour
-- BCC ^+len +x0 +y0                      ; begin_closed_contour
-- ECC ^x0 y0 +len                        ; end_closed_contour
-- DS  ^x0 y0 +dx0 +dy0 +dx1 +dy1 +x1 +y1 ; degenerate_segment
-- LS  ^x0 y0 +x1 +y1                     ; linear_segment
-- LSL ^x0 y0 +len +x1 +y1                ; linear_segment_with_length
-- QS  ^x0 y0 +x1 +y1 +x2 +y2             ; quadratic_segment
-- RQS ^x0 y0 +x1 +y1 +w1 +x2 +y2         ; rational_quadratic_segment
-- CS  ^x0 y0 +x1 +y1 +x2 +y2 +x3 +y3     ; cubic_segment
-- BS  ^+s +t +x0 +y0                     ; begin_segment
-- ES  ^+s +t +x1 +y1                     ; end_segment
-- BD  ^+len +x0 +y0                      ; begin_dash
-- ED  ^+x0 +y0 +len                      ; end_dash
--
-- The degenerate segment represents a segment with zero
-- length. dx0 dy0 represents the tangent direction before
-- the segment. dx1 dy1 the tangent direction after the
-- segment. x1 y1 simply repeats the control point before
-- the segment, for reversibility
--
-- The len in begin/end instructions (when applicable) allows us
-- to find the matching end/begin instruction and is computed
-- automatically.
-- The s, t for begin/end segment is such that s/t should be
-- the signed curvature
-- The dx, dy for begin/end dash should be tangent
-- The len for linear_segment_with_length should be set to
-- sqrt((x0-x1)^2+(y0-y1)^2).
--
-- The idea is that the representation is reversible in the
-- sense that traversing it forward or backward is equally
-- easy. The datastructure also provide easy random access
-- to the data for each instruction

_M.path = overload.handler("path.path")

overload.register("path.path", function()
    return setmetatable({
        type = "path", -- shape type
        instructions = {},
        offsets = {},
        data = {},
        xf = xform.identity()
    }, path_meta)
end)

local function push_data(path, ...)
    local data = path.data
    local n = #data
    for i = 1, select("#", ...) do
        data[n+i] = select(i, ...)
    end
end

local function push_instruction(path, type, rewind)
    rewind = rewind or -2
    local instructions_n = #path.instructions+1
    local data_n = #path.data+1
    path.instructions[instructions_n] = type
    path.offsets[instructions_n] = data_n+rewind
end

local bracket = {
    ["begin_open_contour"] = "end_open_contour",
    ["end_open_contour"] = "begin_open_contour",
    ["begin_closed_contour"] = "end_closed_contour",
    ["end_closed_contour"] = "begin_closed_contour"
}

local function begin_contour(path, type, len, x0, y0)
    assert(not path.ibegin_contour, "nested contour")
    -- add new instruction
    push_instruction(path, type, 0)
    push_data(path, len or 0, x0, y0)
    path.ibegin_contour = #path.instructions
end

local function end_contour(path, type, x0, y0, len)
    -- index of matching begin instruction
    local bc = assert(path.ibegin_contour, "no contour to end")
    -- clear to signal we are not inside a contour anymore
    path.ibegin_contour = nil
    -- index of new end instruction
    local ec = #path.instructions + 1
    -- length is offset between matching instruction indices
    len = ec - bc
    -- if type is not given, infer from matching begin contour
    type = type or bracket[path.instructions[bc]]
    -- make sure begin instruction matches
    path.instructions[bc] = bracket[type]
    -- update length of matching begin instruction
    path.data[path.offsets[bc]] = len
    -- add end instruction
    push_instruction(path, type)
    push_data(path, len)
end

local function ensure_begun(path)
    if not path.ibegin_contour then
        path:begin_open_contour(nil, path.current_x, path.current_y)
    end
end

local function ensure_ended(path, type)
    if path.ibegin_contour then
        -- check for empty contour and insert a degenerate segment
        if #path.instructions == path.ibegin_contour then
            path:degenerate_segment(nil, nil, 0, 0, 0, 0,
                path.current_x, path.current_y)
        end
        end_contour(path, type, nil, nil, nil, nil)
    end
end

function path_meta.__index.begin_open_contour(path, len, x0, y0)
    begin_contour(path, "begin_open_contour", len, x0, y0)
end

function path_meta.__index.begin_closed_contour(path, len, x0, y0)
    begin_contour(path, "begin_closed_contour", len, x0, y0)
end

function path_meta.__index.end_open_contour(path, x0, y0, len)
    end_contour(path, "end_open_contour", x0, y0, len)
end

function path_meta.__index.end_closed_contour(path, x0, y0, len)
    end_contour(path, "end_closed_contour", x0, y0, len)
end

function path_meta.__index.begin_dash(path, len, x0, y0)
    assert(not path.ibegin_dash, "nested dash")
    -- add new instruction
    push_instruction(path, "begin_dash", 0)
    push_data(path, len or 0, x0, y0)
    path.ibegin_dash = #path.instructions
end

function path_meta.__index.end_dash(path, x0, y0, len)
    -- index of matching begin instruction
    local bd = assert(path.ibegin_dash, "no dash to end")
    -- clear to signal we are not inside a contour anymore
    path.ibegin_dash = nil
    -- index of new end instruction
    local ed = #path.instructions + 1
    -- length is offset between matching instruction indices
    len = ed - bd
    -- update length of matching begin instruction
    path.data[path.offsets[bd]] = len
    -- add end instruction
    push_instruction(path, "end_dash")
    push_data(path, len)
end

local function set_previous(path, x, y)
    path.previous_x = x
    path.previous_y = y
end

local function set_current(path, x, y)
    path.current_x = x
    path.current_y = y
end

local function set_start(path, x, y)
    path.start_x = x
    path.start_y = y
end

function path_meta.__index.degenerate_segment(path, x0, y0, dx0, dy0,
    dx1, dy1, x1, y1)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "degenerate_segment")
    push_data(path, dx0, dy0, dx1, dy1, x1, y1)
end

function path_meta.__index.linear_segment(path, x0, y0, x1, y1)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "linear_segment")
    push_data(path, x1, y1)
end

function path_meta.__index.linear_segment_with_length(path, x0, y0, len, x1, y1)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "linear_segment_with_length")
    push_data(path, len, x1, y1)
end

function path_meta.__index.quadratic_segment(path, x0, y0, x1, y1, x2, y2)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "quadratic_segment")
    push_data(path, x1, y1, x2, y2)
end

function path_meta.__index.rational_quadratic_segment(path, x0, y0, x1, y1, w1,
        x2, y2)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "rational_quadratic_segment")
    push_data(path, x1, y1, w1, x2, y2)
end

function path_meta.__index.cubic_segment(path, x0, y0, x1, y1, x2, y2, x3, y3)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "cubic_segment")
    push_data(path, x1, y1, x2, y2, x3, y3)
end

function path_meta.__index.begin_segment(path, s, t, x0, y0)
    push_instruction(path, "begin_segment", 0)
    push_data(path, s, t, x0, y0)
end

function path_meta.__index.end_segment(path, s, t, x1, y1)
    push_instruction(path, "end_segment", 0)
    push_data(path, s, t, x1, y1)
end

local append = {}

function append.move_to_abs(path, x0, y0)
    ensure_ended(path, "end_open_contour")
    path:begin_open_contour(nil, x0, y0)
    set_start(path, x0, y0)
    set_current(path, x0, y0)
    set_previous(path, x0, y0)
    return "line_to_abs" -- implicit command that follows is line
end

function append.close_path(path)
    ensure_ended(path, "end_closed_contour")
end

function append.line_to_abs(path, x1, y1)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x1, y1)
    else
        path:linear_segment(nil, nil, x1, y1)
    end
    set_previous(path, x1, y1)
    set_current(path, x1, y1)
end

function append.quad_to_abs(path, x1, y1, x2, y2)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 and x2 == x0 and y2 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x2, y2)
    else
        path:quadratic_segment(nil, nil, x1, y1, x2, y2)
    end
    set_previous(path, x1, y1)
    set_current(path, x2, y2)
end

function append.rquad_to_abs(path, x1, y1, w1, x2, y2)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0*w1 and y1 == y0*w1 and x2 == x0 and y2 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x2, y2)
    else
        path:rational_quadratic_segment(nil, nil, x1, y1, w1, x2, y2)
    end
    set_previous(path, x2, y2)
    set_current(path, x2, y2)
end

function append.cubic_to_abs(path, x1, y1, x2, y2, x3, y3)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 and x2 == x0 and y2 == y0 and
       x3 == x0 and y3 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x3, y3)
    else
        path:cubic_segment(nil, nil, x1, y1, x2, y2, x3, y3)
    end
    set_previous(path, x2, y2)
    set_current(path, x3, y3)
end

function append.squad_to_abs(path, x2, y2)
    local x1 = 2.*path.current_x - path.previous_x
    local y1 = 2.*path.current_y - path.previous_y
    append.quad_to_abs(path, x1, y1, x2, y2)
end

function append.squad_to_rel(path, x2, y2)
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    append.squad_to_abs(path, x2, y2)
end

function append.rquad_to_rel(path, x1, y1, w1, x2, y2)
    x1 = x1 + path.current_x*w1
    y1 = y1 + path.current_y*w1
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    append.rquad_to_abs(path, x1, y1, w1, x2, y2)
end

-- TODO: split wide angled arcs into two
function append.svgarc_to_abs(path, rx, ry, rot_ang, fa, fs, x2, y2)
    local x0, y0 = path.current_x, path.current_y
    local x1, y1, w1 = arc.torational(x0, y0, rx, ry, rot_ang, fa, fs, x2, y2)
    append.rquad_to_abs(path, x1, y1, w1, x2, y2)
end

function append.svgarc_to_rel(path, rx, ry, rot_ang, fa, fs, x2, y2)
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    append.svgarc_to_abs(path, rx, ry, rot_ang, fa, fs, x2, y2)
end

function append.cubic_to_rel(path, x1, y1, x2, y2, x3, y3)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    x3 = x3 + path.current_x
    y3 = y3 + path.current_y
    append.cubic_to_abs(path, x1, y1, x2, y2, x3, y3)
end

function append.hline_to_abs(path, x1)
    local y1 = path.current_y
    append.line_to_abs(path, x1, y1)
end

function append.hline_to_rel(path, x1)
    x1 = x1 + path.current_x
    append.hline_to_abs(path, x1)
end

function append.line_to_rel(path, x1, y1)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    append.line_to_abs(path, x1, y1)
end

function append.move_to_rel(path, x0, y0)
    x0 = x0 + path.current_x
    y0 = y0 + path.current_y
    append.move_to_abs(path, x0, y0)
    return "line_to_rel"
end

function append.quad_to_rel(path, x1, y1, x2, y2)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    append.quad_to_abs(path, x1, y1, x2, y2)
end

function append.scubic_to_abs(path, x1, y1, x2, y2)
    local x0 = 2.*path.current_x - path.previous_x
    local y0 = 2.*path.current_y - path.previous_y
    append.cubic_to_abs(path, x0, y0, x1, y1, x2, y2)
end

function append.scubic_to_rel(path, x1, y1, x2, y2)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    append.scubic_to_abs(path, x1, y1, x2, y2)
end

function append.vline_to_abs(path, y1)
    local x1 = path.current_x
    append.line_to_abs(path, x1, y1)
end

function append.vline_to_rel(path, y1)
    y1 = y1 + path.current_y
    append.vline_to_abs(path, y1)
end

function path_meta.__index.open(path)
    path.current_x = 0
    path.current_y = 0
    path.previous_x = 0
    path.previous_y = 0
    path.start_x = 0
    path.start_y = 0
end

function path_meta.__index.close(path)
    path.current_x = nil
    path.current_y = nil
    path.previous_x = nil
    path.previous_y = nil
    path.start_x = nil
    path.start_y = nil
    assert(not path.ibegin_dash, "expected end dash")
    assert(not path.ibegin_contour, "expected end contour")
end

local function checkarguments(data, first, last)
    for i = first, last do
        if type(data[i]) ~= "number" then
            error(string.format("entry %d not a number", i))
        end
    end
end

local nargs = command.nargs
local shortname = command.shortname

-- build path from SVG commands
overload.register("path.path", function(svgpath)
    local path = _M.path()
    local c = svgpath[1]
    -- empty path?
    if not c then return path end
    local first = 2
    path:open()
    -- first command must be move_to_*
    assert(c == "move_to_abs" or c == "move_to_rel",
        "path must start with move_to_abs or move_to_rel")
    while true do
        local count = nargs[c]
        local last = first + count
        -- make sure arguments are numbers
        checkarguments(svgpath, first, last-1)
        -- allow command to replace itself (move_to_* becomes line_to_*)
        c = append[c](path, unpack(svgpath, first, last-1)) or c
        first = last
        if c == "close_path" or type(svgpath[first]) ~= "number" then
            c = svgpath[first]
            if not c then break end
            if not shortname[c] then
                error(string.format("entry %d not a command", first))
            end
            first = first + 1
        end
    end
    -- end last contour in path
    ensure_ended(path)
    path:close()
    return path
end, "table")

local ndata = { -- if instruction has n data elements, store n-1
    begin_open_contour = 2,
    begin_closed_contour = 2,
    end_closed_contour = 2,
    end_open_contour = 2,
    degenerate_segment = 7,
    linear_segment = 3,
    linear_segment_with_length = 4,
    quadratic_segment = 5,
    rational_quadratic_segment = 6,
    cubic_segment = 7,
    begin_segment = 3,
    end_segment = 3,
    begin_dash = 2,
    end_dash = 2,
}

local function repack(table, start, abort, ...)
    if abort then return abort end
    start = start - 1
    for i = 1, select('#', ...) do
        table[start+i] = select(i, ...) or table[start+i]
    end
end

-- iterate over path contents
function path_meta.__index.iterate(path, forward, first, last)
    local data = path.data
    local offsets = path.offsets
    local instructions = path.instructions
    first = first or 1
    last = last or #instructions
    for index = first, last do
        local instruction = instructions[index]
        local offset = offsets[index]
        -- invoke method with same name as
        -- instruction from forward table, passing all its data
        local callback = assert(forward[instruction],
            "unhandled instruction '" .. instruction .. "'")
        if repack(data, offset,
            callback(forward, unpack(data, offset, offset+ndata[instruction])))
            then break end
    end
end

local reverseinstruction = {
    begin_open_contour = "end_open_contour",
    begin_closed_contour = "end_closed_contour",
    end_open_contour = "begin_open_contour",
    end_closed_contour = "begin_closed_contour",
    degenerate_segment = "degenerate_segment",
    linear_segment = "linear_segment",
    linear_segment_with_length = "linear_segment_with_length",
    quadratic_segment = "quadratic_segment",
    rational_quadratic_segment = "rational_quadratic_segment",
    cubic_segment = "cubic_segment",
    begin_segment = "end_segment",
    end_segment = "begin_segment",
    begin_dash = "end_dash",
    end_dash = "begin_dash",
}

local reversedata = {}

function reversedata.begin_open_contour(len, x0, y0)
    return x0, y0, len
end

function reversedata.begin_dash(len, x0, y0)
    return x0, y0, len
end

function reversedata.end_dash(x0, y0, len)
    return len, x0, y0
end

reversedata.begin_closed_contour = reversedata.begin_open_contour

function reversedata.begin_segment(s, t, x0, y0)
    return -s, t, x0, y0
end

reversedata.end_segment = reversedata.begin_segment

function reversedata.end_open_contour(x0, y0, len)
    return len, x0, y0
end

reversedata.end_closed_contour = reversedata.end_open_contour

function reversedata.linear_segment(x0, y0, x1, y1)
    return x1, y1, x0, y0
end

function reversedata.degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
    return x1, y1, -dx1, -dy1, -dx0, -dy0, x0, y0
end

function reversedata.linear_segment_with_length(x0, y0, len, x1, y1)
    return x1, y1, len, x0, y0
end

function reversedata.quadratic_segment(x0, y0, x1, y1, x2, y2)
    return x2, y2, x1, y1, x0, y0
end

function reversedata.rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
    return x2, y2, x1, y1, w1, x0, y0
end

function reversedata.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
    return x3, y3, x2, y2, x1, y1, x0, y0
end

local undoreversedata = {}

function undoreversedata.begin_open_contour(x0, y0, len)
    return len, x0, y0
end

undoreversedata.begin_closed_contour = undoreversedata.begin_open_contour

function undoreversedata.begin_dash(x0, y0, len)
    return len, x0, y0
end

undoreversedata.end_dash = undoreversedata.begin_dash


function undoreversedata.begin_segment(s, t, x0, y0)
    return -s, t, x0, y0
end

undoreversedata.end_segment = undoreversedata.begin_segment

function undoreversedata.end_open_contour(len, x0, y0)
    return x0, y0, len
end

undoreversedata.end_closed_contour = undoreversedata.end_open_contour

function undoreversedata.linear_segment(x1, y1, x0, y0)
    return x0, y0, x1, y1
end

function undoreversedata.degenerate_segment(x1, y1, dx1, dy1, dx0, dy0, x0, y0)
    return x0, y0, -dx0, -dy0, -dx1, -dy1, x1, y1
end

function undoreversedata.linear_segment_with_length(x1, y1, len, x0, y0)
    return x0, y0, len, x1, y1
end

function undoreversedata.quadratic_segment(x2, y2, x1, y1, x0, y0)
    return x0, y0, x1, y1, x2, y2
end

function undoreversedata.rational_quadratic_segment(x2, y2, x1, y1, w1, x0, y0)
    return x0, y0, x1, y1, w1, x2, y2
end

function undoreversedata.cubic_segment(x3, y3, x2, y2, x1, y1, x0, y0)
    return x0, y0, x1, y1, x2, y2, x3, y3
end

local function reverserepack(table, start, undoreverse, abort, first, ...)
    if first then return repack(table, start, abort, undoreverse(first, ...))
    else return abort end
end

-- iterate over path contents in reverse order
function path_meta.__index.riterate(path, forward, first, last)
    local data = path.data
    local offsets = path.offsets
    local instructions = path.instructions
    first = first or 1
    last = last or #instructions
    for index = last, first, -1 do
        local instruction = instructions[index]
        local offset = offsets[index]
        -- invoke method with same name as
        -- instruction from forward table, passing all its data
        local reverse = assert(reversedata[instruction], instruction)
        local undoreverse = assert(undoreversedata[instruction], instruction)
        local revinstr = assert(reverseinstruction[instruction], instruction)
        local callback = assert(forward[revinstr], revinstr)
        if reverserepack(data, offset,
            undoreverse, callback(forward,
                reverse(unpack(data, offset, offset+ndata[instruction]))))
            then break end
    end
end

local function nextcontour(first, last)
    return function(path, index)
        if index then index = index + path.data[path.offsets[index]] + 1
        else index = first end
        -- advance until we found a begin
        while 1 do
            if index > last then return nil end
            local instr = path.instructions[index]
            if not instr then return nil end
            if instr == "begin_open_contour" or
               instr == "begin_closed_contour" then break end
            index = index + 1
        end
        return index, index+path.data[path.offsets[index]]
    end
end

function path_meta.__index.contours(path, first, last)
    return nextcontour(first or 1, last or #path.instructions), path, nil
end

local function previouscontour(first, last)
    return function(path, index)
        if not index then index = last+1 end
        -- move from begin to end that preceeds it
        index = index - 1
        -- advance until we find an end
        while 1 do
            if index < first then return nil end
            local instr = path.instructions[index]
            if not instr then return nil end
            if instr == "end_open_contour" or
               instr == "end_closed_contour" then break end
            index = index - 1
        end
        local offset = path.offsets[index]
        local len = path.data[offset+2]
        -- move to matching begin
        index = index - len
        if index < first then return nil end
        return index, index + len
    end
end

function path_meta.__index.rcontours(path, first, last)
    return previouscontour(first or 1, last or #path.instructions), path, nil
end

local function nextdash(first, last)
    return function(path, index)
        if index then index = index + path.data[path.offsets[index]] + 1
        else index = first end
        -- advance until we found a begin
        while 1 do
            if index > last then return nil end
            local instr = path.instructions[index]
            if not instr then return nil end
            if instr == "begin_dash" then break end
            index = index + 1
        end
        return index, index+path.data[path.offsets[index]]
    end
end

function path_meta.__index.dashes(path, first, last)
    return nextdash(first or 1, last or #path.instructions), path, nil
end

local function previousdash(first, last)
    return function(path, index)
        if not index then index = last+1 end
        -- move from begin to end that preceeds it
        index = index - 1
        -- retreat until we find an end
        while 1 do
            if index < first then return nil end
            local instr = path.instructions[index]
            if not instr then return nil end
            if instr == "end_dash" then break end
            index = index - 1
        end
        local offset = path.offsets[index]
        local len = path.data[offset+2]
        -- move to matching begin
        index = index - len
        if index < first then return nil end
        return index, index + len
    end
end

function path_meta.__index.rdashes(path, first, last)
    return previousdash(first or 1, last or #path.instructions), path, nil
end

local function fwdtangent()
    local tx, ty = 0, 0
    local filter = {}
    function filter:set(stx, sty)
        tx, ty = stx, sty
    end
    function filter:begin_open_contour(len, x0, y0)
        tx, ty = 0, 0
    end
    function filter:begin_closed_contour(len, x0, y0)
    end
    function filter:end_closed_contour(x0, y0, len)
    end
    function filter:begin_segment(s, t, x0, y0)
    end
    function filter:end_segment(s, t, x0, y0)
    end
    function filter:end_open_contour(x0, y0, len)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        tx, ty = x1-x0, y1-y0
    end
    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        tx, ty = x1-x0, y1-y0
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        -- if initial orientation is undefined
        if negligible(dx0) and negligible(dy0) then
            dx0, dy0 = tx, ty
            -- replace from propagation
            return nil, x0, y0, dx0, dy0, dx1, dy1, x1, y1
        end
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        tx, ty = bezier.finaltangent2(x0, y0, x1, y1, x2, y2)
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        tx, ty = bezier.finaltangent2rc(x0, y0, x1, y1, w1, x2, y2)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        tx, ty = bezier.finaltangent3(x0, y0, x1, y1, x2, y2, x3, y3)
    end
    function filter:begin_segment(s, t, x, y)
    end
    function filter:end_segment(s, t, x, y)
    end
    function filter:begin_dash(len, x0, y0)
    end
    function filter:end_dash(x0, y0, len)
    end
    return filter
end

local function revtangent()
    local tx, ty = 0, 0
    local filter = {}
    function filter:set(stx, sty)
        tx, ty = stx, sty
    end
    function filter:begin_open_contour(len, x0, y0)
        tx, ty = 0, 0
    end
    function filter:begin_closed_contour(len, x0, y0)
    end
    function filter:end_closed_contour(x0, y0, len)
    end
    function filter:end_open_contour(x0, y0, len)
    end
    function filter:begin_segment(s, t, x0, y0)
    end
    function filter:end_segment(s, t, x0, y0)
    end
    function filter:linear_segment(x0, y0, x1, y1)
        tx, ty = x1-x0, y1-y0
    end
    function filter:linear_segment_with_length(x0, y0, len, x1, y1)
        tx, ty = x1-x0, y1-y0
    end
    function filter:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        -- if initial orientation is undefined
        if negligible(tx) and negligible(ty) then
            if negligible(dx1) and negligible(dy1) then
                dx1, dy1 = -1, 0
                if negligible(dx0) and negligible(dy0) then
                    dx0, dy0 = -1, 0
                end
            else
                if negligible(dx0) and negligible(dy0) then
                    dx0, dy0 = dx1, dy1
                end
            end
        else
            if negligible(dx1) and negligible(dy1) then
                dx1, dy1 = tx, ty
                if negligible(dx0) and negligible(dy0) then
                    dx0, dy0 = tx, ty
                end
            elseif negligible(dx0) and negligible(dy0) then
                dx0, dy0 = tx, ty
            end
        end
        -- replace from propagation
        return nil, x0, y0, dx0, dy0, dx1, dy1, x1, y1
    end
    function filter:begin_segment(s, t, x, y)
    end
    function filter:end_segment(s, t, x, y)
    end
    function filter:begin_dash(len, x0, y0)
    end
    function filter:end_dash(x0, y0, len)
    end
    function filter:quadratic_segment(x0, y0, x1, y1, x2, y2)
        tx, ty = bezier.finaltangent2(x0, y0, x1, y1, x2, y2)
    end
    function filter:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        tx, ty = bezier.finaltangent2rc(x0, y0, x1, y1, w1, x2, y2)
    end
    function filter:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        tx, ty = bezier.finaltangent3(x0, y0, x1, y1, x2, y2, x3, y3)
    end
    return filter
end

local it = filter.initialtangent()
local fwd = fwdtangent()
local rev = revtangent()

function path_meta.__index.orient(p)
    for cf, cl in p:contours() do
        -- get initial orientation
        p:iterate(it, cf, cl)
        local dx0, dy0 = it:get()
        -- get final orientation
        p:riterate(it, cf, cl)
        local dxe, dye = it:get()
        dxe, dye = -dxe, -dye
        -- propagate orientations forward
        fwd:set(dxe, dye)
        p:iterate(fwd, cf, cl)
        -- propagate orientations backward
        rev:set(dx0, dy0)
        p:riterate(rev, cf, cl)
    end
end

function path_meta.__index.path(p)
    return p
end

xform.setmethods(path_meta.__index)
stroke.setmethods(path_meta.__index)

return _M
