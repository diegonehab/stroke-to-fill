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
#include "rvg-lua-xform.h"
#include "rvg-lua-path-data.h"
#include "rvg-lua-rect-data.h"

using namespace rvg;

static int const_rect_data_get_x(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<rect_data::const_ptr>(L, 1)->get_x());
    return 1;
}

static int const_rect_data_get_y(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<rect_data::const_ptr>(L, 1)->get_y());
    return 1;
}

static int const_rect_data_get_width(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<rect_data::const_ptr>(L, 1)->get_width());
    return 1;
}

static int const_rect_data_get_height(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<rect_data::const_ptr>(L, 1)->get_height());
    return 1;
}

static int const_rect_data_as_path_data(lua_State *L) {
    rvg_lua_push<path_data::ptr>(L,
        rvg_lua_check<rect_data::const_ptr>(L, 1)->as_path_data_ptr(
            rvg_lua_opt<xform>(L, 2, identity{})));
    return 1;
}

static luaL_Reg const_rect_data__index[] = {
    {"get_x", const_rect_data_get_x},
    {"get_y", const_rect_data_get_y},
    {"get_width", const_rect_data_get_width},
    {"get_height", const_rect_data_get_height},
    {"as_path_data", const_rect_data_as_path_data},
    { nullptr, nullptr }
};

rect_data::ptr rvg_lua_rect_data_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    int n = top-base+1;
    if (n < 4) luaL_error(L, "not enough arguments (needed 4)");
    if (n > 4) luaL_error(L, "too many arguments (needed 4)");
    return make_intrusive<rect_data>(
        static_cast<rvgf>(luaL_checknumber(L, base+0)),
        static_cast<rvgf>(luaL_checknumber(L, base+1)),
        static_cast<rvgf>(luaL_checknumber(L, base+2)),
        static_cast<rvgf>(luaL_checknumber(L, base+3)));
}

static int create(lua_State *L) {
    rvg_lua_push<rect_data::const_ptr>(L,
        rvg_lua_rect_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_rect_data[] = {
    {"rect_data", create},
    {NULL, NULL}
};

int rvg_lua_rect_data_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_path_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<rect_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<rect_data::const_ptr>(L,
            "const rect_data", ctxidx);
        rvg_lua_setmethods<rect_data::const_ptr>(L,
            const_rect_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_rect_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_rect_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_rect_data, 1); // mod
    return 0;
}
