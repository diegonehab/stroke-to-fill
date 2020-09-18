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
#include "rvg-unorm.h"

#include "rvg-lua-winding-rule.h"
#include "rvg-lua-shape.h"
#include "rvg-lua-paint.h"
#include "rvg-scene-f-to-lua-scene.h"
#include "rvg-lua-scene-data.h"
#include "rvg-lua-facade.h"

using namespace rvg;

template <typename PTR>
static int scene_data_riterate(lua_State *L) {
    auto p = rvg_lua_check<PTR>(L, 1);
    p->riterate(make_scene_f_to_lua_scene_in_stack(L, 2));
    return 0;
}


template <typename PTR>
static int scene_data_iterate(lua_State *L) {
    auto p = rvg_lua_check<PTR>(L, 1);
    p->iterate(make_scene_f_to_lua_scene_in_stack(L, 2));
    return 0;
}

template <typename PTR>
static int scene_data_riterate_brackets(lua_State *L) {
    auto p = rvg_lua_check<PTR>(L, 1);
    p->riterate_brackets(make_scene_f_to_lua_scene_in_stack(L, 2));
    return 0;
}


template <typename PTR>
static int scene_data_iterate_brackets(lua_State *L) {
    auto p = rvg_lua_check<PTR>(L, 1);
    p->iterate_brackets(make_scene_f_to_lua_scene_in_stack(L, 2));
    return 0;
}

static luaL_Reg const_scene_data__index[] = {
    {"iterate", &scene_data_iterate<scene_data::const_ptr> },
    {"riterate", &scene_data_riterate<scene_data::const_ptr> },
    {"iterate_brackets",
        &scene_data_iterate_brackets<scene_data::const_ptr> },
    {"riterate_brackets",
        &scene_data_riterate_brackets<scene_data::const_ptr> },
    { nullptr, nullptr }
};

static int scene_data_painted_shape(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    e_winding_rule wr = rvg_lua_check<e_winding_rule>(L, 2);
    const shape &sh = rvg_lua_check<shape>(L, 3);
    const paint &pa = rvg_lua_check<paint>(L, 4);
    se->painted_shape(wr, sh, pa);
    return 0;
}

static int scene_data_tensor_product_patch(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    const patch<16,4> &tpp = rvg_lua_check<patch<16,4>>(L, 2);
    se->tensor_product_patch(tpp);
    return 0;
}

static int scene_data_coons_patch(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    const patch<12,4> &cp = rvg_lua_check<patch<12,4>>(L, 2);
    se->coons_patch(cp);
    return 0;
}

static int scene_data_gouraud_triangle(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    const patch<3,3> &gt = rvg_lua_check<patch<3,3>>(L, 2);
    se->gouraud_triangle(gt);
    return 0;
}

static int scene_data_stencil_shape(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    e_winding_rule wr = rvg_lua_check<e_winding_rule>(L, 2);
    const shape &sh = rvg_lua_check<shape>(L, 3);
    se->stencil_shape(wr, sh);
    return 0;
}

static int scene_data_begin_transform(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    lua_Integer d = luaL_checkinteger(L, 2);
    const xform &xf = rvg_lua_check<xform>(L, 3);
    se->begin_transform(static_cast<unsigned short>(d), xf);
    return 0;
}

static int scene_data_end_transform(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    lua_Integer d = luaL_checkinteger(L, 2);
    const xform &xf = rvg_lua_check<xform>(L, 3);
    se->end_transform(static_cast<unsigned short>(d), xf);
    return 0;
}

static int scene_data_end_blur(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    lua_Integer d = luaL_checkinteger(L, 2);
    auto r = rvg_lua_checkfloat(L, 3);
    se->end_blur(static_cast<unsigned short>(d), r);
    return 0;
}

static int scene_data_begin_blur(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    lua_Integer d = luaL_checkinteger(L, 2);
    auto r = rvg_lua_checkfloat(L, 3);
    se->begin_blur(static_cast<unsigned short>(d), r);
    return 0;
}

static int scene_data_begin_fade(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    auto d = luaL_checkinteger(L, 2);
    auto o = unorm<float>::clamped(rvg_lua_checkfloat(L, 3));
    se->begin_fade(static_cast<unsigned short>(d), o);
    return 0;
}

static int scene_data_end_fade(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    auto d = luaL_checkinteger(L, 2);
    auto o = unorm<float>::clamped(rvg_lua_checkfloat(L, 3));
    se->end_fade(static_cast<unsigned short>(d), o);
    return 0;
}

static int scene_data_begin_clip(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    auto d = luaL_checkinteger(L, 2);
    se->begin_clip(static_cast<unsigned short>(d));
    return 0;
}

static int scene_data_activate_clip(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    auto d = luaL_checkinteger(L, 2);
    se->activate_clip(static_cast<unsigned short>(d));
    return 0;
}

static int scene_data_end_clip(lua_State *L) {
	auto se = rvg_lua_check<scene_data::ptr>(L, 1);
    auto d = luaL_checkinteger(L, 2);
    se->end_clip(static_cast<unsigned short>(d));
    return 0;
}

static luaL_Reg scene_data__index[] = {
    {"iterate", &scene_data_iterate<scene_data::ptr> },
    {"riterate", &scene_data_riterate<scene_data::ptr> },
    {"iterate_brackets", &scene_data_iterate_brackets<scene_data::ptr> },
    {"riterate_brackets", &scene_data_riterate_brackets<scene_data::ptr> },
	{"painted_shape", scene_data_painted_shape},
	{"tensor_product_patch", scene_data_tensor_product_patch},
	{"coons_patch", scene_data_coons_patch},
	{"gouraud_triangle", scene_data_gouraud_triangle},
	{"stencil_shape", scene_data_stencil_shape},
	{"begin_transform", scene_data_begin_transform},
	{"end_transform", scene_data_end_transform},
	{"begin_clip", scene_data_begin_clip},
	{"activate_clip", scene_data_activate_clip},
	{"end_clip", scene_data_end_clip},
	{"begin_fade", scene_data_begin_fade},
	{"end_fade", scene_data_end_fade},
	{"begin_blur", scene_data_begin_blur},
	{"end_blur", scene_data_end_blur},
    { nullptr, nullptr }
};

static int create(lua_State *L) {
    if (lua_gettop(L) > 1) luaL_error(L, "too many arguments");
    rvg_lua_push<scene_data::ptr>(L, rvg_lua_facade_scene_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_scene_data[] = {
    {"scene_data", create},
    {NULL, NULL}
};

int rvg_lua_scene_data_init(lua_State *L, int ctxidx) {
    rvg_lua_winding_rule_init(L, ctxidx);
    rvg_lua_shape_init(L, ctxidx);
    rvg_lua_paint_init(L, ctxidx);
    if (!rvg_lua_typeexists<scene_data::ptr>(L, ctxidx)) {
		rvg_lua_createtype<scene_data::ptr>(L, "scene_data", ctxidx);
        rvg_lua_setmethods<scene_data::ptr>(L, scene_data__index, 0, ctxidx);
    }
    if (!rvg_lua_typeexists<scene_data::const_ptr>(L, ctxidx)) {
		rvg_lua_createtype<scene_data::const_ptr>(L, "const scene_data",
			ctxidx);
		rvg_lua_setmethods<scene_data::const_ptr>(L,
			const_scene_data__index, 0, ctxidx);
    }
    return 0;
}

int rvg_lua_scene_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_scene_data_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_scene_data, 1); // mod
    return 0;
}
