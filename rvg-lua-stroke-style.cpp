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
#include "rvg-lua-stroke-style.h"
#include "rvg-lua-strokable.h"
#include "rvg-stroke-method.h"

using namespace rvg;

static int const_stroke_dashes__index(lua_State *L) {
    int n = static_cast<int>(luaL_checkinteger(L, 2));
    const auto &dashes = *rvg_lua_check<stroke_dashes::const_ptr>(L, 1);
    if (n >= 1 && n <= static_cast<int>(dashes.size())) {
        lua_pushnumber(L, dashes[n-1]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int const_stroke_dashes__len(lua_State *L) {
    lua_pushinteger(L, static_cast<lua_Integer>(
        rvg_lua_check<stroke_dashes::const_ptr>(L, 1)->size()));
    return 1;
}

static luaL_Reg const_stroke_dashes_meta[] = {
    {"__index", const_stroke_dashes__index},
    {"__len", const_stroke_dashes__len},
    {nullptr, nullptr}
};

static int const_stroke_style_get_dashes(lua_State *L) {
    rvg_lua_push<stroke_dashes::const_ptr>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_dashes_ptr());
	return 1;
}

static int const_stroke_style_get_join(lua_State *L) {
	rvg_lua_enum_push<e_stroke_join>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_join());
	return 1;
}

static int const_stroke_style_get_inner_join(lua_State *L) {
	rvg_lua_enum_push<e_stroke_join>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_inner_join());
	return 1;
}

static int const_stroke_style_get_initial_cap(lua_State *L) {
	rvg_lua_enum_push<e_stroke_cap>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_initial_cap());
	return 1;
}

static int const_stroke_style_get_terminal_cap(lua_State *L) {
	rvg_lua_enum_push<e_stroke_cap>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_terminal_cap());
	return 1;
}

static int const_stroke_style_get_dash_initial_cap(lua_State *L) {
	rvg_lua_enum_push<e_stroke_cap>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_dash_initial_cap());
	return 1;
}

static int const_stroke_style_get_dash_terminal_cap(lua_State *L) {
	rvg_lua_enum_push<e_stroke_cap>(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_dash_terminal_cap());
	return 1;
}

static int const_stroke_style_get_miter_limit(lua_State *L) {
	lua_pushnumber(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_miter_limit());
	return 1;
}

static int const_stroke_style_get_dash_offset(lua_State *L) {
	lua_pushnumber(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_dash_offset());
	return 1;
}

static int const_stroke_style_get_resets_on_move(lua_State *L) {
	lua_pushboolean(L,
        rvg_lua_check<stroke_style::const_ptr>(L, 1)->get_resets_on_move());
	return 1;
}

static luaL_Reg const_stroke_style__index[] = {
    {"get_dashes", const_stroke_style_get_dashes },
    {"get_join", const_stroke_style_get_join },
    {"get_inner_join", const_stroke_style_get_inner_join },
    {"get_initial_cap", const_stroke_style_get_initial_cap },
    {"get_terminal_cap", const_stroke_style_get_terminal_cap },
    {"get_dash_initial_cap", const_stroke_style_get_dash_initial_cap },
    {"get_dash_terminal_cap", const_stroke_style_get_dash_terminal_cap },
    {"get_miter_limit", const_stroke_style_get_miter_limit },
    {"get_dash_offset", const_stroke_style_get_dash_offset },
    {"get_resets_on_move", const_stroke_style_get_resets_on_move },
    { nullptr, nullptr }
};

static void set_stroke_style_defaults(lua_State *L, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    rvg_lua_gettypemetatable<stroke_style::const_ptr>(L, ctxidx);
    lua_getfield(L, -1, "__index");
    rvg_lua_enum_push<e_stroke_cap>(L, stroke_style::default_cap, ctxidx);
    lua_setfield(L, -2, "default_cap");
    rvg_lua_enum_push<e_stroke_join>(L, stroke_style::default_join, ctxidx);
    lua_setfield(L, -2, "default_join");
    rvg_lua_enum_push<e_stroke_join>(L, stroke_style::default_inner_join,
        ctxidx);
    lua_setfield(L, -2, "default_inner_join");
    lua_pushnumber(L, stroke_style::default_miter_limit);
    lua_setfield(L, -2, "default_miter_limit");
    lua_pushnumber(L, stroke_style::default_dash_offset);
    lua_setfield(L, -2, "default_dash_offset");
    lua_pushboolean(L, stroke_style::default_resets_on_move);
    lua_setfield(L, -2, "default_resets_on_move");
    lua_pop(L, 2);
}

static rvg_lua_named_value<e_stroke_cap> named_stroke_caps[] = {
    {"butt", e_stroke_cap::butt},
    {"round", e_stroke_cap::round},
    {"square", e_stroke_cap::square},
    {"triangle", e_stroke_cap::triangle},
    {"fletching", e_stroke_cap::fletching},
    { nullptr, e_stroke_cap::butt},
};

static rvg_lua_named_value<e_stroke_join> named_stroke_joins[] = {
    {"arcs", e_stroke_join::arcs},
    {"miter_clip", e_stroke_join::miter_clip},
    {"miter_or_bevel", e_stroke_join::miter_or_bevel},
    {"round", e_stroke_join::round},
    {"bevel", e_stroke_join::bevel},
    { nullptr, e_stroke_join::miter_or_bevel},
};

static rvg_lua_named_value<e_stroke_method> named_stroke_methods[] = {
    { "native", e_stroke_method::native},
#ifdef STROKER_RVG
    { "rvg", e_stroke_method::rvg},
#endif
#ifdef STROKER_LIVAROT
    { "livarot_stroke", e_stroke_method::livarot_stroke},
    { "livarot_outline", e_stroke_method::livarot_outline},
#endif
#ifdef STROKER_DIRECT2D
    { "direct2d", e_stroke_method::direct2d},
#endif
#ifdef STROKER_QUARTZ
    { "quartz", e_stroke_method::quartz},
#endif
#ifdef STROKER_AGG
    { "agg", e_stroke_method::agg},
#endif
#ifdef STROKER_SKIA
    { "skia", e_stroke_method::skia},
#endif
#ifdef STROKER_QT5
    { "qt5", e_stroke_method::qt5},
#endif
#ifdef STROKER_MUPDF
    { "mupdf", e_stroke_method::mupdf},
#endif
#ifdef STROKER_CAIRO
    { "cairo_traps", e_stroke_method::cairo_traps},
    { "cairo_polygon", e_stroke_method::cairo_polygon},
    { "cairo_tristrip", e_stroke_method::cairo_tristrip},
#endif
#ifdef STROKER_GS
    { "gs", e_stroke_method::gs},
    { "gs_compat", e_stroke_method::gs_compat},
    { "gs_fast", e_stroke_method::gs_fast},
#endif
#ifdef STROKER_OPENVG_RI
    { "openvg_ri", e_stroke_method::openvg_ri},
#endif
    { nullptr, e_stroke_method::native},
};

rvg::stroke_style::ptr rvg_lua_stroke_style_create(lua_State *L, int base) {
    (void) base; (void) L;
    return make_intrusive<stroke_style>();
}

static int create(lua_State *L) {
    return rvg_lua_push<stroke_style::const_ptr>(L,
        rvg_lua_stroke_style_create(L, 1));
}

static const luaL_Reg mod_stroke_style[] = {
    {"stroke_style", create},
    {NULL, NULL}
};

struct stroke_style_strokable_deref {
    const stroke_style &operator()(stroke_style::const_ptr s) {
        return *s;
    }
};

struct stroke_style_strokable_push {
    int operator()(lua_State *L, stroke_style &&s) {
        return rvg_lua_push<stroke_style::const_ptr>(L,
            make_intrusive<stroke_style>(std::move(s)));
    }
};

int rvg_lua_stroke_style_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<e_stroke_cap>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_stroke_cap>(L, "stroke_cap",
            named_stroke_caps, ctxidx);
    }
    if (!rvg_lua_typeexists<e_stroke_join>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_stroke_join>(L, "stroke_join",
            named_stroke_joins, ctxidx);
    }
    if (!rvg_lua_typeexists<e_stroke_method>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_stroke_method>(L, "stroke_method",
            named_stroke_methods, ctxidx);
    }
    if (!rvg_lua_typeexists<stroke_dashes::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<stroke_dashes::const_ptr>(L,
            "const stroke_dashes", ctxidx, const_stroke_dashes_meta);
    }
    if (!rvg_lua_typeexists<stroke_style::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<stroke_style::const_ptr>(L,
            "const stroke_style", ctxidx);
            rvg_lua_set_strokable<
                stroke_style::const_ptr,
                stroke_style_strokable_deref,
                stroke_style_strokable_push>(L, ctxidx);
            rvg_lua_setmethods<stroke_style::const_ptr>(L,
                const_stroke_style__index, 0, ctxidx);
            set_stroke_style_defaults(L, ctxidx);
    }
    return 0;
}

int rvg_lua_stroke_style_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_stroke_style_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_stroke_style, 1); // mod
    rvg_lua_enum_pushmap<e_stroke_cap>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "stroke_cap"); // mod
    rvg_lua_enum_pushmap<e_stroke_join>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "stroke_join"); // mod
    rvg_lua_enum_pushmap<e_stroke_method>(L, ctxidx); // mod map
    rvg_lua_readonlyproxy(L); // mod mapproxy
    lua_setfield(L, -2, "stroke_method"); // mod
    return 0;
}
