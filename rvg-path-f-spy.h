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
#ifndef RVG_PATH_F_SPY_H
#define RVG_PATH_F_SPY_H

#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <utility>

#include "rvg-i-path.h"
#include "rvg-i-point-path.h"
#include "rvg-path-f-null.h"

namespace rvg {

template <typename SINK>
class path_f_spy final:
    public meta::inherit_if_i_input_path<SINK,
        i_input_path<path_f_spy<SINK>>>,
    public meta::inherit_if_i_regular_path<SINK,
        i_regular_path<path_f_spy<SINK>>>,
    public meta::inherit_if_i_decorated_path<SINK,
        i_decorated_path<path_f_spy<SINK>>>,
    public meta::inherit_if_i_monotonic_parameters<SINK,
        i_monotonic_parameters<path_f_spy<SINK>>>,
    public meta::inherit_if_i_cubic_parameters<SINK,
        i_cubic_parameters<path_f_spy<SINK>>>,
    public meta::inherit_if_i_offsetting_parameters<SINK,
        i_offsetting_parameters<path_f_spy<SINK>>>,
    public meta::inherit_if_i_join_parameters<SINK,
        i_join_parameters<path_f_spy<SINK>>>,
    public meta::inherit_if_i_dashing_parameters<SINK,
        i_dashing_parameters<path_f_spy<SINK>>>
{

    std::string m_name;
    std::ostream &m_out;
    SINK m_sink;

public:

    explicit path_f_spy(const char *name, std::ostream &out, SINK &&sink):
        m_name(name),
        m_out(out),
        m_sink(std::forward<SINK>(sink)) {
        m_out << std::setprecision(std::numeric_limits<rvgf>::digits10);
            ;
    }

private:

friend i_input_path<path_f_spy<SINK>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        m_out << m_name << "begin_contour("
            << x0 << ", " << y0 << ")\n";
        return m_sink.begin_contour(x0, y0);
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        m_out << m_name << "end_open_contour("
            << x0 << ", " << y0 << ")\n";
        return m_sink.end_open_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        m_out << m_name << "end_closed_contour("
            << x0 << ", " << y0 << ")\n";
        return m_sink.end_closed_contour(x0, y0);
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        m_out << m_name << "linear_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ")\n";
        return m_sink.linear_segment(x0, y0, x1, y1);
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        m_out << m_name << "quadratic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ")\n";
        return m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        m_out << m_name << "rational_quadratic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", " << w1 << ", "
            << x2 << ", " << y2 << ")\n";
        return m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1,
            x2, y2);
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        m_out << m_name << "cubic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ", "
            << x3 << ", " << y3 << ")\n";
        return m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

friend i_regular_path<path_f_spy<SINK>>;

    void do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        m_out << m_name << "begin_regular_contour("
            << xi << ", " << yi << ", "
            << dxi << ", " << dyi << ")\n";
        return m_sink.begin_regular_contour(xi, yi, dxi, dyi);
    }

    void do_end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        m_out << m_name << "end_regular_open_contour("
            << dxf << ", " << dyf << ", "
            << xf << ", " << yf << ")\n";
        return m_sink.end_regular_open_contour(dxf, dyf, xf, yf);
    }

    void do_end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        m_out << m_name << "end_regular_closed_contour("
            << dxf << ", " << dyf << ", "
            << xf << ", " << yf << ")\n";
        return m_sink.end_regular_closed_contour(dxf, dyf, xf, yf);
    }

    void do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
            rvgf xf, rvgf yf) {
        m_out << m_name << "degenerate_segment("
            << xi << ", " << yi << ", "
            << dx << ", " << dy << ", "
            << xf << ", " << yf << ")\n";
        return m_sink.degenerate_segment(xi, yi, dx, dy, xf, yf);
    }

    void do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        m_out << m_name << "cusp("
            << dxi << ", " << dyi << ", "
            << x << ", " << y << ", "
            << dxf << ", " << dyf << ", "
            << w << ")\n";
        return m_sink.cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        m_out << m_name << "inner_cusp("
            << dxi << ", " << dyi << ", "
            << x << ", " << y << ", "
            << dxf << ", " << dyf << ", "
            << w << ")\n";
        return m_sink.inner_cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        m_out << m_name << "begin_segment_piece("
            << xi << ", " << yi << ", "
            << dxi << ", " << dyi << ")\n";
        return m_sink.begin_segment_piece(xi, yi, dxi, dyi);
    }

    void do_end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        m_out << m_name << "end_segment_piece("
            << dxf << ", " << dyf << ", "
            << xf << ", " << yf << ")\n";
        return m_sink.end_segment_piece(dxf, dyf, xf, yf);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1) {
        m_out << m_name << "linear_segment_piece("
            << ti << ", " << tf << ", "
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ")\n";
        return m_sink.linear_segment_piece(ti, tf, x0, y0, x1, y1);
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        m_out << m_name << "quadratic_segment_piece("
            << ti << ", " << tf << ", "
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ")\n";
        return m_sink.quadratic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        m_out << m_name << "rational_quadratic_segment_piece("
            << ti << ", " << tf << ", "
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", " << w1 << ", "
            << x2 << ", " << y2 << ")\n";
        return m_sink.rational_quadratic_segment_piece(ti, tf, x0, y0,
            x1, y1, w1, x2, y2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        m_out << m_name << "cubic_segment_piece("
            << ti << ", " << tf << ", "
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ", "
            << x3 << ", " << y3 << ")\n";
        return m_sink.cubic_segment_piece(ti, tf, x0, y0, x1, y1,
            x2, y2, x3, y3);
    }

friend i_cubic_parameters<path_f_spy<SINK>>;

    void do_inflection_parameter(rvgf t) {
        m_out << m_name << "inflection_parameter(" << t << ")\n";
        return m_sink.inflection_parameter(t);
    }

    void do_double_point_parameter(rvgf t) {
        m_out << m_name << "double_point_parameter(" << t << ")\n";
        return m_sink.double_point_parameter(t);
    }

friend i_monotonic_parameters<path_f_spy<SINK>>;

    void do_root_dx_parameter(rvgf t) {
        m_out << m_name << "root_dx_parameter(" << t << ")\n";
        return m_sink.root_dx_parameter(t);
    }

    void do_root_dy_parameter(rvgf t) {
        m_out << m_name << "root_dy_parameter(" << t << ")\n";
        return m_sink.root_dy_parameter(t);
    }

    void do_root_dw_parameter(rvgf t) {
        m_out << m_name << "root_dw_parameter(" << t << ")\n";
        return m_sink.root_dw_parameter(t);
    }

friend i_offsetting_parameters<path_f_spy<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        m_out << m_name << "offset_cusp_parameter(" << t << ")\n";
        return m_sink.offset_cusp_parameter(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        m_out << m_name << "evolute_cusp_parameter(" << t << ")\n";
        return m_sink.evolute_cusp_parameter(t);
    }

friend i_join_parameters<path_f_spy<SINK>>;

    void do_join_tangent_parameter(rvgf t) {
        m_out << m_name << "join_tangent_parameter(" << t << ")\n";
        return m_sink.join_tangent_parameter(t);
    }

    void do_join_vertex_parameter(rvgf t) {
        m_out << m_name << "join_vertex_parameter(" << t << ")\n";
        return m_sink.join_vertex_parameter(t);
    }

friend i_decorated_path<path_f_spy<SINK>>;

    void do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        m_out << m_name << "initial_cap("
            << x << ", " << y << ", "
            << dx << ", " << dy << ")\n";
        return m_sink.initial_cap(x, y, dx, dy);
    }

    void do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        m_out << m_name << "terminal_cap("
            << dx << ", " << dy << ", "
            << x << ", " << y << ")\n";
        return m_sink.terminal_cap(dx, dy, x, y);
    }

    void do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        m_out << m_name << "initial_butt_cap("
            << x << ", " << y << ", "
            << dx << ", " << dy << ")\n";
        return m_sink.initial_butt_cap(x, y, dx, dy);
    }

    void do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        m_out << m_name << "terminal_butt_cap("
            << dx << ", " << dy << ", "
            << x << ", " << y << ")\n";
        return m_sink.terminal_butt_cap(dx, dy, x, y);
    }

    void do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        m_out << m_name << "backward_initial_cap("
            << x << ", " << y << ", "
            << dx << ", " << dy << ")\n";
        return m_sink.backward_initial_cap(x, y, dx, dy);
    }

    void do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        m_out << m_name << "backward_terminal_cap("
            << dx << ", " << dy << ", "
            << x << ", " << y << ")\n";
        return m_sink.backward_terminal_cap(dx, dy, x, y);
    }

    void do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        m_out << m_name << "backward_initial_butt_cap("
            << x << ", " << y << ", "
            << dx << ", " << dy << ")\n";
        return m_sink.backward_initial_butt_cap(x, y, dx, dy);
    }

    void do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        m_out << m_name << "backward_terminal_butt_cap("
            << dx << ", " << dy << ", "
            << x << ", " << y << ")\n";
        return m_sink.backward_terminal_butt_cap(dx, dy, x, y);
    }

    void do_join(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        m_out << m_name << "join("
            << dxi << ", " << dyi << ", "
            << x << ", " << y << ", "
            << dxf << ", " << dyf << ", "
            << w << ")\n";
        return m_sink.join(dxi, dyi, x, y, dxf, dyf, w);
    }

    void do_inner_join(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        m_out << m_name << "inner_join("
            << dxi << ", " << dyi << ", "
            << x << ", " << y << ", "
            << dxf << ", " << dyf << ", "
            << w << ")\n";
        return m_sink.inner_join(dxi, dyi, x, y, dxf, dyf, w);
    }

friend i_dashing_parameters<path_f_spy<SINK>>;

    void do_begin_dash_parameter(rvgf t) {
        m_out << m_name << "begin_dash_parameter(" << t << ")\n";
        return m_sink.begin_dash_parameter(t);
    }

    void do_end_dash_parameter(rvgf t) {
        m_out << m_name << "end_dash_parameter(" << t << ")\n";
        return m_sink.end_dash_parameter(t);
    }

    void do_backward_begin_dash_parameter(rvgf t) {
        m_out << m_name << "backward_begin_dash_parameter(" << t << ")\n";
        return m_sink.backward_begin_dash_parameter(t);
    }

    void do_backward_end_dash_parameter(rvgf t) {
        m_out << m_name << "backward_end_dash_parameter(" << t << ")\n";
        return m_sink.backward_end_dash_parameter(t);
    }

};

template <typename SINK>
inline path_f_spy<SINK> make_path_f_spy(const char *name, std::ostream &out,
    SINK &&sink) {
    return path_f_spy<SINK>(name, out, std::forward<SINK>(sink));
}

template <typename SINK>
inline path_f_spy<SINK> make_path_f_spy(const char *name, SINK &&sink) {
    return path_f_spy<SINK>(name, std::cerr, std::forward<SINK>(sink));
}

inline path_f_spy<path_f_null> make_path_f_spy(const char *name,
    std::ostream &out = std::cerr) {
    return path_f_spy<path_f_null>(name, out, make_path_f_null());
}

} // namespace rvg

#endif
