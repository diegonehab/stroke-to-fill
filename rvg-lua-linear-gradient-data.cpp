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
#include "rvg-lua-color-ramp.h"
#include "rvg-lua-linear-gradient-data.h"

using namespace rvg;

static int const_linear_gradient_data_get_x1(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<linear_gradient_data::const_ptr>(L, 1)->
        get_x1());
    return 1;
}

static int const_linear_gradient_data_get_y1(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<linear_gradient_data::const_ptr>(L, 1)->
        get_y1());
    return 1;
}

static int const_linear_gradient_data_get_x2(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<linear_gradient_data::const_ptr>(L, 1)->
        get_x2());
    return 1;
}

static int const_linear_gradient_data_get_y2(lua_State *L) {
    lua_pushnumber(L, rvg_lua_check<linear_gradient_data::const_ptr>(L, 1)->
        get_y2());
    return 1;
}

static int const_linear_gradient_data_get_color_ramp(lua_State *L) {
    rvg_lua_push<color_ramp::const_ptr>(L,
        rvg_lua_check<linear_gradient_data::const_ptr>(L, 1)->
            get_color_ramp_ptr());
    return 1;
}

static luaL_Reg const_linear_gradient_data__index[] = {
    {"get_x1", const_linear_gradient_data_get_x1 },
    {"get_y1", const_linear_gradient_data_get_y1 },
    {"get_x2", const_linear_gradient_data_get_x2 },
    {"get_y2", const_linear_gradient_data_get_y2 },
    {"get_color_ramp", const_linear_gradient_data_get_color_ramp },
    { nullptr, nullptr }
};

linear_gradient_data::ptr rvg_lua_linear_gradient_data_create(lua_State *L,
    int base) {
    color_ramp::const_ptr ramp_ptr = rvg_lua_check<
        color_ramp::const_ptr>(L, base+0);
    float x1 = rvg_lua_checkfloat(L, base+1);
    float y1 = rvg_lua_checkfloat(L, base+2);
    float x2 = rvg_lua_checkfloat(L, base+3);
    float y2 = rvg_lua_checkfloat(L, base+4);
    return make_intrusive<linear_gradient_data>(ramp_ptr, x1, y1, x2, y2);
}

static int create(lua_State *L) {
    rvg_lua_push<linear_gradient_data::const_ptr>(L,
        rvg_lua_linear_gradient_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_linear_gradient_data[] = {
    {"linear_gradient_data", create},
    {NULL, NULL}
};

int rvg_lua_linear_gradient_data_init(lua_State *L, int ctxidx) {
    rvg_lua_color_ramp_init(L, ctxidx);
    if (!rvg_lua_typeexists<linear_gradient_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<linear_gradient_data::const_ptr>(L,
            "const linear_gradient_data", ctxidx);
        rvg_lua_setmethods<linear_gradient_data::const_ptr>(L,
            const_linear_gradient_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_linear_gradient_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_linear_gradient_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_linear_gradient_data, 1); // mod
    return 0;
}
