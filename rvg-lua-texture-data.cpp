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
#include "rvg-lua-spread.h"
#include "rvg-lua-image.h"

#include "rvg-lua-texture-data.h"

using namespace rvg;

static int const_texture_data_get_image(lua_State *L) {
    rvg_lua_push<i_image::const_ptr>(L,
        rvg_lua_check<texture_data::const_ptr>(L, 1)->get_image_ptr());
    return 1;
}

static int const_texture_data_get_spread(lua_State *L) {
    rvg_lua_enum_push<e_spread>(L,
        rvg_lua_check<texture_data::const_ptr>(L, 1)->get_spread());
    return 1;
}

static luaL_Reg const_texture_data__index[] = {
    {"get_image", const_texture_data_get_image },
    {"get_spread", const_texture_data_get_spread },
    { nullptr, nullptr }
};

texture_data::ptr rvg_lua_texture_data_create(lua_State *L, int base) {
    e_spread spread = rvg_lua_check<e_spread>(L, base+0);
    auto img_ptr = rvg_lua_check_either<
        i_image::const_ptr,
        i_image::ptr
    >(L, base+1);
    return make_intrusive<texture_data>(spread, img_ptr);
}

static int create(lua_State *L) {
    return rvg_lua_push<texture_data::const_ptr>(L,
        rvg_lua_texture_data_create(L, 1));
}

static const luaL_Reg mod_texture_data[] = {
    {"texture_data", create},
    {nullptr, nullptr}
};

int rvg_lua_texture_data_init(lua_State *L, int ctxidx) {
    rvg_lua_spread_init(L, ctxidx);
    rvg_lua_require(L, "image"); lua_pop(L, 1);
    if (!rvg_lua_typeexists<texture_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<texture_data::const_ptr>(L,
            "const texture_data", ctxidx);
        rvg_lua_setmethods<texture_data::const_ptr>(L,
            const_texture_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_texture_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_texture_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_texture_data, 1); // mod
    return 0;
}
