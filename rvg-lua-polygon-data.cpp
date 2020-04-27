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
#include "rvg-lua-path-data.h"
#include "rvg-lua-xform.h"
#include "rvg-lua-polygon-data.h"

namespace rvg {
	struct polygon_data_coordinates {
		polygon_data::const_ptr ptr;
	};
}

using namespace rvg;

static int const_polygon_data_coordinates__index(lua_State *L) {
    int n = static_cast<int>(luaL_checkinteger(L, 2));
    const auto &coords = rvg_lua_check<polygon_data_coordinates>(L, 1).ptr->
        get_coordinates();
    if (n >= 1 && n <= static_cast<int>(coords.size())) {
        lua_pushnumber(L, coords[n-1]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int const_polygon_data_coordinates__len(lua_State *L) {
    lua_pushinteger(L, static_cast<lua_Integer>(rvg_lua_check<
            polygon_data_coordinates>(L, 1).ptr->get_coordinates().size()));
    return 1;
}

static luaL_Reg const_polygon_data_coordinates_meta[] = {
    {"__index", const_polygon_data_coordinates__index},
    {"__len", const_polygon_data_coordinates__len},
    {nullptr, nullptr}
};

static int const_polygon_data_get_coordinates(lua_State *L) {
    rvg_lua_push<polygon_data_coordinates>(L,
        polygon_data_coordinates{
            rvg_lua_check<polygon_data::const_ptr>(L, 1)});
    return 1;
}

static int const_polygon_data_as_path_data(lua_State *L) {
    rvg_lua_push<path_data::ptr>(L,
        rvg_lua_check<polygon_data::const_ptr>(L, 1)->as_path_data_ptr(
            rvg_lua_opt<xform>(L, 2, identity{})));
    return 1;
}

static luaL_Reg const_polygon_data__index[] = {
    {"get_coordinates", const_polygon_data_get_coordinates},
    {"as_path_data", const_polygon_data_as_path_data},
    { nullptr, nullptr }
};

polygon_data::ptr rvg_lua_polygon_data_create(lua_State *L, int base) {
    luaL_checktype(L, base, LUA_TTABLE);
    int n = rvg_lua_len(L, base);
    auto get = [](lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "entry %d is not a number", item);
        rvgf f = static_cast<rvgf>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        return f;
    };
    auto range = rvg_lua_make_table_range<rvgf>(L, base, 1, n+1, get);
    return make_intrusive<polygon_data>(range.first, range.second);
}

static int create_lua_polygon_data(lua_State *L) {
    rvg_lua_push<polygon_data::const_ptr>(L,
        rvg_lua_polygon_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_polygon_data[] = {
    {"polygon_data", create_lua_polygon_data},
    {NULL, NULL}
};

int rvg_lua_polygon_data_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_path_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<polygon_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<polygon_data::const_ptr>(L,
            "const polygon_data", ctxidx);
        rvg_lua_setmethods<polygon_data::const_ptr>(L,
            const_polygon_data__index, 0, ctxidx);
    }
    if (!rvg_lua_typeexists<polygon_data_coordinates>(L, ctxidx)) {
        rvg_lua_createtype<polygon_data_coordinates>(L,
            "const polygon_data_coordinates", ctxidx,
            const_polygon_data_coordinates_meta);
    }
    return 0;
}

int rvg_lua_polygon_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_polygon_data_init(L, ctxidx); // mod
	lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_polygon_data, 1); // mod
    return 0;
}
