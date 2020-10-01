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
#include "rvg-lua-xformable.h"
#include "rvg-lua-stroke-style.h"
#include "rvg-lua-strokable.h"
#include "rvg-lua-circle-data.h"
#include "rvg-lua-polygon-data.h"
#include "rvg-lua-path-data.h"
#include "rvg-lua-rect-data.h"
#include "rvg-lua-triangle-data.h"

#include "rvg-lua-shape.h"

using namespace rvg;

static int const_shape_stroke_data_get_shape(lua_State *L) {
    const auto &s = rvg_lua_check<shape::stroke_data>(L, 1);
    rvg_lua_push<shape>(L, s.get_shape());
    return 1;
}

static int const_shape_stroke_data_get_style(lua_State *L) {
    const auto &s = rvg_lua_check<shape::stroke_data>(L, 1);
    rvg_lua_push<stroke_style::const_ptr>(L, s.get_style_ptr());
    return 1;
}

static int const_shape_stroke_data_get_width(lua_State *L) {
    const auto &s = rvg_lua_check<shape::stroke_data>(L, 1);
    lua_pushnumber(L, s.get_width());
    return 1;
}

static luaL_Reg const_shape_stroke_data__index[] = {
    {"get_shape", const_shape_stroke_data_get_shape},
    {"get_style", const_shape_stroke_data_get_style},
    {"get_width", const_shape_stroke_data_get_width},
    { nullptr, nullptr }
};

static rvg_lua_named_value<shape::e_type> named_shape_types[] = {
    {"path", shape::e_type::path},
    {"circle", shape::e_type::circle},
    {"triangle", shape::e_type::triangle},
    {"rect", shape::e_type::rect},
    {"polygon", shape::e_type::polygon},
    {"blend", shape::e_type::blend},
    {"stroke", shape::e_type::stroke},
    {"empty", shape::e_type::empty},
    { nullptr, shape::e_type::empty}
};

static int as_path_data(lua_State *L) {
    rvg_lua_push<path_data::const_ptr>(L,
        rvg_lua_check<shape>(L, 1).as_path_data_ptr(
            rvg_lua_opt<xform>(L, 2, identity{})));
    return 1;
}

static int get_type(lua_State *L) {
    rvg_lua_enum_push<shape::e_type>(L, rvg_lua_check<shape>(L, 1).get_type());
    return 1;
}

static int get_path_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_path()) {
        rvg_lua_push<path_data::const_ptr>(L, s.get_path_data_ptr());
    } else {
        luaL_error(L, "shape is not a path");
    }
    return 1;
}

static int get_circle_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_circle()) {
        rvg_lua_push<circle_data::const_ptr>(L, s.get_circle_data_ptr());
    } else {
        luaL_error(L, "shape is not a circle");
    }
    return 1;
}

static int get_rect_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_rect()) {
        rvg_lua_push<rect_data::const_ptr>(L, s.get_rect_data_ptr());
    } else {
        luaL_error(L, "shape is not a rect");
    }
    return 1;
}

static int get_polygon_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_polygon()) {
        rvg_lua_push<polygon_data::const_ptr>(L, s.get_polygon_data_ptr());
    } else {
        luaL_error(L, "shape is not a polygon");
    }
    return 1;
}

static int get_triangle_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_triangle()) {
        rvg_lua_push<triangle_data::const_ptr>(L, s.get_triangle_data_ptr());
    } else {
        luaL_error(L, "shape is not a triangle");
    }
    return 1;
}

static int get_stroke_data(lua_State *L) {
    const auto &s = rvg_lua_check<shape>(L, 1);
    if (s.is_stroke()) {
        rvg_lua_push<shape::stroke_data>(L, s.get_stroke_data());
    } else {
        luaL_error(L, "shape is not a stroke");
    }
    return 1;
}

template <>
int rvg_lua_tostring<shape>(lua_State *L) {
    switch (rvg_lua_to<shape>(L, 1).get_type()) {
        case shape::e_type::polygon:
            lua_pushliteral(L, "shape{polygon}");
            return 1;
        case shape::e_type::rect:
            lua_pushliteral(L, "shape{rect}");
            return 1;
        case shape::e_type::triangle:
            lua_pushliteral(L, "shape{triangle}");
            return 1;
        case shape::e_type::path:
            lua_pushliteral(L, "shape{path}");
            return 1;
        case shape::e_type::circle:
            lua_pushliteral(L, "shape{circle}");
            return 1;
        case shape::e_type::empty:
            lua_pushliteral(L, "shape{empty}");
            return 1;
        case shape::e_type::stroke:
            lua_pushliteral(L, "shape{stroke}");
            return 1;
        case shape::e_type::blend:
            lua_pushliteral(L, "shape{blend}");
            return 1;
        default:
            lua_pushliteral(L, "shape{unknown}");
            return 1;
    }
}

static int stroked(lua_State *L) {
    if (lua_isnoneornil(L, 3))
        rvg_lua_push<shape>(L, rvg_lua_check<shape>(L, 1).stroked(
            rvg_lua_checkfloat(L, 2)));
    else
        rvg_lua_push<shape>(L, rvg_lua_check<shape>(L, 1).stroked(
            rvg_lua_checkfloat(L, 2),
            rvg_lua_check<stroke_style::const_ptr>(L, 3)));
    return 1;
}

static int is_stroke(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_stroke());
    return 1;
}

static int is_rect(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_rect());
    return 1;
}

static int is_path(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_path());
    return 1;
}

static int is_triangle(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_triangle());
    return 1;
}

static int is_circle(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_circle());
    return 1;
}

static int is_polygon(lua_State *L) {
    lua_pushboolean(L, rvg_lua_check<shape>(L, 1).is_polygon());
    return 1;
}

static int get_key(lua_State *L) {
    const auto &sh = rvg_lua_check<shape>(L, 1);
    const void *p = nullptr;
    float width = 0.f;
    switch (sh.get_type()) {
        case shape::e_type::polygon:
            p = sh.get_polygon_data_ptr().get();
            break;
        case shape::e_type::rect:
            p = sh.get_rect_data_ptr().get();
            break;
        case shape::e_type::triangle:
            p = sh.get_triangle_data_ptr().get();
            break;
        case shape::e_type::path:
            p = sh.get_path_data_ptr().get();
            break;
        case shape::e_type::circle:
            p = sh.get_circle_data_ptr().get();
            break;
        case shape::e_type::stroke:
            p = sh.get_stroke_data().get_shape_ptr().get();
            break;
        default:
            break;
    }
    constexpr int key_size = sizeof(xform)+2*sizeof(p)+sizeof(width);
    char key[key_size];
    memset(key, 0, sizeof(key));
    int offset = 0;
    memcpy(key+offset, &sh.get_xf(), sizeof(xform));
    offset += sizeof(xform);
    memcpy(key+offset, &p, sizeof(p));
    offset += sizeof(p);
    if (sh.get_type() == shape::e_type::stroke) {
        p = sh.get_stroke_data().get_style_ptr().get();
        memcpy(key+offset, &p, sizeof(p));
        offset += sizeof(p);
        width = sh.get_stroke_data().get_width();
        memcpy(key+offset, &width, sizeof(width));
        offset += sizeof(width);
    }
    lua_pushlstring(L, key, offset);
    return 1;
}

static luaL_Reg shape__index[] = {
    {"get_key", get_key },
    {"stroked", stroked },
    {"is_stroke", is_stroke },
    {"is_path", is_path },
    {"is_rect", is_rect },
    {"is_triangle", is_triangle },
    {"is_circle", is_circle },
    {"is_polygon", is_polygon },
    {"get_type", get_type },
    {"get_path_data", get_path_data },
    {"get_rect_data", get_rect_data },
    {"get_triangle_data", get_triangle_data },
    {"get_circle_data", get_circle_data },
    {"get_polygon_data", get_polygon_data },
    {"get_stroke_data", get_stroke_data },
    {"as_path_data", as_path_data },
    { nullptr, nullptr }
};

static int create_triangle(lua_State *L) {
    return rvg_lua_push<shape>(L, shape{rvg_lua_triangle_data_create(L, 1)});
}

static int create_circle(lua_State *L) {
    return rvg_lua_push<shape>(L, shape{rvg_lua_circle_data_create(L, 1)});
}

static int create_path(lua_State *L) {
    return rvg_lua_push<shape>(L, shape{rvg_lua_path_data_create(L, 1)});
}

static int create_rect(lua_State *L) {
    return rvg_lua_push<shape>(L, shape{rvg_lua_rect_data_create(L, 1)});
}

static int create_polygon(lua_State *L) {
    return rvg_lua_push<shape>(L, shape{rvg_lua_polygon_data_create(L, 1)});
}

static const luaL_Reg mod_shape[] = {
    {"triangle", create_triangle},
    {"circle", create_circle},
    {"rect", create_rect},
    {"polygon", create_polygon},
    {"path", create_path},
    {nullptr, nullptr}
};

struct shape_strokable_deref {
    const shape &operator()(const shape &s) {
        return s;
    }
};

struct shape_strokable_push {
    int operator()(lua_State *L, shape &&s) {
        return rvg_lua_push<shape>(L, std::move(s));
    }
};

int rvg_lua_shape_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);
    rvg_lua_path_data_init(L, ctxidx);
    rvg_lua_circle_data_init(L, ctxidx);
    rvg_lua_triangle_data_init(L, ctxidx);
    rvg_lua_polygon_data_init(L, ctxidx);
    rvg_lua_rect_data_init(L, ctxidx);
    rvg_lua_stroke_style_init(L, ctxidx);
    if (!rvg_lua_typeexists<shape::e_type>(L, ctxidx)) {
        rvg_lua_enum_createtype<shape::e_type>(L, "shape_type",
            named_shape_types, ctxidx);
    }
    if (!rvg_lua_typeexists<shape::stroke_data>(L, ctxidx)) {
        rvg_lua_createtype<shape::stroke_data>(L,
            "const stroke_data", ctxidx);
        rvg_lua_setmethods<shape::stroke_data>(L,
            const_shape_stroke_data__index, 0, ctxidx);
    }
    if (!rvg_lua_typeexists<shape>(L, ctxidx)) {
        rvg_lua_createtype<shape>(L, "shape", ctxidx);
        rvg_lua_set_xformable<shape>(L, ctxidx);
        rvg_lua_set_strokable<shape, shape_strokable_deref,
            shape_strokable_push>(L, ctxidx);
        rvg_lua_setmethods<shape>(L, shape__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_shape_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_shape_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_shape, 1); // mod
    rvg_lua_enum_pushmap<shape::e_type>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "shape_type"); // mod
    return 0;
}
