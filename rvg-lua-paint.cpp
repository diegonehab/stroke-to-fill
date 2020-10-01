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
#include "rvg-lua-xform.h"
#include "rvg-lua-xformable.h"
#include "rvg-lua-linear-gradient-data.h"
#include "rvg-lua-radial-gradient-data.h"
#include "rvg-lua-texture-data.h"

#include "rvg-lua-paint.h"

using namespace rvg;

template <>
int rvg_lua_tostring<paint>(lua_State *L) {
    switch (rvg_lua_to<paint>(L, 1).get_type()) {
        case paint::e_type::solid_color:
            lua_pushliteral(L, "paint{solid_color}");
            return 1;
        case paint::e_type::linear_gradient:
            lua_pushliteral(L, "paint{linear_gradient}");
            return 1;
        case paint::e_type::radial_gradient:
            lua_pushliteral(L, "paint{radial_gradient}");
            return 1;
        case paint::e_type::texture:
            lua_pushliteral(L, "paint{texture}");
            return 1;
        case paint::e_type::empty:
            lua_pushliteral(L, "paint{empty}");
            return 1;
        default:
            lua_pushliteral(L, "paint{unknown}");
            return 1;
    }
}

static int paint_get_type(lua_State *L) {
    rvg_lua_enum_push<paint::e_type>(L, rvg_lua_check<paint>(L, 1).get_type());
    return 1;
}

static int paint_get_texture_data(lua_State *L) {
    const auto &p = rvg_lua_check<paint>(L, 1);
    if (p.is_texture()) {
        rvg_lua_push<texture_data::const_ptr>(L, p.get_texture_data_ptr());
    } else {
        luaL_error(L, "paint is not a texture");
    }
    return 1;
}

static int paint_get_opacity(lua_State *L) {
    lua_pushnumber(L, unorm<double>{
        rvg_lua_check<paint>(L, 1).get_opacity()});
    return 1;
}

static int paint_get_solid_color(lua_State *L) {
    const auto &p = rvg_lua_check<paint>(L, 1);
    if (p.is_solid_color()) {
        rvg_lua_push<RGBA8>(L, p.get_solid_color());
    } else {
        luaL_error(L, "paint is not a solid_color");
    }
    return 1;
}

static int paint_get_linear_gradient_data(lua_State *L) {
    const auto &p = rvg_lua_check<paint>(L, 1);
    if (p.is_linear_gradient()) {
        rvg_lua_push<linear_gradient_data::const_ptr>(L,
            p.get_linear_gradient_data_ptr());
    } else {
        luaL_error(L, "paint is not a linear_gradient");
    }
    return 1;
}

static int paint_get_radial_gradient_data(lua_State *L) {
    const auto &p = rvg_lua_check<paint>(L, 1);
    if (p.is_radial_gradient()) {
        rvg_lua_push<radial_gradient_data::const_ptr>(L,
            p.get_radial_gradient_data_ptr());
    } else {
        luaL_error(L, "paint is not a radial_gradient");
    }
    return 1;
}

static int paint_get_key(lua_State *L) {
    const auto &pa = rvg_lua_check<paint>(L, 1);
    uint8_t o = pa.get_opacity();
    auto t = pa.get_type();
    if (t == paint::e_type::solid_color) {
        auto c = pa.get_solid_color();
        constexpr int key_size = sizeof(c)+sizeof(o);
        char key[key_size];
        memcpy(key, &c, sizeof(c));
        key[sizeof(c)] = o;
        lua_pushlstring(L, key, sizeof(key));
        return 1;
    } else {
        const void *p = nullptr;
        switch (t) {
            case paint::e_type::linear_gradient:
                p = pa.get_linear_gradient_data_ptr().get();
                break;
            case paint::e_type::radial_gradient:
                p = pa.get_radial_gradient_data_ptr().get();
                break;
            case paint::e_type::texture:
                p = pa.get_texture_data_ptr().get();
                break;
            default:
                break;
        }
        constexpr int key_size = sizeof(xform)+sizeof(p)+sizeof(o);
        char key[key_size];
        memset(key, 0, sizeof(key));
        int offset = 0;
        memcpy(key+offset, &pa.get_xf(), sizeof(xform));
        offset += sizeof(xform);
        memcpy(key+offset, &p, sizeof(p));
        offset += sizeof(p);
        key[offset] = o;
        lua_pushlstring(L, key, sizeof(key));
        return 1;
    }
}

static luaL_Reg paint__index[] = {
    {"get_type", paint_get_type },
    {"get_key", paint_get_key },
    {"get_texture_data", paint_get_texture_data },
    {"get_opacity", paint_get_opacity },
    {"get_solid_color", paint_get_solid_color },
    {"get_linear_gradient_data", paint_get_linear_gradient_data },
    {"get_radial_gradient_data", paint_get_radial_gradient_data },
    { nullptr, nullptr }
};

static rvg_lua_named_value<paint::e_type> named_paint_types[] = {
    {"solid_color", paint::e_type::solid_color},
    {"linear_gradient", paint::e_type::linear_gradient},
    {"radial_gradient", paint::e_type::radial_gradient},
    {"texture", paint::e_type::texture},
    {"empty", paint::e_type::empty},
    { nullptr, paint::e_type::empty}
};

static int create_texture(lua_State *L) {
    unorm8 opacity = unorm<float>::clamped(rvg_lua_optfloat(L, 3, 1.f));
    rvg_lua_push<paint>(L, paint{rvg_lua_texture_data_create(L, 1),
        opacity});
    return 1;
}

static int create_linear_gradient(lua_State *L) {
    unorm8 opacity = unorm<float>::clamped(rvg_lua_optfloat(L, 6, 1.f));
    rvg_lua_push<paint>(L, paint{rvg_lua_linear_gradient_data_create(L, 1),
        opacity});
    return 1;
}

static int create_radial_gradient(lua_State *L) {
    unorm8 opacity = unorm<float>::clamped(rvg_lua_optfloat(L, 7, 1.f));
    rvg_lua_push<paint>(L, paint{rvg_lua_radial_gradient_data_create(L, 1),
        opacity});
    return 1;
}

static int create_solid_color(lua_State *L) {
    RGBA8 rgba8 = rvg_lua_check<RGBA8>(L, 1);
    unorm8 opacity = unorm<float>::clamped(
        rvg_lua_optfloat(L, 2, 1.f));
    rvg_lua_push<paint>(L, paint{rgba8, opacity});
    return 1;
}

static const luaL_Reg mod_paint[] = {
    {"solid_color", create_solid_color},
    {"linear_gradient", create_linear_gradient},
    {"radial_gradient", create_radial_gradient},
    {"texture", create_texture},
    {nullptr, nullptr}
};

int rvg_lua_paint_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_rgba_init(L, ctxidx);
    rvg_lua_linear_gradient_data_init(L, ctxidx);
    rvg_lua_radial_gradient_data_init(L, ctxidx);
    rvg_lua_texture_data_init(L, ctxidx);
    if (!rvg_lua_typeexists<paint::e_type>(L, ctxidx)) {
        rvg_lua_enum_createtype<paint::e_type>(L, "paint_type",
            named_paint_types, ctxidx);
    }
    if (!rvg_lua_typeexists<paint>(L, ctxidx)) {
        rvg_lua_createtype<paint>(L, "paint", ctxidx);
        rvg_lua_set_xformable<paint>(L, ctxidx);
        rvg_lua_setmethods<paint>(L, paint__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_paint_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_paint_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_paint, 1); // mod
    rvg_lua_enum_pushmap<paint::e_type>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "paint_type"); // mod
    return 0;
}

