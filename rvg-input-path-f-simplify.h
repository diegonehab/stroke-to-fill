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
#ifndef RVG_INPUT_PATH_F_SIMPLIFY_H
#define RVG_INPUT_PATH_F_SIMPLIFY_H

#include <type_traits>
#include <utility>

#include "rvg-i-input-path.h"

namespace rvg {

template <typename SINK>
class input_path_f_simplify final:
    public i_sink<input_path_f_simplify<SINK>>,
    public i_input_path<input_path_f_simplify<SINK>> {

    rvgf m_tol;
    SINK m_sink;

    rvgf m_px, m_py, m_ppx, m_ppy;

public:

    explicit input_path_f_simplify(rvgf tol, SINK &&sink):
        m_tol{tol},
        m_sink{std::forward<SINK>(sink)} {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

    void close(rvgf x, rvgf y) {
        advance();
        if (m_px != x || m_py != y)
            m_sink.linear_segment(m_px, m_py, x, y);
    }

    void advance(void) {
        if (m_ppx != m_px || m_ppy != m_py)
            m_sink.linear_segment(m_ppx, m_ppy, m_px, m_py);
        m_ppx = m_px;
        m_ppy = m_py;
    }

friend i_input_path<input_path_f_simplify<SINK>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        m_sink.begin_contour(x0, y0);
        m_ppx = m_px = x0;
        m_ppy = m_py = y0;
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        close(x0, y0);
        m_sink.end_open_contour(x0, y0);
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        close(x0, y0);
        return m_sink.end_closed_contour(x0, y0);
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        rvgf x01 = x1 - x0;
        rvgf y01 = y1 - y0;
        //assert(m_px == x0 && m_py == y0);
        if (std::fabs(x01) > m_tol || std::fabs(y01) > m_tol) {
            rvgf dx = m_px - m_ppx;
            rvgf dy = m_py - m_ppy;
            if (std::fabs(dx) > m_tol || std::fabs(dy) > m_tol) {
                rvgf el2 = dx*dx + dy*dy;
                rvgf perp_d = dy*x01 - dx*y01;
                if (perp_d*perp_d < m_tol*m_tol*el2) {
                    m_px = x1; m_py = y1;
                    return;
                }
            }
            advance();
            m_ppx = x0; m_ppy = y0;
            m_px = x1; m_py = y1;
        }
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        advance();
        m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2);
        m_ppx = m_px = x2;
        m_ppy = m_py = y2;
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        advance();
        m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
        m_ppx = m_px = x2;
        m_ppy = m_py = y2;
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        advance();
        m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
        m_ppx = m_px = x3;
        m_ppy = m_py = y3;
    }

    void do_rational_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf w1,
        rvgf x2, rvgf y2, rvgf w2, rvgf x3, rvgf y3) {
        advance();
        m_sink.rational_cubic_segment(x0, y0, x1, y1, w1, x2, y2, w2, x3, y3);
        m_ppx = m_px = x3;
        m_ppy = m_py = y3;
    }

};

template <typename SINK>
auto make_input_path_f_simplify(rvgf tol, SINK &&sink) {
    return input_path_f_simplify<SINK>{tol, std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
