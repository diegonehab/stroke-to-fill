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
#include "rvg-lua-triangle-data.h"

using namespace rvg;

static int const_triangle_data_get_x1(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_x1());
    return 1;
}

static int const_triangle_data_get_x2(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_x2());
    return 1;
}

static int const_triangle_data_get_x3(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_x3());
    return 1;
}

static int const_triangle_data_get_y1(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_y1());
    return 1;
}

static int const_triangle_data_get_y2(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_y2());
    return 1;
}

static int const_triangle_data_get_y3(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<triangle_data::const_ptr>(L, 1)->
        get_y3());
    return 1;
}

static int const_triangle_data_as_path_data(lua_State *L) {
    rvg_lua_push<path_data::ptr>(L,
        rvg_lua_check<triangle_data::const_ptr>(L, 1)->as_path_data_ptr(
            rvg_lua_opt<xform>(L, 2, identity{})));
    return 1;
}

static luaL_Reg const_triangle_data__index[] = {
    {"get_x1", const_triangle_data_get_x1},
    {"get_x2", const_triangle_data_get_x2},
    {"get_x3", const_triangle_data_get_x3},
    {"get_y1", const_triangle_data_get_y1},
    {"get_y2", const_triangle_data_get_y2},
    {"get_y3", const_triangle_data_get_y3},
    {"as_path_data", const_triangle_data_as_path_data},
    { nullptr, nullptr }
};

triangle_data::ptr rvg_lua_triangle_data_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    int n = top-base+1;
    if (n < 6) luaL_error(L, "not enough arguments (needed 6)");
    if (n > 6) luaL_error(L, "too many arguments (needed 6)");
    return make_intrusive<triangle_data>(
        static_cast<rvgf>(luaL_checknumber(L, base+0)),
        static_cast<rvgf>(luaL_checknumber(L, base+1)),
        static_cast<rvgf>(luaL_checknumber(L, base+2)),
        static_cast<rvgf>(luaL_checknumber(L, base+3)),
        static_cast<rvgf>(luaL_checknumber(L, base+4)),
        static_cast<rvgf>(luaL_checknumber(L, base+5)));
}

static int create(lua_State *L) {
    rvg_lua_push<triangle_data::const_ptr>(L,
        rvg_lua_triangle_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_triangle_data[] = {
    {"triangle_data", create},
    {NULL, NULL}
};

int rvg_lua_triangle_data_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_path_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<triangle_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<triangle_data::const_ptr>(L,
            "const triangle_data", ctxidx);
        rvg_lua_setmethods<triangle_data::const_ptr>(L,
            const_triangle_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_triangle_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_triangle_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_triangle_data, 1); // mod
    return 0;
}
