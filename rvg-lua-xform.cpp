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
#include <cassert>
#include <ostream>

#include "rvg-lua.h"
#include "rvg-lua-xform.h"
#include "rvg-lua-bbox.h"

using namespace rvg;


template <typename T>
static T* raw_check_pointer(lua_State *L, int idx,
    int ctxidx = lua_upvalueindex(1)) {
    if (!rvg_lua_is<T>(L, idx, ctxidx)) rvg_lua_argerror<T>(L, idx, ctxidx);
    return rvg_lua_topointer<T>(L, idx);
}

template <typename T>
static T raw_check(lua_State *L, int idx, int ctxidx = lua_upvalueindex(1)) {
    if (!rvg_lua_is<T>(L, idx, ctxidx))
        rvg_lua_argerror<T>(L, idx, ctxidx);
    return rvg_lua_to<T>(L, idx);
}

enum class e_type {
    identity,
    translation,
    rotation,
    scaling,
    linearity,
    affinity,
    projectivity,
    none,
};

static int unsafe_gettype(lua_State *L) {
    lua_getfield(L, 1, "type");
    return 1;
}

// Try to get a field "type" from the object at idx
static e_type safe_gettype(lua_State *L, int idx) {
    idx = rvg_lua_abs_index(L, idx);
    lua_pushcfunction(L, unsafe_gettype);
    lua_pushvalue(L, idx);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        lua_pop(L, 1);
        return e_type::none;
    }
    if (!lua_isinteger(L, -1)) {
        return e_type::none;
    }
    int t = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (t < 0 || t >= static_cast<int>(e_type::none)) {
        return e_type::none;
    }
    return static_cast<e_type>(t);
}

// Convert the transformation at idx into a xform (projectivity)
template <> xform
rvg_lua_check<xform>(lua_State *L, int idx, int ctxidx) {
    switch (safe_gettype(L, idx)) {
        case e_type::identity:
            return static_cast<xform>(raw_check<identity>(L, idx, ctxidx));
        case e_type::translation:
            return static_cast<xform>(raw_check<translation>(L, idx, ctxidx));
        case e_type::scaling:
            return static_cast<xform>(raw_check<scaling>(L, idx, ctxidx));
        case e_type::rotation:
            return static_cast<xform>(raw_check<rotation>(L, idx, ctxidx));
        case e_type::linearity:
            return static_cast<xform>(raw_check<linearity>(L, idx, ctxidx));
        case e_type::affinity:
            return static_cast<xform>(raw_check<affinity>(L, idx, ctxidx));
        case e_type::projectivity:
        default:
            return static_cast<xform>(raw_check<projectivity>(L, idx, ctxidx));
    }
}

static int unsafe_toxform(lua_State *L) {
    lua_getfield(L, 1, "toxform");
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    return 1;
}

// Try to call the method toxform from the transformation at idx
static void safe_toxform(lua_State *L, int idx) {
    idx = rvg_lua_abs_index(L, idx);
    lua_pushcfunction(L, unsafe_toxform);
    lua_pushvalue(L, idx);
    lua_pcall(L, 1, 1, 0);
}

// Get a pointer to the xform (projectivity) at idx
// To do this, the function tries to call its toxform method.
// Since we can't increase the number of elements on the stack,
// and we can't have the object be collected (leaving a dangling pointer)
// we replace the original object with the xform
template <>
xform* rvg_lua_check_pointer<xform>(lua_State *L, int idx, int ctxidx) {
    safe_toxform(L, idx);
    if (!rvg_lua_is<xform>(L, -1, ctxidx)) {
        lua_pop(L, 1);
        rvg_lua_argerror<xform>(L, idx, ctxidx);
    }
    lua_replace(L, idx);
    return rvg_lua_topointer<xform>(L, idx);
}

template <>
int rvg_lua_tostring<projectivity>(lua_State *L) {
    const auto &xf = *reinterpret_cast<projectivity *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "projectivity{%f,%f,%f,%f,%f,%f,%f,%f,%f}",
        xf[0][0], xf[0][1], xf[0][2],
        xf[1][0], xf[1][1], xf[1][2],
        xf[2][0], xf[2][1], xf[2][2]
    );
    return 1;
}

template <>
int rvg_lua_tostring<affinity>(lua_State *L) {
    const auto &xf = *reinterpret_cast<affinity *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "affinity{%f,%f,%f,%f,%f,%f}",
        xf[0][0], xf[0][1], xf[0][2],
        xf[1][0], xf[1][1], xf[1][2]
    );
    return 1;
}

template <>
int rvg_lua_tostring<linearity>(lua_State *L) {
    const auto &xf = *reinterpret_cast<linearity *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "linearity{%f,%f,%f,%f}",
        xf[0][0], xf[0][1],
        xf[1][0], xf[1][1]
    );
    return 1;
}

template <>
int rvg_lua_tostring<translation>(lua_State *L) {
    const auto &xf = *reinterpret_cast<translation *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "translation{%f,%f}", xf.get_tx(), xf.get_ty());
    return 1;
}

template <>
int rvg_lua_tostring<rotation>(lua_State *L) {
    const auto &xf = *reinterpret_cast<rotation *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "rotation{%f,%f}", xf.get_cos(), xf.get_sin());
    return 1;
}

template <>
int rvg_lua_tostring<scaling>(lua_State *L) {
    const auto &xf = *reinterpret_cast<scaling *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "scaling{%f,%f}", xf.get_sx(), xf.get_sy());
    return 1;
}

template <>
int rvg_lua_tostring<identity>(lua_State *L) {
    lua_pushstring(L, "identity");
    return 1;
}

xform rvg_lua_xform_identity(lua_State *L, int base) {
	if (lua_gettop(L) > base) luaL_error(L, "invalid number of arguments");
	return make_identity();
}

xform rvg_lua_xform_rotation(lua_State *L, int base) {
    base = base-1;
	switch (lua_gettop(L)-base) {
		case 1:
			return make_rotation(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)));
		case 3:
			return make_rotation(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)),
                static_cast<rvgf>(luaL_checknumber(L, 2+base)),
                static_cast<rvgf>(luaL_checknumber(L, 3+base)));
		default:
			luaL_error(L, "invalid number of arguments");
            return make_identity();
	}
}

xform rvg_lua_xform_scaling(lua_State *L, int base) {
    base = base-1;
	switch (lua_gettop(L)-base) {
		case 1:
			return make_scaling(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)));
		case 2:
			return make_scaling(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)),
                static_cast<rvgf>(luaL_checknumber(L, 2+base)));
		case 3:
			return make_scaling(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)),
                static_cast<rvgf>(luaL_checknumber(L, 2+base)),
                static_cast<rvgf>(luaL_checknumber(L, 3+base)));
		case 4:
			return make_scaling(
                static_cast<rvgf>(luaL_checknumber(L, 1+base)),
                static_cast<rvgf>(luaL_checknumber(L, 2+base)),
                static_cast<rvgf>(luaL_checknumber(L, 3+base)),
                static_cast<rvgf>(luaL_checknumber(L, 4+base)));
		default:
			luaL_error(L, "invalid number of arguments");
            return make_identity();
	}
}

xform rvg_lua_xform_translation(lua_State *L, int base) {
    base = base-1;
    if (lua_gettop(L)-base > 2) luaL_error(L, "invalid number of arguments");
    return make_translation(static_cast<rvgf>(luaL_checknumber(L, 1+base)),
        static_cast<rvgf>(luaL_optnumber(L, 2+base, 0.f)));
}

xform rvg_lua_xform_linearity(lua_State *L, int base) {
    base = base-1;
    if (lua_gettop(L)-base != 4)
		luaL_error(L, "invalid number of arguments");
    return make_linearity(
        static_cast<rvgf>(luaL_checknumber(L, 1+base)),
        static_cast<rvgf>(luaL_checknumber(L, 2+base)),
        static_cast<rvgf>(luaL_checknumber(L, 3+base)),
        static_cast<rvgf>(luaL_checknumber(L, 4+base)));
}

xform rvg_lua_xform_affinity(lua_State *L, int base) {
    base = base-1;
    if (lua_gettop(L)-base != 6)
		luaL_error(L, "invalid number of arguments");
    return make_affinity(
        static_cast<rvgf>(luaL_checknumber(L, 1+base)),
        static_cast<rvgf>(luaL_checknumber(L, 2+base)),
        static_cast<rvgf>(luaL_checknumber(L, 3+base)),
        static_cast<rvgf>(luaL_checknumber(L, 4+base)),
        static_cast<rvgf>(luaL_checknumber(L, 5+base)),
        static_cast<rvgf>(luaL_checknumber(L, 6+base)));
}

xform rvg_lua_xform_projectivity(lua_State *L, int base) {
    base = base-1;
    if (lua_gettop(L)-base != 9)
		luaL_error(L, "invalid number of arguments");
    return make_projectivity(
        static_cast<rvgf>(luaL_checknumber(L, 1+base)),
        static_cast<rvgf>(luaL_checknumber(L, 2+base)),
        static_cast<rvgf>(luaL_checknumber(L, 3+base)),
        static_cast<rvgf>(luaL_checknumber(L, 4+base)),
        static_cast<rvgf>(luaL_checknumber(L, 5+base)),
        static_cast<rvgf>(luaL_checknumber(L, 6+base)),
        static_cast<rvgf>(luaL_checknumber(L, 7+base)),
        static_cast<rvgf>(luaL_checknumber(L, 8+base)),
        static_cast<rvgf>(luaL_checknumber(L, 9+base)));
}

xform rvg_lua_xform_windowviewport(lua_State *L, int base) {
    base = base-1;
    if (lua_gettop(L)-base < 2 || lua_gettop(L)-base > 5)
		luaL_error(L, "invalid number of arguments");
    return make_windowviewport(
        rvg_lua_check<window>(L, 1+base),
        rvg_lua_check<viewport>(L, 2+base),
        rvg_lua_opt<e_align>(L, 3+base, e_align::mid),
        rvg_lua_opt<e_align>(L, 4+base, e_align::mid),
        rvg_lua_opt<e_aspect>(L, 5+base, e_aspect::none));
}

template <typename XF>
static int apply(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    auto x = static_cast<rvgf>(luaL_checknumber(L, 2));
    auto y = static_cast<rvgf>(luaL_checknumber(L, 3));
    auto w = static_cast<rvgf>(luaL_optnumber(L, 4, 1.f));
    std::tie(x, y, w) = xf->apply(x, y, w);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, w);
    return 3;
}

template <typename XF>
static int inverse(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, xf->inverse());
    return 1;
}

template <typename XF>
static int transpose(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, xf->transpose());
    return 1;
}

template <typename XF>
static int adjugate(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, xf->adjugate());
    return 1;
}

template <typename XF>
static int det(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    lua_pushnumber(L, xf->det());
    return 1;
}

template <typename XF>
static int translated(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, xf->translated(
        static_cast<rvgf>(luaL_checknumber(L, 2)),
        static_cast<rvgf>(luaL_checknumber(L, 3))));
    return 1;
}

template <typename XF>
static int transformed(lua_State *L) {
    const XF &xf = *raw_check_pointer<XF>(L, 1);
    switch (safe_gettype(L, 2)) {
        case e_type::identity:
            lua_pop(L, 1);
            return 1;
        case e_type::translation:
            return rvg_lua_push(L, raw_check<translation>(L, 2) * xf);
        case e_type::scaling:
            return rvg_lua_push(L, raw_check<scaling>(L, 2) * xf);
        case e_type::rotation:
            return rvg_lua_push(L, raw_check<rotation>(L, 2) * xf);
        case e_type::linearity:
            return rvg_lua_push(L, raw_check<linearity>(L, 2) * xf);
        case e_type::affinity:
            return rvg_lua_push(L, raw_check<affinity>(L, 2) * xf);
        case e_type::projectivity:
            return rvg_lua_push(L, raw_check<projectivity>(L, 2) * xf);
        default:
            luaL_argerror(L, 2, "expected xform");
            return 0;
    }
}

template <typename XF>
static int rotated(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    switch (lua_gettop(L)) {
        case 2:
            return rvg_lua_push(L, xf->rotated(
                static_cast<rvgf>(luaL_checknumber(L, 2))));
        case 3:
            return rvg_lua_push(L, xf->rotated(
                static_cast<rvgf>(luaL_checknumber(L, 2)),
                static_cast<rvgf>(luaL_checknumber(L, 3))));
        case 4:
            return rvg_lua_push(L, xf->rotated(
                static_cast<rvgf>(luaL_checknumber(L, 2)),
                static_cast<rvgf>(luaL_checknumber(L, 3)),
                static_cast<rvgf>(luaL_checknumber(L, 4))));
        default:
            luaL_error(L, "invalid number of arguments");
            return 0;
    }
}

template <typename XF>
static int linear(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    return rvg_lua_push(L, xf->linear(
        static_cast<rvgf>(luaL_checknumber(L, 2)),
        static_cast<rvgf>(luaL_checknumber(L, 3)),
        static_cast<rvgf>(luaL_checknumber(L, 4)),
        static_cast<rvgf>(luaL_checknumber(L, 5))));
}

template <typename XF>
static int affine(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    return rvg_lua_push(L, xf->affine(
        static_cast<rvgf>(luaL_checknumber(L, 2)),
        static_cast<rvgf>(luaL_checknumber(L, 3)),
        static_cast<rvgf>(luaL_checknumber(L, 4)),
        static_cast<rvgf>(luaL_checknumber(L, 5)),
        static_cast<rvgf>(luaL_checknumber(L, 6)),
        static_cast<rvgf>(luaL_checknumber(L, 7))));
}

template <typename XF>
static int projected(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    return rvg_lua_push(L, xf->projected(
        static_cast<rvgf>(luaL_checknumber(L, 2)),
        static_cast<rvgf>(luaL_checknumber(L, 3)),
        static_cast<rvgf>(luaL_checknumber(L, 4)),
        static_cast<rvgf>(luaL_checknumber(L, 5)),
        static_cast<rvgf>(luaL_checknumber(L, 6)),
        static_cast<rvgf>(luaL_checknumber(L, 7)),
        static_cast<rvgf>(luaL_checknumber(L, 8)),
        static_cast<rvgf>(luaL_checknumber(L, 9)),
        static_cast<rvgf>(luaL_checknumber(L, 10))));
}

template <typename XF>
static int scaled(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    switch (lua_gettop(L)) {
        case 2:
            return rvg_lua_push(L, xf->scaled(
                static_cast<rvgf>(luaL_checknumber(L, 2))));
        case 3:
            return rvg_lua_push(L, xf->scaled(
                static_cast<rvgf>(luaL_checknumber(L, 2)),
                static_cast<rvgf>(luaL_checknumber(L, 3))));
        case 5:
            return rvg_lua_push(L, xf->scaled(
                static_cast<rvgf>(luaL_checknumber(L, 2)),
                static_cast<rvgf>(luaL_checknumber(L, 3)),
                static_cast<rvgf>(luaL_checknumber(L, 4)),
                static_cast<rvgf>(luaL_checknumber(L, 5))));
        default:
            luaL_error(L, "invalid number of arguments");
            return 0;
    }
}

template <typename XF>
static int windowviewport(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, xf->windowviewport(
            rvg_lua_check<window>(L, 2),
            rvg_lua_check<viewport>(L, 3),
            rvg_lua_opt<e_align>(L, 4, e_align::mid),
            rvg_lua_opt<e_align>(L, 5, e_align::mid),
            rvg_lua_opt<e_aspect>(L, 6, e_aspect::none)));
    return 1;
}

template <typename XF>
static int toxform(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, static_cast<xform>(*xf));
    return 1;
}

template <>
int toxform<xform>(lua_State *) {
    return 1;
}

template <typename XF>
static int toprojectivity(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, static_cast<projectivity>(*xf));
    return 1;
}

template <>
int toprojectivity<projectivity>(lua_State *) {
    return 1;
}

template <typename XF>
static int toaffinity(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, static_cast<affinity>(*xf));
    return 1;
}

template <>
int toaffinity<projectivity>(lua_State *L) {
    const projectivity &p = *raw_check_pointer<projectivity>(L, 1);
    rvg_lua_push(L, affinity{p[0][0], p[0][1], p[0][2], p[1][0], p[1][1], p[1][2]});
    return 1;
}

template <typename XF>
static int tolinear(lua_State *L) {
    const XF *xf = raw_check_pointer<XF>(L, 1);
    rvg_lua_push(L, static_cast<linearity>(*xf));
    return 1;
}

template <>
int tolinear<translation>(lua_State *L) {
    rvg_lua_push(L, linearity{0, 0, 0, 0});
    return 1;
}

template <>
int tolinear<affinity>(lua_State *L) {
    const affinity &a = *raw_check_pointer<affinity>(L, 1);
    rvg_lua_push(L, linearity{a[0][0], a[0][1], a[1][0], a[1][1]});
    return 1;
}

template <>
int tolinear<projectivity>(lua_State *L) {
    const projectivity &p = *raw_check_pointer<projectivity>(L, 1);
    rvg_lua_push(L, linearity{p[0][0], p[0][1], p[1][0], p[1][1]});
    return 1;
}

// xform is special in that it has both methods and
// other data. it receives a table with the methdos as an
// additional upvalue
template <typename XF, int M, int N>
static int index_or_field(lua_State *L) {
    if (lua_isnumber(L, 2)) {
        auto i = static_cast<int>(lua_tointeger(L, 2) - 1);
        auto j = i % N;
        i /= N;
        if (i >= 0 && i < M && j >= 0 && j < N) {
            lua_pushnumber(L,
                raw_check<XF>(L, 1, lua_upvalueindex(2))[i][j]);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_gettable(L, lua_upvalueindex(1));
    }
    return 1;
}

template <typename XF>
static int index_func(lua_State *L) {
    lua_gettable(L, lua_upvalueindex(1));
    return 1;
}

template <>
int index_func<projectivity>(lua_State *L) {
    return index_or_field<projectivity, 3, 3>(L);
}

template <>
int index_func<affinity>(lua_State *L) {
    return index_or_field<affinity, 2, 3>(L);
}

template <>
int index_func<linearity>(lua_State *L) {
    return index_or_field<linearity, 2, 2>(L);
}

template <typename XF>
static int len(lua_State *L) {
	luaL_error(L, "not linearity, affinity, or projectivity");
    return 1;
}

template <>
int len<linearity>(lua_State *L) {
    lua_pushinteger(L, 4);
    return 1;
}

template <>
int len<affinity>(lua_State *L) {
    lua_pushinteger(L, 6);
    return 1;
}

template <>
int len<projectivity>(lua_State *L) {
    lua_pushinteger(L, 9);
    return 1;
}

template <typename XF>
static int mul(lua_State *L) {
    constexpr int ctxidx = lua_upvalueindex(2);
    const XF &xf = *raw_check_pointer<XF>(L, 1, ctxidx);
    switch (safe_gettype(L, 2)) {
        case e_type::identity:
            lua_pop(L, 2);
            return 1;
        case e_type::translation:
            return rvg_lua_push(L, xf *
                raw_check<translation>(L, 2, ctxidx), ctxidx);
        case e_type::scaling:
            return rvg_lua_push(L, xf *
                raw_check<scaling>(L, 2, ctxidx), ctxidx);
        case e_type::rotation:
            return rvg_lua_push(L, xf *
                raw_check<rotation>(L, 2, ctxidx), ctxidx);
        case e_type::linearity:
            return rvg_lua_push(L, xf *
                raw_check<linearity>(L, 2, ctxidx), ctxidx);
        case e_type::affinity:
            return rvg_lua_push(L, xf *
                raw_check<affinity>(L, 2, ctxidx), ctxidx);
        case e_type::projectivity:
            return rvg_lua_push(L, xf *
                raw_check<projectivity>(L, 2, ctxidx), ctxidx);
        default:
            luaL_argerror(L, 2, "expected xform");
            return 0;
    }
}

template <typename XF>
static int add(lua_State *L) {
    constexpr int ctxidx = lua_upvalueindex(2);
    rvg_lua_push(L,
        raw_check<XF>(L, 1, ctxidx) +
        raw_check<XF>(L, 2, ctxidx), ctxidx);
    return 1;
}

template <typename XF>
static int sub(lua_State *L) {
    constexpr int ctxidx = lua_upvalueindex(2);
    rvg_lua_push(L,
        raw_check<XF>(L, 1, ctxidx) -
        raw_check<XF>(L, 2, ctxidx), ctxidx);
    return 1;
}

template <typename XF>
static int eq(lua_State *L) {
    constexpr int ctxidx = lua_upvalueindex(2);
    lua_pushboolean(L,
        raw_check<XF>(L, 1, ctxidx) ==
        raw_check<XF>(L, 2, ctxidx));
    return 1;
}

static int create_identity(lua_State *L) {
	if (lua_gettop(L) > 0) luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, identity{});
}

static int create_rotation(lua_State *L) {
	switch (lua_gettop(L)) {
		case 1:
			return rvg_lua_push(L, rotation{
                static_cast<rvgf>(luaL_checknumber(L, 1))});
		case 2:
			return rvg_lua_push(L, rotation{
                static_cast<rvgf>(luaL_checknumber(L, 1)),
                static_cast<rvgf>(luaL_checknumber(L, 2))});
		case 3: {
            rvgf deg = static_cast<rvgf>(luaL_checknumber(L, 1));
            rvgf cx = static_cast<rvgf>(luaL_checknumber(L, 2));
            rvgf cy = static_cast<rvgf>(luaL_checknumber(L, 3));
            return rvg_lua_push(L, translation{-cx, -cy}.rotated(deg).
                translated(cx, cy));
        }
		default:
			luaL_error(L, "invalid number of arguments");
            return 0;
	}
}

static int create_scaling(lua_State *L) {
	switch (lua_gettop(L)) {
		case 1:
			return rvg_lua_push(L, scaling{
                static_cast<rvgf>(luaL_checknumber(L, 1))});
		case 2:
			return rvg_lua_push(L, scaling{
                static_cast<rvgf>(luaL_checknumber(L, 1)),
                static_cast<rvgf>(luaL_checknumber(L, 2))});
		case 3: {
            rvgf s = static_cast<rvgf>(luaL_checknumber(L, 1));
            rvgf cx = static_cast<rvgf>(luaL_checknumber(L, 2));
            rvgf cy = static_cast<rvgf>(luaL_checknumber(L, 3));
            return rvg_lua_push(L, translation{-cx, -cy}.scaled(s).
                translated(cx, cy));
        }
		case 4: {
            rvgf sx = static_cast<rvgf>(luaL_checknumber(L, 1));
            rvgf sy = static_cast<rvgf>(luaL_checknumber(L, 2));
            rvgf cx = static_cast<rvgf>(luaL_checknumber(L, 3));
            rvgf cy = static_cast<rvgf>(luaL_checknumber(L, 4));
            return rvg_lua_push(L, translation{-cx, -cy}.scaled(sx, sy).
                translated(cx, cy));
        }
		default:
			luaL_error(L, "invalid number of arguments");
            return 0;
	}
}

static int create_translation(lua_State *L) {
	if (lua_gettop(L) > 2) luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, translation{
        static_cast<rvgf>(luaL_checknumber(L, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 2, 0.f))
    });
}

static int create_windowviewport(lua_State *L) {
    if (lua_gettop(L) < 2 || lua_gettop(L) > 5)
		luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, make_windowviewport(
        rvg_lua_check<window>(L, 1),
        rvg_lua_check<viewport>(L, 2),
        rvg_lua_opt<e_align>(L, 3, e_align::mid),
        rvg_lua_opt<e_align>(L, 4, e_align::mid),
        rvg_lua_opt<e_aspect>(L, 5, e_aspect::none)));
}

static int create_linear(lua_State *L) {
	if (lua_gettop(L) > 4) luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, linearity{
        static_cast<rvgf>(luaL_optnumber(L, 1, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 2, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 3, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 4, 1))
    });
}

static int create_affinity(lua_State *L) {
	if (lua_gettop(L) > 6) luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, affinity{
        static_cast<rvgf>(luaL_optnumber(L, 1, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 2, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 3, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 4, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 5, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 6, 0))
    });
}

static int create_projectivity(lua_State *L) {
	if (lua_gettop(L) > 9) luaL_error(L, "invalid number of arguments");
    return rvg_lua_push(L, projectivity{
        static_cast<rvgf>(luaL_optnumber(L, 1, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 2, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 3, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 4, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 5, 1)),
        static_cast<rvgf>(luaL_optnumber(L, 6, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 7, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 8, 0)),
        static_cast<rvgf>(luaL_optnumber(L, 9, 1))
    });
}

static const luaL_Reg mod_xform[] = {
    {"identity", create_identity},
    {"rotation", create_rotation},
    {"translation", create_translation},
    {"scaling", create_scaling},
    {"linearity", create_linear},
    {"affinity", create_affinity},
    {"projectivity", create_projectivity},
    {"windowviewport", create_windowviewport},
    {NULL, NULL}
};

static rvg_lua_named_value<e_aspect> named_aspect[] = {
    {"none", e_aspect::none},
    {"extend", e_aspect::extend},
    {"trim", e_aspect::trim},
    {nullptr, e_aspect::none},
};

static rvg_lua_named_value<e_align> named_align[] = {
    {"min", e_align::min},
    {"mid", e_align::mid},
    {"max", e_align::max},
    {nullptr, e_align::min},
};

template <typename XF>
int init(lua_State *L, const char *xf_name, e_type xf_type, int ctxidx) {
    static luaL_Reg index_table[] = {
        {"rotated", rotated<XF> },
        {"translated", translated<XF> },
        {"scaled", scaled<XF> },
        {"linear", linear<XF> },
        {"affine", affine<XF> },
        {"projected", projected<XF> },
        {"windowviewport", windowviewport<XF> },
        {"transformed", transformed<XF> },
        {"apply", apply<XF> },
        {"toxform", toxform<XF> },
        {"tolinear", tolinear<XF> },
        {"toaffinity", toaffinity<XF> },
        {"toprojectivity", toprojectivity<XF> },
        {"inverse", inverse<XF> },
        {"adjugate", adjugate<XF> },
        {"transpose", transpose<XF> },
        {"det", det<XF> },
        { nullptr, nullptr }
    };
    static luaL_Reg meta[] = {
        {"__index", index_func<XF> },
        {"__len", len<XF> },
        {"__mul", mul<XF> },
        {"__add", add<XF> },
        {"__sub", sub<XF> },
        {"__eq", eq<XF> },
        { nullptr, nullptr }
    };
    if (!rvg_lua_typeexists<XF>(L, ctxidx)) {
		rvg_lua_createtype<XF>(L, xf_name, ctxidx);
		rvg_lua_setmethods<XF>(L, index_table, 0, ctxidx);
		// xform is special in that it has both methods and
		// other data. So we take the xform__index_table that
		// setmethods created for us and pass it as an
		// additional upvalue to the index_func that
		// replaces it
		rvg_lua_gettypemetatable<XF>(L, ctxidx);
		lua_getfield(L, -1, "__index");
        lua_pushinteger(L, static_cast<int>(xf_type));
        lua_setfield(L, -2, "type");
		rvg_lua_setmetamethods<XF>(L, meta, 1, ctxidx);
		lua_pop(L, 1);
    }
    return 0;
}

int rvg_lua_xform_init(lua_State *L, int ctxidx) {
	rvg_lua_bbox_init(L, ctxidx);
    if (!rvg_lua_typeexists<e_align>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_align>(L, "align", named_align, ctxidx);
    }
    if (!rvg_lua_typeexists<e_aspect>(L, ctxidx)) {
        rvg_lua_enum_createtype<e_aspect>(L, "aspect", named_aspect, ctxidx);
    }
    init<identity>(L, "identity", e_type::identity, ctxidx);
    init<translation>(L, "translation", e_type::translation, ctxidx);
    init<rotation>(L, "rotation", e_type::rotation, ctxidx);
    init<scaling>(L, "scaling", e_type::scaling, ctxidx);
    init<linearity>(L, "linearity", e_type::linearity, ctxidx);
    init<affinity>(L, "affinity", e_type::affinity, ctxidx);
    init<projectivity>(L, "projectivity", e_type::projectivity, ctxidx);
    return 0;
}

int rvg_lua_xform_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_xform_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_xform, 1); // mod
    return 0;
}
