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
#include "rvg-lua-path-filters.h"

#include "rvg-path-f-to-lua-path.h"
#include "rvg-cubic-parameters-f-forward-if.h"
#include "rvg-dashing-parameters-f-forward-if.h"
#include "rvg-decorated-path-f-forward-if.h"
#include "rvg-input-path-f-forward-if.h"
#include "rvg-join-parameters-f-forward-if.h"
#include "rvg-monotonic-parameters-f-forward-if.h"
#include "rvg-offsetting-parameters-f-forward-if.h"
#include "rvg-regular-path-f-forward-if.h"

#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-downgrade-degenerates.h"
#include "rvg-input-path-f-find-cubic-parameters.h"
#include "rvg-input-path-f-find-monotonic-parameters.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"
#include "rvg-path-f-spy.h"

namespace rvg {

template <typename SINK>
class path_f_forward_if:
    public i_path<path_f_forward_if<SINK>> {

   lua_State *m_L;
   SINK m_sink;

public:

   path_f_forward_if(lua_State *L, SINK &&sink):
       m_L{L},
       m_sink{std::forward<SINK>(sink)} {
       ;
   }

private:

friend i_input_path<path_f_forward_if<SINK>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        return make_input_path_f_forward_if(m_sink).
            begin_contour(x0, y0);
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        return make_input_path_f_forward_if(m_sink).
            end_open_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        return make_input_path_f_forward_if(m_sink).
            end_closed_contour(x0, y0);
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        return make_input_path_f_forward_if(m_sink).
            linear_segment(x0, y0, x1, y1);
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        return make_input_path_f_forward_if(m_sink).
            quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        return make_input_path_f_forward_if(m_sink).
            rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return make_input_path_f_forward_if(m_sink).
            cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

friend i_regular_path<path_f_forward_if<SINK>>;

    void do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return make_regular_path_f_forward_if(m_sink).
            begin_regular_contour(xi, yi, dxi, dyi);
    }

    void do_end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return make_regular_path_f_forward_if(m_sink).
            end_regular_open_contour(dxf, dyf, xf, yf);
    }

    void do_end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return make_regular_path_f_forward_if(m_sink).
            end_regular_closed_contour(dxf, dyf, xf, yf);
    }

    void do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
            rvgf xf, rvgf yf) {
        return make_regular_path_f_forward_if(m_sink).
            degenerate_segment(xi, yi, dx, dy, xf, yf);
    }

    void do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return make_regular_path_f_forward_if(m_sink).
            cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return make_regular_path_f_forward_if(m_sink).
            inner_cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return make_regular_path_f_forward_if(m_sink).
            begin_segment_piece(xi, yi, dxi, dyi);
    }

    void do_end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return make_regular_path_f_forward_if(m_sink).
            end_segment_piece(dxf, dyf, xf, yf);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1) {
        return make_regular_path_f_forward_if(m_sink).
            linear_segment_piece(ti, tf, x0, y0, x1, y1);
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return make_regular_path_f_forward_if(m_sink).
            quadratic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return make_regular_path_f_forward_if(m_sink).
            rational_quadratic_segment_piece(ti, tf, x0, y0,
            x1, y1, w1, x2, y2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return make_regular_path_f_forward_if(m_sink).
            cubic_segment_piece(ti, tf, x0, y0, x1, y1,
            x2, y2, x3, y3);
    }

friend i_cubic_parameters<path_f_forward_if<SINK>>;

    void do_inflection_parameter(rvgf t) {
        return make_cubic_parameters_f_forward_if(m_sink).
            inflection_parameter(t);
    }

    void do_double_point_parameter(rvgf t) {
        return make_cubic_parameters_f_forward_if(m_sink).
            double_point_parameter(t);
    }

friend i_monotonic_parameters<path_f_forward_if<SINK>>;

    void do_root_dx_parameter(rvgf t) {
        return make_monotonic_parameters_f_forward_if(m_sink).
            root_dx_parameter(t);
    }

    void do_root_dy_parameter(rvgf t) {
        return make_monotonic_parameters_f_forward_if(m_sink).
            root_dy_parameter(t);
    }

    void do_root_dw_parameter(rvgf t) {
        return make_monotonic_parameters_f_forward_if(m_sink).
            root_dw_parameter(t);
    }

friend i_offsetting_parameters<path_f_forward_if<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        return make_offsetting_parameters_f_forward_if(m_sink).
            offset_cusp_parameter(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        return make_offsetting_parameters_f_forward_if(m_sink).
            evolute_cusp_parameter(t);
    }

friend i_join_parameters<path_f_forward_if<SINK>>;

    void do_join_tangent_parameter(rvgf t) {
        return make_join_parameters_f_forward_if(m_sink).
            join_tangent_parameter(t);
    }

    void do_join_vertex_parameter(rvgf t) {
        return make_join_parameters_f_forward_if(m_sink).
            join_vertex_parameter(t);
    }

friend i_decorated_path<path_f_forward_if<SINK>>;

    void do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return make_decorated_path_f_forward_if(m_sink).
            initial_cap(x, y, dx, dy);
    }

    void do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return make_decorated_path_f_forward_if(m_sink).
            terminal_cap(dx, dy, x, y);
    }

    void do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return make_decorated_path_f_forward_if(m_sink).
            initial_butt_cap(x, y, dx, dy);
    }

    void do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return make_decorated_path_f_forward_if(m_sink).
            terminal_butt_cap(dx, dy, x, y);
    }

    void do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return make_decorated_path_f_forward_if(m_sink).
            backward_initial_cap(x, y, dx, dy);
    }

    void do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return make_decorated_path_f_forward_if(m_sink).
            backward_terminal_cap(dx, dy, x, y);
    }

    void do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return make_decorated_path_f_forward_if(m_sink).
            backward_initial_butt_cap(x, y, dx, dy);
    }

    void do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return make_decorated_path_f_forward_if(m_sink).
            backward_terminal_butt_cap(dx, dy, x, y);
    }

    void do_join(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return make_decorated_path_f_forward_if(m_sink).
            join(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_inner_join(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return make_decorated_path_f_forward_if(m_sink).
            inner_join(dxi, dyi, x, y, dxf, dyf, w);
    }

friend i_dashing_parameters<path_f_forward_if<SINK>>;

    void do_begin_dash_parameter(rvgf t) {
        return make_dashing_parameters_f_forward_if(m_sink).
            begin_dash_parameter(t);
    }

    void do_end_dash_parameter(rvgf t) {
        return make_dashing_parameters_f_forward_if(m_sink).
            end_dash_parameter(t);
    }

    void do_backward_begin_dash_parameter(rvgf t) {
        return make_dashing_parameters_f_forward_if(m_sink).
            backward_begin_dash_parameter(t);
    }

    void do_backward_end_dash_parameter(rvgf t) {
        return make_dashing_parameters_f_forward_if(m_sink).
            backward_end_dash_parameter(t);
    }

};


template <typename SINK>
auto make_path_f_forward_if(lua_State *L, SINK &&sink) {
    return path_f_forward_if<SINK>{L, std::forward<SINK>(sink)};
}

}

using namespace rvg;

template <typename P, typename ...As, typename F, size_t ...Is>
static int invoke_method_helper(lua_State *L, F f, std::index_sequence<Is...>) {
    P *p = rvg_lua_check_pointer<P>(L, 1);
    ((*p).*f)(rvg_lua_check<As>(L, Is+2)...);
    return 0;
}

template <typename I, typename P, typename ...As>
static int invoke_method(lua_State *L, void (I::*f)(As...)) {
    return invoke_method_helper<P, As...>(L, f,
        std::make_index_sequence<sizeof...(As)>{});
}

template <typename P>
static int lua_path_f_to_path_begin_contour(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf>(L, &I::begin_contour);
}

template <typename P>
static int lua_path_f_to_path_end_open_contour(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf>(L, &I::end_open_contour);
}

template <typename P>
static int lua_path_f_to_path_end_closed_contour(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf>(L, &I::end_closed_contour);
}

template <typename P>
static int lua_path_f_to_path_linear_segment(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf,rvgf,rvgf,rvgf>(L, &I::linear_segment);
}

template <typename P>
static int lua_path_f_to_path_quadratic_segment(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::quadratic_segment);
}

template <typename P>
static int lua_path_f_to_path_rational_quadratic_segment(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::rational_quadratic_segment);
}

template <typename P>
static int lua_path_f_to_path_cubic_segment(lua_State *L) {
    using I = i_input_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::cubic_segment);
}

template <typename P>
static int lua_path_f_to_path_cusp(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::cusp);
}

template <typename P>
static int lua_path_f_to_path_inner_cusp(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::inner_cusp);
}

template <typename P>
static int lua_path_f_to_path_begin_regular_contour(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::begin_regular_contour);
}

template <typename P>
static int lua_path_f_to_path_end_regular_open_contour(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::end_regular_open_contour);
}

template <typename P>
static int lua_path_f_to_path_end_regular_closed_contour(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::end_regular_closed_contour);
}

template <typename P>
static int lua_path_f_to_path_degenerate_segment(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::degenerate_segment);
}

template <typename P>
static int lua_path_f_to_path_begin_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::begin_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_end_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::end_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_linear_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::linear_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_quadratic_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::quadratic_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_rational_quadratic_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
           rvgf>(L, &I::rational_quadratic_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_cubic_segment_piece(lua_State *L) {
    using I = i_regular_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
           rvgf, rvgf>(L, &I::cubic_segment_piece);
}

template <typename P>
static int lua_path_f_to_path_inflection_parameter(lua_State *L) {
    using I = i_cubic_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::inflection_parameter);
}

template <typename P>
static int lua_path_f_to_path_double_point_parameter(lua_State *L) {
    using I = i_cubic_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::double_point_parameter);
}

template <typename P>
static int lua_path_f_to_path_root_dx_parameter(lua_State *L) {
    using I = i_monotonic_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::root_dx_parameter);
}

template <typename P>
static int lua_path_f_to_path_root_dy_parameter(lua_State *L) {
    using I = i_monotonic_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::root_dy_parameter);
}

template <typename P>
static int lua_path_f_to_path_root_dw_parameter(lua_State *L) {
    using I = i_monotonic_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::root_dw_parameter);
}

template <typename P>
static int lua_path_f_to_path_offset_cusp_parameter(lua_State *L) {
    using I = i_offsetting_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::offset_cusp_parameter);
}

template <typename P>
static int lua_path_f_to_path_evolute_cusp_parameter(lua_State *L) {
    using I = i_offsetting_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::evolute_cusp_parameter);
}

template <typename P>
static int lua_path_f_to_path_join_tangent_parameter(lua_State *L) {
    using I = i_join_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::join_tangent_parameter);
}

template <typename P>
static int lua_path_f_to_path_join_vertex_parameter(lua_State *L) {
    using I = i_join_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::join_vertex_parameter);
}

template <typename P>
static int lua_path_f_to_path_initial_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(L, &I::initial_cap);
}

template <typename P>
static int lua_path_f_to_path_terminal_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(L, &I::terminal_cap);
}

template <typename P>
static int lua_path_f_to_path_backward_initial_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::backward_initial_cap);
}

template <typename P>
static int lua_path_f_to_path_backward_terminal_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::backward_terminal_cap);
}

template <typename P>
static int lua_path_f_to_path_initial_butt_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(L, &I::initial_butt_cap);
}

template <typename P>
static int lua_path_f_to_path_terminal_butt_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(L, &I::terminal_butt_cap);
}

template <typename P>
static int lua_path_f_to_path_backward_initial_butt_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::backward_initial_butt_cap);
}

template <typename P>
static int lua_path_f_to_path_backward_terminal_butt_cap(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf>(
        L, &I::backward_terminal_butt_cap);
}

template <typename P>
static int lua_path_f_to_path_join(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::join);
}

template <typename P>
static int lua_path_f_to_path_inner_join(lua_State *L) {
    using I = i_decorated_path<P>;
    return invoke_method<I, P, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf>(
        L, &I::inner_join);
}

template <typename P>
static int lua_path_f_to_path_begin_dash_parameter(lua_State *L) {
    using I = i_dashing_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::begin_dash_parameter);
}

template <typename P>
static int lua_path_f_to_path_end_dash_parameter(lua_State *L) {
    using I = i_dashing_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::end_dash_parameter);
}

template <typename P>
static int lua_path_f_to_path_backward_begin_dash_parameter(lua_State *L) {
    using I = i_dashing_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::backward_begin_dash_parameter);
}

template <typename P>
static int lua_path_f_to_path_backward_end_dash_parameter(lua_State *L) {
    using I = i_dashing_parameters<P>;
    return invoke_method<I, P, rvgf>(L, &I::backward_end_dash_parameter);
}

template <typename P>
static int lua_path_f_to_path_init(lua_State *L, const char *path_f_name,
    int ctxidx) {
    static luaL_Reg lua_path_f_to_path__index[] = {
        {"begin_contour", &lua_path_f_to_path_begin_contour<P>},
        {"end_open_contour",
            &lua_path_f_to_path_end_open_contour<P>},
        {"end_closed_contour",
            &lua_path_f_to_path_end_closed_contour<P>},
        {"linear_segment", &lua_path_f_to_path_linear_segment<P>},
        {"quadratic_segment",
            &lua_path_f_to_path_quadratic_segment<P>},
        {"rational_quadratic_segment",
            &lua_path_f_to_path_rational_quadratic_segment<P>},
        {"cubic_segment", &lua_path_f_to_path_cubic_segment<P>},
        {"begin_regular_contour",
            &lua_path_f_to_path_begin_regular_contour<P>},
        {"end_regular_open_contour",
            &lua_path_f_to_path_end_regular_open_contour<P>},
        {"end_regular_closed_contour",
            &lua_path_f_to_path_end_regular_closed_contour<P>},
        {"degenerate_segment",
            &lua_path_f_to_path_degenerate_segment<P>},
        {"cusp", &lua_path_f_to_path_cusp<P>},
        {"inner_cusp", &lua_path_f_to_path_inner_cusp<P>},
        {"begin_segment_piece",
            &lua_path_f_to_path_begin_segment_piece<P>},
        {"end_segment_piece",
            &lua_path_f_to_path_end_segment_piece<P>},
        {"linear_segment_piece",
            &lua_path_f_to_path_linear_segment_piece<P>},
        {"quadratic_segment_piece",
            &lua_path_f_to_path_quadratic_segment_piece<P>},
        {"rational_quadratic_segment_piece",
            &lua_path_f_to_path_rational_quadratic_segment_piece<P>},
        {"regular_cubic_segment",
            &lua_path_f_to_path_cubic_segment_piece<P>},
        {"inflection_parameter",
            &lua_path_f_to_path_inflection_parameter<P>},
        {"double_point_parameter",
            &lua_path_f_to_path_double_point_parameter<P>},
        {"root_dx_parameter",
            &lua_path_f_to_path_root_dx_parameter<P>},
        {"root_dy_parameter",
            &lua_path_f_to_path_root_dy_parameter<P>},
        {"root_dw_parameter",
            &lua_path_f_to_path_root_dw_parameter<P>},
        {"offset_cusp_parameter",
            &lua_path_f_to_path_offset_cusp_parameter<P>},
        {"evolute_cusp_parameter",
            &lua_path_f_to_path_evolute_cusp_parameter<P>},
        {"join_tangent_parameter",
            &lua_path_f_to_path_join_tangent_parameter<P>},
        {"join_vertex_parameter",
            &lua_path_f_to_path_join_vertex_parameter<P>},
        {"initial_cap", &lua_path_f_to_path_initial_cap<P>},
        {"terminal_cap", &lua_path_f_to_path_terminal_cap<P>},
        {"backward_initial_cap",
            &lua_path_f_to_path_backward_initial_cap<P>},
        {"backward_terminal_cap",
            &lua_path_f_to_path_backward_terminal_cap<P>},
        {"join", &lua_path_f_to_path_join<P>},
        {"inner_join", &lua_path_f_to_path_inner_join<P>},
        {"initial_butt_cap",
            &lua_path_f_to_path_initial_butt_cap<P>},
        {"terminal_butt_cap",
            &lua_path_f_to_path_terminal_butt_cap<P>},
        {"backward_initial_butt_cap",
            &lua_path_f_to_path_backward_initial_butt_cap<P>},
        {"backward_terminal_butt_cap",
            &lua_path_f_to_path_backward_terminal_butt_cap<P>},
        {"begin_dash_parameter",
            &lua_path_f_to_path_begin_dash_parameter<P>},
        {"end_dash_parameter",
            &lua_path_f_to_path_end_dash_parameter<P>},
        {"backward_begin_dash_parameter",
            &lua_path_f_to_path_backward_begin_dash_parameter<P>},
        {"backward_end_dash_parameter",
            &lua_path_f_to_path_backward_end_dash_parameter<P>},
        { nullptr, nullptr }
    };

    if (!rvg_lua_typeexists<P>(L, ctxidx)) {
        rvg_lua_createtype<P>(L, path_f_name, ctxidx);
        rvg_lua_setmethods<P>(L, lua_path_f_to_path__index, 0, ctxidx);
    }

    return 0;
}

static auto make_lua_input_path_f_close_all_contours(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_close_all_contours(
            make_path_f_to_lua_path_ref(L, 1)));
}

static int filter_make_input_path_f_close_all_contours(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_close_all_contours(L));
}

static auto make_lua_input_path_f_close_contours(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_close_contours(
            make_path_f_to_lua_path_ref(L, 1)));
}

static int filter_make_input_path_f_close_contours(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_close_contours(L));
}

static auto make_lua_input_path_f_downgrade_degenerates(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_downgrade_degenerates(
            luaL_checknumber(L, 1),
            make_path_f_to_lua_path_ref(L, 2)));
}

static int filter_make_input_path_f_downgrade_degenerates(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_downgrade_degenerates(L));
}

static auto make_lua_input_path_f_xform(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_xform(rvg_lua_check<xform>(L, 1),
            make_path_f_to_lua_path_ref(L, 2)));
}

static int filter_make_input_path_f_xform(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_xform(L));
}

static auto make_lua_path_f_spy_snk(lua_State *L) {
    return make_path_f_forward_if(L,
        make_path_f_spy(luaL_checkstring(L, 1)));
}

static auto make_lua_path_f_spy_fwd(lua_State *L) {
    return make_path_f_forward_if(L,
        make_path_f_spy(luaL_checkstring(L, 1),
            make_path_f_to_lua_path_ref(L, 2)));
}

static int filter_make_path_f_spy(lua_State *L) {
    if (lua_gettop(L) < 2) {
        return rvg_lua_push(L, make_lua_path_f_spy_snk(L));
    } else {
        return rvg_lua_push(L, make_lua_path_f_spy_fwd(L));
    }
}

static auto make_lua_input_path_f_find_cubic_parameters(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_find_cubic_parameters(
            make_path_f_to_lua_path_ref(L, 1)));
}

static int filter_make_input_path_f_find_cubic_parameters(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_find_cubic_parameters(L));
}

static auto make_lua_input_path_f_rational_quadratic_to_cubics(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_rational_quadratic_to_cubics(
            make_path_f_to_lua_path_ref(L, 1)));
}

static int filter_make_input_path_f_rational_quadratic_to_cubics(lua_State *L) {
    return rvg_lua_push(L,
        make_lua_input_path_f_rational_quadratic_to_cubics(L));
}

static auto make_lua_input_path_f_find_monotonic_parameters(lua_State *L) {
    return make_path_f_forward_if(L,
        make_input_path_f_find_monotonic_parameters(
            make_path_f_to_lua_path_ref(L, 1)));
}

static int filter_make_input_path_f_find_monotonic_parameters(lua_State *L) {
    return rvg_lua_push(L, make_lua_input_path_f_find_monotonic_parameters(L));
}

static const luaL_Reg modfilter[] = {
    {"make_input_path_f_xform", filter_make_input_path_f_xform},
    {"make_path_f_spy", filter_make_path_f_spy},
    {"make_input_path_f_close_all_contours", filter_make_input_path_f_close_all_contours},
    {"make_input_path_f_close_contours", filter_make_input_path_f_close_contours},
    {"make_input_path_f_downgrade_degenerates",
        filter_make_input_path_f_downgrade_degenerates},
    {"make_input_path_f_find_cubic_parameters",
        filter_make_input_path_f_find_cubic_parameters},
    {"make_input_path_f_find_monotonic_parameters",
        filter_make_input_path_f_find_monotonic_parameters},
    {"make_input_path_f_rational_quadratic_to_cubics",
        filter_make_input_path_f_rational_quadratic_to_cubics},
    {NULL, NULL}
};

int rvg_lua_filters_init(lua_State *L, int ctxidx) {
    rvg_lua_xform_init(L, ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_xform(nullptr))
    >(L, "input_path_f_xform", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_close_all_contours(nullptr))
    >(L, "input_path_f_close_all_contours", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_close_contours(nullptr))
    >(L, "input_path_f_close_contours", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_downgrade_degenerates(nullptr))
    >(L, "input_path_f_downgrade_degenerates", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_path_f_spy_snk(nullptr))
    >(L, "path_f_spy sink", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_path_f_spy_fwd(nullptr))
    >(L, "path_f_spy forwarder", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_find_cubic_parameters(nullptr))
    >(L, "input_path_f_find_cubic_parameters", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_find_monotonic_parameters(nullptr))
    >(L, "input_path_f_find_monotonic_parameters", ctxidx);

    lua_path_f_to_path_init<
        decltype(make_lua_input_path_f_rational_quadratic_to_cubics(nullptr))
    >(L, "input_path_f_rational_quadratic_to_cubics", ctxidx);

    return 0;
}

int rvg_lua_path_filters_export(lua_State *L, int ctxidx) {
    // mod
    rvg_lua_filters_init(L, ctxidx); // mod
    lua_pushvalue(L, ctxidx); // mod ctxtab
    rvg_lua_setfuncs(L, modfilter, 1); // mod
    return 1;
}
