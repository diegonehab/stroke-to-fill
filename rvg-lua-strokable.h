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
#ifndef RVG_LUA_STROKABLE_H
#define RVG_LUA_STROKABLE_H

#include "rvg-lua.h"
#include "rvg-lua-stroke-style.h"

namespace rvg {

namespace detail {

    template <typename T, typename D, typename P>
    static int strokable_capped(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).capped(
            rvg_lua_check<e_stroke_cap>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_initial_capped(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).initial_capped(
            rvg_lua_check<e_stroke_cap>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_terminal_capped(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).terminal_capped(
            rvg_lua_check<e_stroke_cap>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_dash_initial_capped(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).dash_initial_capped(
            rvg_lua_check<e_stroke_cap>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_dash_terminal_capped(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).dash_terminal_capped(
            rvg_lua_check<e_stroke_cap>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_joined(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).joined(
            rvg_lua_check<e_stroke_join>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_inner_joined(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).inner_joined(
            rvg_lua_check<e_stroke_join>(L, 2)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_miter_limited(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).miter_limited(
            rvg_lua_optfloat(L, 2, stroke_style::default_miter_limit)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_reset_on_move(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).reset_on_move(
            rvg_lua_optboolean(L, 2, stroke_style::default_resets_on_move)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_dash_offset(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).dash_offset(
            rvg_lua_optfloat(L, 2, stroke_style::default_dash_offset)));
        return 1;
    }

    template <typename T, typename D, typename P>
    static int strokable_dashed(lua_State *L) {
        D deref; P push;
        push(L, deref(rvg_lua_check<T>(L, 1)).dashed(
            rvg_lua_checkfloatvector(L, 2)));
        return 1;
    }
}

template <typename T, typename D, typename P>
static void rvg_lua_set_strokable(lua_State *L, int ctxidx) {
    ctxidx = rvg_lua_abs_index(L, ctxidx);
    static luaL_Reg methods[] = {
        { "reset_on_move", &detail::strokable_reset_on_move<T, D, P> },
        { "dash_offset", &detail::strokable_dash_offset<T, D, P> },
        { "miter_limited", &detail::strokable_miter_limited<T, D, P> },
        { "capped", &detail::strokable_capped<T, D, P> },
        { "initial_capped", &detail::strokable_initial_capped<T, D, P> },
        { "terminal_capped", &detail::strokable_terminal_capped<T, D, P> },
        { "dash_initial_capped", &detail::strokable_dash_initial_capped<T, D, P> },
        { "dash_terminal_capped", &detail::strokable_dash_terminal_capped<T, D, P> },
        { "joined", &detail::strokable_joined<T, D, P> },
        { "inner_joined", &detail::strokable_inner_joined<T, D, P> },
        { "dashed", &detail::strokable_dashed<T, D, P> },
        { nullptr, nullptr }
    };
    rvg_lua_setmethods<T>(L, methods, 0, ctxidx);
}

}

#endif
