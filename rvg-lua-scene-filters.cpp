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
#include <cmath>

#include "rvg-lua.h"
#include "rvg-lua-xform.h"
#include "rvg-lua-scene-filters.h"
#include "rvg-stroke-method.h"
#include "rvg-scene-f-to-lua-scene.h"
#include "rvg-scene-f-spy.h"
#include "rvg-scene-f-stroke.h"

using namespace rvg;

template <typename P>
static int lua_scene_f_painted_shape(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->painted_shape(
        rvg_lua_check<e_winding_rule>(L, 2),
        rvg_lua_check<shape>(L, 3),
        rvg_lua_check<paint>(L, 4));
    return 0;
}

template <typename P>
static int lua_scene_f_tensor_product_patch(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->tensor_product_patch(
        rvg_lua_check<patch<16,4>>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_coons_patch(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->coons_patch(
        rvg_lua_check<patch<12,4>>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_gouraud_triangle(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->gouraud_triangle(
        rvg_lua_check<patch<3,3>>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_stencil_shape(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->stencil_shape(
        rvg_lua_check<e_winding_rule>(L, 2),
        rvg_lua_check<shape>(L, 3));
    return 0;
}

template <>
unsigned short rvg_lua_check<unsigned short>(lua_State *L, int idx, int) {
    return static_cast<unsigned short>(luaL_checkinteger(L, idx));
}

template <typename P>
static int lua_scene_f_begin_transform(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->begin_transform(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_check<xform>(L, 3));
    return 0;
}

template <typename P>
static int lua_scene_f_end_transform(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->end_transform(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_check<xform>(L, 3));
    return 0;
}

template <typename P>
static int lua_scene_f_begin_blur(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->begin_blur(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_checkfloat(L, 3));
    return 0;
}

template <typename P>
static int lua_scene_f_end_blur(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->end_blur(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_checkfloat(L, 3));
    return 0;
}

template <>
unorm<float> rvg_lua_check<unorm<float>>(lua_State *L, int idx, int) {
    return unorm<float>::clamped(rvg_lua_checkfloat(L, idx));
}

template <typename P>
static int lua_scene_f_begin_fade(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->begin_fade(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_check<unorm<float>>(L, 3));
    return 0;
}

template <typename P>
static int lua_scene_f_end_fade(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->end_fade(
        rvg_lua_check<unsigned short>(L, 2),
        rvg_lua_check<unorm<float>>(L, 3));
    return 0;
}

template <typename P>
static int lua_scene_f_begin_clip(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->begin_clip(
        rvg_lua_check<unsigned short>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_activate_clip(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->activate_clip(
        rvg_lua_check<unsigned short>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_end_clip(lua_State *L) {
    rvg_lua_check_pointer<P>(L, 1)->end_clip(
        rvg_lua_check<unsigned short>(L, 2));
    return 0;
}

template <typename P>
static int lua_scene_f_init(lua_State *L, const char *scene_f_name,
    int ctxidx) {
    static luaL_Reg lua_scene_f__index[] = {
        {"painted_shape", &lua_scene_f_painted_shape<P>},
        {"tensor_product_patch", &lua_scene_f_tensor_product_patch<P>},
        {"coons_patch", &lua_scene_f_coons_patch<P>},
        {"gouraud_triangle", &lua_scene_f_gouraud_triangle<P>},
        {"stencil_shape", &lua_scene_f_stencil_shape<P>},
        {"begin_transform", &lua_scene_f_begin_transform<P>},
        {"end_transform", &lua_scene_f_end_transform<P>},
        {"begin_clip", &lua_scene_f_begin_clip<P>},
        {"activate_clip", &lua_scene_f_activate_clip<P>},
        {"end_clip", &lua_scene_f_end_clip<P>},
        {"begin_fade", &lua_scene_f_begin_fade<P>},
        {"end_fade", &lua_scene_f_end_fade<P>},
        {"begin_blur", &lua_scene_f_begin_blur<P>},
        {"end_blur", &lua_scene_f_end_blur<P>},
        { nullptr, nullptr }
    };

    if (!rvg_lua_typeexists<P>(L, ctxidx)) {
        rvg_lua_createtype<P>(L, scene_f_name, ctxidx);
        rvg_lua_setmethods<P>(L, lua_scene_f__index, 0, ctxidx);
    }

    return 0;
}

static auto make_lua_scene_f_spy_snk(lua_State *L) {
    return make_scene_f_spy(luaL_checkstring(L, 1));
}

static auto make_lua_scene_f_spy_fwd(lua_State *L) {
    return make_scene_f_spy(luaL_checkstring(L, 1),
        make_scene_f_to_lua_scene_ref(L, 2));
}

static int filter_make_scene_f_spy(lua_State *L) {
    if (lua_gettop(L) < 2) {
        return rvg_lua_push(L, make_lua_scene_f_spy_snk(L));
    } else {
        return rvg_lua_push(L, make_lua_scene_f_spy_fwd(L));
    }
}

static auto make_lua_scene_f_stroke(lua_State *L) {
    if (lua_gettop(L) > 2) {
        return make_scene_f_stroke(rvg_lua_check<e_stroke_method>(L, 1),
            lua_toboolean(L, 2), make_scene_f_to_lua_scene_ref(L, 3));
    } else {
        return make_scene_f_stroke(rvg_lua_check<e_stroke_method>(L, 1),
            make_scene_f_to_lua_scene_ref(L, 2));
    }
}

static int filter_make_scene_f_stroke(lua_State *L) {
    return rvg_lua_push(L, make_lua_scene_f_stroke(L));
}

static const luaL_Reg modfilter[] = {
    {"make_scene_f_spy", filter_make_scene_f_spy},
    {"make_scene_f_stroke", filter_make_scene_f_stroke},
    {NULL, NULL}
};

int rvg_lua_scene_filters_init(lua_State *L, int ctxidx) {

    lua_scene_f_init<
        decltype(make_lua_scene_f_spy_snk(nullptr))
    >(L, "scene_f_spy sink", ctxidx);

    lua_scene_f_init<
        decltype(make_lua_scene_f_spy_fwd(nullptr))
    >(L, "scene_f_spy forwarder", ctxidx);

    lua_scene_f_init<
        decltype(make_lua_scene_f_stroke(nullptr))
    >(L, "scene_f_stroke", ctxidx);


    return 0;
}

int rvg_lua_scene_filters_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_scene_filters_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, modfilter, 1); // mod
    return 1;
}
