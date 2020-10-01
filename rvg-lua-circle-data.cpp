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
#include "rvg-lua-circle-data.h"

using namespace rvg;

static int const_circle_data_get_cx(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<circle_data::const_ptr>(L, 1)->
        get_cx());
    return 1;
}

static int const_circle_data_get_cy(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<circle_data::const_ptr>(L, 1)->
        get_cy());
    return 1;
}

static int const_circle_data_get_r(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<circle_data::const_ptr>(L, 1)->
        get_r());
    return 1;
}

static int const_circle_data_as_path_data(lua_State *L) {
    rvg_lua_push<path_data::ptr>(L,
        rvg_lua_check<circle_data::const_ptr>(L, 1)->as_path_data_ptr(
            rvg_lua_opt<xform>(L, 2, identity{})));
    return 1;
}

static luaL_Reg const_circle_data__index[] = {
    {"get_cx", const_circle_data_get_cx},
    {"get_cy", const_circle_data_get_cy},
    {"get_r", const_circle_data_get_r},
    {"as_path_data", const_circle_data_as_path_data},
    { nullptr, nullptr }
};


circle_data::ptr rvg_lua_circle_data_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    int n = top-base+1;
    if (n < 3) luaL_error(L, "not enough arguments (needed 3)");
    return make_intrusive<circle_data>(
        static_cast<rvgf>(luaL_checknumber(L, base+0)),
        static_cast<rvgf>(luaL_checknumber(L, base+1)),
        static_cast<rvgf>(luaL_checknumber(L, base+2)));
}

static int create(lua_State *L) {
    rvg_lua_push<circle_data::const_ptr>(L,
        rvg_lua_circle_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_circle_data[] = {
    {"circle_data", create},
    {NULL, NULL}
};

int rvg_lua_circle_data_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_path_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<circle_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<circle_data::const_ptr>(L,
            "const circle_data", ctxidx);
        rvg_lua_setmethods<circle_data::const_ptr>(L,
            const_circle_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_circle_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_circle_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_circle_data, 1); // mod
    return 0;
}
