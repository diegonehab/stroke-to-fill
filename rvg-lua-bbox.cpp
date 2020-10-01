// Stroke-to-fill conversion program and test harness
// Copyright (C) 2020 Diego Nehab
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// Contact information: diego.nehab@gmail.com
//
#include "rvg-lua.h"
#include "rvg-lua-bbox.h"
#include "rvg-floatint.h"

using namespace rvg;

#include <iostream>

template <typename BBOX>
static int bboxindex(lua_State *L) {
    int i = static_cast<int>(luaL_checkinteger(L, 2));
    BBOX *b = rvg_lua_topointer<BBOX>(L, 1);
    if (i >= 1 && i <= static_cast<int>(b->corners().size())) {
        rvg_lua_push<typename BBOX::value_type>(L, static_cast<typename BBOX::value_type>(b->corners()[i-1]));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

template <typename BBOX>
static int bboxlen(lua_State *L) {
    BBOX *b = rvg_lua_topointer<BBOX>(L, 1);
    lua_pushinteger(L, static_cast<int>(b->corners().size()));
    return 1;
}

static luaL_Reg window_meta[] = {
    {"__index", &bboxindex<window>},
    {"__len", &bboxlen<window>},
    {nullptr, nullptr}
};

static luaL_Reg viewport_meta[] = {
    {"__index", &bboxindex<viewport>},
    {"__len", &bboxlen<viewport>},
    {nullptr, nullptr}
};

viewport rvg_lua_viewport_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    int n = top-base+1;
    if (n < 4) luaL_error(L, "not enough arguments (needed 4)");
    return make_viewport(
        static_cast<rvgi>(luaL_checkinteger(L, base+0)),
		static_cast<rvgi>(luaL_checkinteger(L, base+1)),
		static_cast<rvgi>(luaL_checkinteger(L, base+2)),
		static_cast<rvgi>(luaL_checkinteger(L, base+3)));
}

window rvg_lua_window_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    int n = top-base+1;
    if (n < 4) luaL_error(L, "not enough arguments (needed 4)");
    return make_window(
        static_cast<rvgf>(luaL_checknumber(L, base+0)),
		static_cast<rvgf>(luaL_checknumber(L, base+1)),
		static_cast<rvgf>(luaL_checknumber(L, base+2)),
		static_cast<rvgf>(luaL_checknumber(L, base+3)));
}

int create_viewport(lua_State *L) {
    return rvg_lua_push<viewport>(L, rvg_lua_viewport_create(L, 1));
}

int create_window(lua_State *L) {
    return rvg_lua_push<window>(L, rvg_lua_window_create(L, 1));
}


static const luaL_Reg mod_bbox_data[] = {
    {"window", create_window},
    {"viewport", create_viewport},
    {NULL, NULL}
};

int rvg_lua_bbox_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<window>(L, ctxidx))
		rvg_lua_createtype<window>(L, "window", ctxidx, window_meta);
    if (!rvg_lua_typeexists<viewport>(L, ctxidx))
		rvg_lua_createtype<viewport>(L, "viewport", ctxidx, viewport_meta);
    return 0;
}

int rvg_lua_bbox_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_bbox_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_bbox_data, 1); // mod
    return 0;
}
