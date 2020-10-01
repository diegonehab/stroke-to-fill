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
#include "rvg-svg-path-parse.h"

#include "rvg-path-f-to-lua-path.h"
#include "rvg-lua-path-data.h"

using namespace rvg;

template <typename PTR>
static int path_data_iterate(lua_State *L) {
    PTR p = rvg_lua_check<PTR>(L, 1);
    p->iterate(make_path_f_to_lua_path_in_stack(L, 2));
    return 0;
}

template <typename PTR>
static int path_data_riterate(lua_State *L) {
    PTR p = rvg_lua_check<PTR>(L, 1);
    p->riterate(make_path_f_to_lua_path_in_stack(L, 2));
    return 0;
}

template <typename ...As, typename F, size_t ...Is>
int path_data_invoke_method_helper(lua_State *L, F f,
    std::index_sequence<Is...>) {
    path_data::ptr p = rvg_lua_check<path_data::ptr>(L, 1);
    ((*p).*f)(rvg_lua_check<As>(L, Is+2)...);
    return 0;
}

template <typename ...As, typename P>
int path_data_invoke_method(lua_State *L, void (P::*f)(As...)) {
    return path_data_invoke_method_helper<As...>(L, f,
        std::make_index_sequence<sizeof...(As)>{});
}

static int path_data_begin_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf>(L,
        &path_data::begin_contour);
}

static int path_data_end_open_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf>(L,
            &path_data::end_open_contour);
}

static int path_data_end_closed_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf>(L,
            &path_data::end_closed_contour);
}

static int path_data_linear_segment(lua_State *L) {
    return path_data_invoke_method<rvgf,rvgf,rvgf,rvgf>(L,
        &path_data::linear_segment);
}

static int path_data_quadratic_segment(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf,
       rvgf>(L, &path_data::quadratic_segment);
}

static int path_data_rational_quadratic_segment(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
       rvgf>(L, &path_data::rational_quadratic_segment);
}

static int path_data_cubic_segment(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
       rvgf, rvgf>(L, &path_data::cubic_segment);
}

static int path_data_cusp(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf>(L, &path_data::cusp);
}

static int path_data_inner_cusp(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf>(L, &path_data::inner_cusp);
}

static int path_data_begin_regular_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::begin_regular_contour);
}

static int path_data_end_regular_open_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::end_regular_open_contour);
}

static int path_data_end_regular_closed_contour(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::end_regular_closed_contour);
}

static int path_data_degenerate_segment(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::degenerate_segment);
}

static int path_data_begin_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::begin_segment_piece);
}

static int path_data_end_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::end_segment_piece);
}

static int path_data_linear_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf>(L, &path_data::linear_segment_piece);
}

static int path_data_quadratic_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf>(L, &path_data::quadratic_segment_piece);
}

static int path_data_rational_quadratic_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
           rvgf, rvgf>(L, &path_data::rational_quadratic_segment_piece);
}

static int path_data_cubic_segment_piece(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf, rvgf, rvgf>(L, &path_data::cubic_segment_piece);
}

static int path_data_inflection_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::inflection_parameter);
}

static int path_data_double_point_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L,
        &path_data::double_point_parameter);
}

static int path_data_root_dx_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::root_dx_parameter);
}

static int path_data_root_dy_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::root_dy_parameter);
}

static int path_data_root_dw_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::root_dw_parameter);
}

static int path_data_offset_cusp_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::offset_cusp_parameter);
}

static int path_data_evolute_cusp_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::evolute_cusp_parameter);
}

static int path_data_join_tangent_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::join_tangent_parameter);
}

static int path_data_join_vertex_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::join_vertex_parameter);
}

static int path_data_initial_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::initial_cap);
}

static int path_data_terminal_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::terminal_cap);
}

static int path_data_backward_initial_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::backward_initial_cap);
}

static int path_data_backward_terminal_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::backward_terminal_cap);
}

static int path_data_initial_butt_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::initial_butt_cap);
}

static int path_data_terminal_butt_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::terminal_butt_cap);
}

static int path_data_backward_initial_butt_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::backward_initial_butt_cap);
}

static int path_data_backward_terminal_butt_cap(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf>(
        L, &path_data::backward_terminal_butt_cap);
}

static int path_data_join(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
       rvgf>(L, &path_data::join);
}

static int path_data_inner_join(lua_State *L) {
    return path_data_invoke_method<rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
       rvgf>(L, &path_data::inner_join);
}

static int path_data_begin_dash_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::begin_dash_parameter);
}

static int path_data_end_dash_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::end_dash_parameter);
}

static int path_data_backward_begin_dash_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::backward_begin_dash_parameter);
}

static int path_data_backward_end_dash_parameter(lua_State *L) {
    return path_data_invoke_method<rvgf>(L, &path_data::backward_end_dash_parameter);
}

static int path_data_clear(lua_State *L) {
    path_data::ptr p = rvg_lua_check<path_data::ptr>(L, 1);
    p->clear();
    return 0;
}

static int path_data_shrink_to_fit(lua_State *L) {
    path_data::ptr p = rvg_lua_check<path_data::ptr>(L, 1);
    p->shrink_to_fit();
    return 0;
}

template <typename PTR>
static int path_data_size(lua_State *L) {
    PTR p = rvg_lua_check<PTR>(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(p->size()));
    return 1;
}

template <typename PTR>
static int path_data_empty(lua_State *L) {
    PTR p = rvg_lua_check<PTR>(L, 1);
    lua_pushboolean(L, p->empty());
    return 1;
}

static luaL_Reg path_data__index[] = {
    {"size", &path_data_size<path_data::ptr> },
    {"empty", &path_data_empty<path_data::ptr> },
    {"iterate", &path_data_iterate<path_data::ptr> },
    {"riterate", &path_data_riterate<path_data::ptr> },
    {"clear", path_data_clear },
    {"shrink_to_fit", path_data_shrink_to_fit },
    {"begin_contour", path_data_begin_contour},
    {"end_open_contour", path_data_end_open_contour},
    {"end_closed_contour", path_data_end_closed_contour},
    {"linear_segment", path_data_linear_segment},
    {"quadratic_segment", path_data_quadratic_segment},
    {"rational_quadratic_segment", path_data_rational_quadratic_segment},
    {"cubic_segment", path_data_cubic_segment},
    {"begin_regular_contour", path_data_begin_regular_contour},
    {"end_regular_open_contour", path_data_end_regular_open_contour},
    {"end_regular_closed_contour", path_data_end_regular_closed_contour},
    {"degenerate_segment", path_data_degenerate_segment},
    {"cusp", path_data_cusp},
    {"inner_cusp", path_data_inner_cusp},
    {"begin_segment_piece", path_data_begin_segment_piece},
    {"end_segment_piece", path_data_end_segment_piece},
    {"linear_segment_piece", path_data_linear_segment_piece},
    {"quadratic_segment_piece", path_data_quadratic_segment_piece},
    {"rational_quadratic_segment_piece",
        path_data_rational_quadratic_segment_piece},
    {"regular_cubic_segment", path_data_cubic_segment_piece},
    {"inflection_parameter", path_data_inflection_parameter},
    {"double_point_parameter", path_data_double_point_parameter},
    {"root_dx_parameter", path_data_root_dx_parameter},
    {"root_dy_parameter", path_data_root_dy_parameter},
    {"root_dw_parameter", path_data_root_dw_parameter},
    {"offset_cusp_parameter", path_data_offset_cusp_parameter},
    {"evolute_cusp_parameter", path_data_evolute_cusp_parameter},
    {"join_tangent_parameter", path_data_join_tangent_parameter},
    {"join_vertex_parameter", path_data_join_vertex_parameter},
    {"initial_cap", path_data_initial_cap},
    {"terminal_cap", path_data_terminal_cap},
    {"backward_initial_cap", path_data_backward_initial_cap},
    {"backward_terminal_cap", path_data_backward_terminal_cap},
    {"join", path_data_join},
    {"inner_join", path_data_inner_join},
    {"initial_butt_cap", path_data_initial_butt_cap},
    {"terminal_butt_cap", path_data_terminal_butt_cap},
    {"backward_initial_butt_cap", path_data_backward_initial_butt_cap},
    {"backward_terminal_butt_cap", path_data_backward_terminal_butt_cap},
    {"begin_dash_parameter", path_data_begin_dash_parameter},
    {"end_dash_parameter", path_data_end_dash_parameter},
    {"backward_begin_dash_parameter", path_data_backward_begin_dash_parameter},
    {"backward_end_dash_parameter", path_data_backward_end_dash_parameter},
    { nullptr, nullptr }
};

static luaL_Reg const_path_data__index[] = {
    {"size", &path_data_size<path_data::ptr> },
    {"empty", &path_data_size<path_data::ptr> },
    {"iterate", &path_data_iterate<path_data::const_ptr> },
    {"riterate", &path_data_riterate<path_data::const_ptr> },
    { nullptr, nullptr }
};

static path_data::ptr create_from_string(lua_State *L, int base) {
    const char *svg = lua_tostring(L, base);
    auto p = make_intrusive<path_data>();
    rvg_lua_push<path_data::ptr>(L, p);
    if (!svg_path_iterate(svg, *p))
        luaL_argerror(L, 1, "invalid SVG path data");
    return p;
}

static path_data::ptr create_from_table(lua_State *L, int base) {
    int n = rvg_lua_len(L, 1);
    auto p = make_intrusive<path_data>();
    auto get = [](lua_State *L, int tab, int item) {
        lua_rawgeti(L, tab, item);
        if (lua_isnumber(L, -1)) {
            rvgf f = static_cast<rvgf>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            return svg_path_token(svg_path_token::e_type::number, f);
        } else if (lua_isstring(L, -1)) {
            int i = *lua_tostring(L, -1);
            lua_pop(L, 1);
            return svg_path_token(svg_path_token::e_type::command, rvgi{i});
        } else {
            luaL_error(L, "entry %d is not a number or SVG path command", item);
            return svg_path_token();
        }
    };
    auto range = rvg_lua_make_table_range<svg_path_token>(L, base, 1, n+1, get);
    if (!svg_path_iterate(range.first, range.second, *p))
        luaL_argerror(L, 1, "invalid SVG path data");
    return p;
}

path_data::ptr rvg_lua_path_data_create(lua_State *L, int base) {
    int top = lua_gettop(L);
    if (top < base) {
        return make_intrusive<path_data>();
    } else if (lua_isstring(L, base)) {
        return create_from_string(L, base);
    } else if (lua_istable(L, base)) {
        return create_from_table(L, base);
    } else if (rvg_lua_is<path_data::ptr>(L, base)) {
        return make_intrusive<path_data>(*rvg_lua_to<path_data::ptr>(L, base));
    } else if (rvg_lua_is<path_data::const_ptr>(L, base)) {
        return make_intrusive<path_data>(*rvg_lua_to<
            path_data::const_ptr>(L, base));
    } else {
        luaL_argerror(L, 1, "expected table, string, or path data");
    }
    return make_intrusive<path_data>();
}

static rvg_lua_named_value<const char *> named_svg_commands[] = {
	{"T", "T"}, {"t", "t"}, {"R", "R"}, {"r", "r"},
	{"A", "A"}, {"a", "a"}, {"C", "C"}, {"c", "c"},
	{"H", "H"}, {"h", "h"}, {"L", "L"}, {"l", "l"},
	{"M", "M"}, {"m", "m"}, {"Q", "Q"}, {"q", "q"},
	{"S", "S"}, {"s", "s"}, {"V", "V"}, {"v", "v"},
	{"Z", "Z"}, {nullptr, nullptr}
};

static void merge_svg_commands(lua_State *L, int tabidx) {
    tabidx = rvg_lua_abs_index(L, tabidx);
    for (auto p = named_svg_commands; p->name; p++) {
        lua_pushstring(L, p->value);
        lua_setfield(L, tabidx, p->name);
    }
}

static int create(lua_State *L) {
    rvg_lua_push<path_data::ptr>(L, rvg_lua_path_data_create(L, 1));
    return 1;
}

static const luaL_Reg mod_path_data[] = {
    {"path_data", create},
    {NULL, NULL}
};

int rvg_lua_path_data_init(lua_State *L, int ctxidx) {
    if (!rvg_lua_typeexists<path_data::ptr>(L, ctxidx)) {
        rvg_lua_createtype<path_data::ptr>(L, "path_data", ctxidx);
        rvg_lua_setmethods<path_data::ptr>(L, path_data__index, 0, ctxidx);
    }
    if (!rvg_lua_typeexists<path_data::const_ptr>(L, ctxidx)) {
        rvg_lua_createtype<path_data::const_ptr>(L, "const path_data", ctxidx);
        rvg_lua_setmethods<path_data::const_ptr>(L, const_path_data__index,
            0, ctxidx);
    }
    return 0;
}

int rvg_lua_path_data_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_path_data_init(L, ctxidx); // mod
    merge_svg_commands(L, -1); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, mod_path_data, 1); // mod
    return 0;
}
