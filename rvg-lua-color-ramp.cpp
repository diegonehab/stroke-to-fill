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
#include "rvg-lua-rgba.h"
#include "rvg-lua-spread.h"
#include "rvg-lua-color-ramp.h"

namespace rvg {
    struct const_ramp_color_stops {
        color_ramp::const_ptr ptr;
    };
}

using namespace rvg;

static int color_stop_get_offset(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<color_stop>(L, 1).get_offset());
    return 1;
}

static int color_stop_get_color(lua_State *L) {
    rvg_lua_push<RGBA8>(L, rvg_lua_check<color_stop>(L, 1).get_color());
    return 1;
}

static luaL_Reg color_stop__index[] = {
    {"get_offset", color_stop_get_offset },
    {"get_color", color_stop_get_color },
    { nullptr, nullptr }
};

static int const_ramp_color_stops__index(lua_State *L) {
    int i = static_cast<int>(luaL_checkinteger(L, 2));
    const auto &stops = rvg_lua_check<const_ramp_color_stops>(L, 1).ptr->
        get_color_stops();
    if (i >= 1 && i <= static_cast<int>(stops.size())) {
        rvg_lua_push<color_stop>(L, stops[i-1]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int const_ramp_color_stops__len(lua_State *L) {
    lua_Integer n = static_cast<lua_Integer>(
        rvg_lua_check<const_ramp_color_stops>(L, 1).ptr->
            get_color_stops().size());
    lua_pushinteger(L, n);
    return 1;
}

static luaL_Reg const_ramp_color_stops_meta[] = {
    {"__index", const_ramp_color_stops__index},
    {"__len", const_ramp_color_stops__len},
    {nullptr, nullptr}
};

static int const_color_ramp_get_spread(lua_State *L) {
    rvg_lua_enum_push<e_spread>(L,
        rvg_lua_check<color_ramp::const_ptr>(L, 1)->get_spread());
    return 1;
}

static int const_color_ramp_get_color_stops(lua_State *L) {
    rvg_lua_push<const_ramp_color_stops>(L,
        const_ramp_color_stops{rvg_lua_check<color_ramp::const_ptr>(L, 1)});
    return 1;
}

static luaL_Reg const_color_ramp__index[] = {
    {"get_spread", const_color_ramp_get_spread },
    {"get_color_stops", const_color_ramp_get_color_stops },
    { nullptr, nullptr }
};

color_ramp::const_ptr rvg_lua_color_ramp_create(lua_State *L, int base) {
	int top = lua_gettop(L);
	int n = top-base+1;
    e_spread spread = rvg_lua_check<e_spread>(L, base);
    luaL_checktype(L, base+1, LUA_TTABLE);
    if (n > 2) luaL_error(L, "too many arguments (needed 2)");
    n = rvg_lua_len(L, base+1);
    auto get = [prev=0.f](lua_State *L, int tab, int item) mutable {
		lua_rawgeti(L, tab, item);
		if (lua_type(L, -1) != LUA_TTABLE)
			luaL_error(L, "index %d not a stop", item);
		lua_rawgeti(L, -1, 1);
		if (lua_type(L, -1) != LUA_TNUMBER)
			luaL_error(L, "invalid offset at stop %d", item);
        auto cur = rvg_lua_tofloat(L, -1);
        if (cur < prev || cur > 1.f)
			luaL_error(L, "invalid offset %f at stop %d", cur, item);
        prev = cur;
		lua_rawgeti(L, -2, 2);
		if (!rvg_lua_is<RGBA8>(L, -1))
			luaL_error(L, "invalid color at stop %d", item);
		color_stop s{cur, rvg_lua_to<RGBA8>(L, -1)};
		lua_pop(L, 3);
		return s;
	};
    auto range = rvg_lua_make_table_range<color_stop>(L, base+1, 1, n+1, get);
    return make_intrusive<color_ramp>(spread, range.first, range.second);
}

static int create(lua_State *L) {
    return rvg_lua_push<color_ramp::const_ptr>(L,
		rvg_lua_color_ramp_create(L, 1));
}

static const luaL_Reg mod_color_ramp[] = {
    {"color_ramp", create},
    {nullptr, nullptr}
};

int rvg_lua_color_ramp_init(lua_State *L, int ctxidx) {
    rvg_lua_rgba_init(L, ctxidx);
    rvg_lua_spread_init(L, ctxidx);
    if (!rvg_lua_typeexists<const_ramp_color_stops>(L, ctxidx)) {
        rvg_lua_createtype<const_ramp_color_stops>(L,
            "const color_stops", ctxidx, const_ramp_color_stops_meta);
    }
    if (!rvg_lua_typeexists<color_stop>(L, ctxidx)) {
        rvg_lua_createtype<color_stop>(L, "color_stop", ctxidx);
        rvg_lua_setmethods<color_stop>(L, color_stop__index, 0, ctxidx);
    }
    if (!rvg_lua_typeexists<color_ramp::const_ptr>(L, ctxidx)) {
		rvg_lua_createtype<color_ramp::const_ptr>(L,
			"const color_ramp", ctxidx);
		rvg_lua_setmethods<color_ramp::const_ptr>(L,
            const_color_ramp__index, 0, ctxidx);
    }
	return 0;
}

int rvg_lua_color_ramp_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_color_ramp_init(L, ctxidx);
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_color_ramp, 1); // mod
    return 0;
}
