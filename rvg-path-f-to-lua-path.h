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
#ifndef RVG_PATH_F_TO_LUA_PATH_H
#define RVG_PATH_F_TO_LUA_PATH_H

#include "rvg-lua.h"
#include "rvg-i-path.h"

namespace rvg {

// The is a Lua object in the stack, given by its index.
// If the Lua object is guaranteed to remain in the stack from construction to
// destruction of the filter, set REF to false.
// Otherwise, set it to true, which will cause the filter to hold a reference to
// the object until the filter is destroyed.
template <bool REF>
class path_f_to_lua_path: public i_path<path_f_to_lua_path<REF>> {

	lua_State *m_L;
	int m_sink;

public:
	path_f_to_lua_path(lua_State *L, int sink):
		m_L{L},
        m_sink{rvg_lua_abs_index(L, sink)} {
        if (REF) {
            lua_pushvalue(m_L, sink);
            m_sink = luaL_ref(m_L, LUA_REGISTRYINDEX);
        }
    }

	path_f_to_lua_path(const path_f_to_lua_path<REF> &other) = delete;
	path_f_to_lua_path &operator=(const path_f_to_lua_path<REF> &other) = delete;

    path_f_to_lua_path(path_f_to_lua_path<REF> &&other):
        m_L{other.m_L},
        m_sink{other.m_sink} {
        other.m_L = nullptr;
        other.m_sink = LUA_NOREF;
    }

    path_f_to_lua_path<REF> &operator=(path_f_to_lua_path<REF> &&other) {
        m_L = other.m_L;
        m_sink = other.m_sink;
        other.m_L = nullptr;
        other.m_sink = LUA_NOREF;
    }

    ~path_f_to_lua_path() {
        if (REF && m_L) {
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_sink);
        }
    }

private:

    void push_helper(void) { ; }

    template <typename ...REST>
    void push_helper(rvgf first, REST ...rest) {
        lua_pushnumber(m_L, first);
        push_helper(rest...);
    }

	template <typename ...REST>
	void call(const char *method, REST ...rest) {
        if (REF) {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_sink);
            lua_getfield(m_L, -1, method);
            lua_insert(m_L, -2);
        } else {
            lua_getfield(m_L, m_sink, method);
			lua_pushvalue(m_L, m_sink);
        }
		if (!lua_isnil(m_L, -2)) {
			push_helper(rest...);
			lua_call(m_L, sizeof...(REST)+1, 0);
		} else {
			lua_pop(m_L, 2);
		}
    }

friend i_input_path<path_f_to_lua_path<REF>>;

	void do_begin_contour(rvgf x0, rvgf y0) {
		call("begin_contour", x0, y0);
	}

	void do_end_open_contour(rvgf x0, rvgf y0) {
		call("end_open_contour", x0, y0);
	}

	void do_end_closed_contour(rvgf x0, rvgf y0) {
		call("end_closed_contour", x0, y0);
	}

	void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
		call("linear_segment", x0, y0, x1, y1);
	}

	void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
		rvgf x2, rvgf y2) {
		call("quadratic_segment", x0, y0, x1, y1, x2, y2);
	}

	void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
		rvgf w1, rvgf x2, rvgf y2) {
		call("rational_quadratic_segment", x0, y0, x1, y1, w1,
            x2, y2);
    }

	void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
		rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
		call("cubic_segment", x0, y0, x1, y1, x2, y2, x3, y3);
	}

friend i_regular_path<path_f_to_lua_path<REF>>;

    void do_begin_regular_contour(rvgf dx0, rvgf dy0, rvgf x0, rvgf y0) {
        call("begin_regular_contour", dx0, dy0, x0, y0);
    }

    void do_end_regular_open_contour(rvgf x0, rvgf y0, rvgf dx0, rvgf dy0) {
        call("end_regular_open_contour", x0, y0, dx0, dy0);
    }

    void do_end_regular_closed_contour(rvgf x0, rvgf y0,
        rvgf dx0, rvgf dy0) {
        call("end_regular_closed_contour", x0, y0, dx0, dy0);
    }

    void do_degenerate_segment(rvgf x0, rvgf y0, rvgf dx, rvgf dy,
        rvgf x1, rvgf y1) {
        call("degenerate_segment", x0, y0, dx, dy, x1, y1);
    }

    void do_cusp(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        call("cusp", dx0, dy0, x, y, dx1, dy1, w);
    }

    void do_inner_cusp(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        call("inner_cusp", dx0, dy0, x, y, dx1, dy1, w);
    }

    void do_begin_segment_piece(rvgf dxi, rvgf dyi, rvgf xi, rvgf yi) {
        call("begin_segment_piece", dxi, dyi, xi, yi);
    }

    void do_end_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        call("end_regular_open_contour", xi, yi, dxi, dyi);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1) {
        call("linear_segment_piece", ti, tf, x0, y0, x1, y1);
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        call("quadratic_segment_piece", ti, tf, x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        call("rational_quadratic_segment_piece", ti, tf, x0, y0, x1, y1, w1,
            x2, y2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        call("cubic_segment_piece", ti, tf, x0, y0, x1, y1, x2, y2, x3, y3);
    }

friend i_offsetting_parameters<path_f_to_lua_path<REF>>;

    void do_offset_cusp_parameter(rvgf t) {
        call("offset_cusp_parameter", t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        call("evolute_cusp_parameter", t);
    }

friend i_join_parameters<path_f_to_lua_path<REF>>;

    void do_join_tangent_parameter(rvgf t) {
        call("join_tangent_parameter", t);
    }

    void do_join_vertex_parameter(rvgf t) {
        call("join_vertex_parameter", t);
    }

friend i_cubic_parameters<path_f_to_lua_path<REF>>;

    void do_inflection_parameter(rvgf t) {
        call("inflection_parameter", t);
    }

    void do_double_point_parameter(rvgf t) {
        call("double_point_parameter", t);
    }

friend i_decorated_path<path_f_to_lua_path<REF>>;

    void do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        call("initial_cap", x, y, dx, dy);
    }

    void do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        call("terminal_cap", dx, dy, x, y);
    }

    void do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        call("initial_butt_cap", x, y, dx, dy);
    }

    void do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        call("terminal_butt_cap", dx, dy, x, y);
    }

    void do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        call("backward_initial_cap", x, y, dx, dy);
    }

    void do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        call("backward_terminal_cap", dx, dy, x, y);
    }

    void do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        call("backward_initial_butt_cap", x, y, dx, dy);
    }

    void do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        call("backward_terminal_butt_cap", dx, dy, x, y);
    }

    void do_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        call("join", dx0, dy0, x, y, dx1, dy1, w);
    }

    void do_inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        call("inner_join", dx0, dy0, x, y, dx1, dy1, w);
    }

friend i_dashing_parameters<path_f_to_lua_path<REF>>;

    void do_begin_dash_parameter(rvgf t) {
        call("begin_dash_parameter", t);
    }

    void do_end_dash_parameter(rvgf t) {
        call("end_dash_parameter", t);
    }

    void do_backward_begin_dash_parameter(rvgf t) {
        call("backward_begin_dash_parameter", t);
    }

    void do_backward_end_dash_parameter(rvgf t) {
        call("backward_end_dash_parameter", t);
    }

friend i_monotonic_parameters<path_f_to_lua_path<REF>>;

    void do_root_dx_parameter(rvgf t) {
        call("root_dx_parameter", t);
    }

    void do_root_dy_parameter(rvgf t) {
        call("root_dy_parameter", t);
    }

    void do_root_dw_parameter(rvgf t) {
        call("root_dw_parameter", t);
    }

};

static inline auto
make_path_f_to_lua_path_in_stack(lua_State *L, int sink) {
    return path_f_to_lua_path<false>{L, sink};
}

static inline auto
make_path_f_to_lua_path_ref(lua_State *L, int sink) {
    return path_f_to_lua_path<true>{L, sink};
}

}

#endif
