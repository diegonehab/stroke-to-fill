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
#ifndef RVG_LUA_XFORMABLE_H
#define RVG_LUA_XFORMABLE_H

#include "rvg-lua.h"
#include "rvg-lua-xform.h"

namespace rvg {

namespace detail {

    template <typename T>
    static int xformable_translated(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_translation(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_rotated(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_rotation(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_scaled(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_scaling(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_linear(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_linearity(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_affine(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_affinity(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_projected(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_projectivity(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_windowviewport(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_xform_windowviewport(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_transformed(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).transformed(
                rvg_lua_check<xform>(L, 2)));
        return 1;
    }

    template <typename T>
    static int xformable_without_xf(lua_State *L) {
        rvg_lua_push<T>(L, rvg_lua_check<T>(L, 1).without_xf());
        return 1;
    }

    template <typename T>
    static int xformable_get_xf(lua_State *L) {
        rvg_lua_push<xform>(L, rvg_lua_check<T>(L, 1).get_xf());
        return 1;
    }
}

template <typename T>
static void rvg_lua_set_xformable(lua_State *L, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    static luaL_Reg methods[] = {
        { "translated", &detail::xformable_translated<T> },
        { "rotated", &detail::xformable_rotated<T> },
        { "scaled", &detail::xformable_scaled<T> },
        { "linear", &detail::xformable_linear<T> },
        { "affine", &detail::xformable_affine<T> },
        { "projected", &detail::xformable_projected<T> },
        { "windowviewport", &detail::xformable_windowviewport<T> },
        { "transformed", &detail::xformable_transformed<T> },
        { "without_xf", &detail::xformable_without_xf<T> },
        { "get_xf", &detail::xformable_get_xf<T> },
        { nullptr, nullptr }
    };
    rvg_lua_setmethods<T>(L, methods, 0, ctxidx);
}

}

#endif
