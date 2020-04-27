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
#ifndef RVG_INPUT_PATH_F_TO_SVG_PATH_H
#define RVG_INPUT_PATH_F_TO_SVG_PATH_H

#include <type_traits>

#include "rvg-i-input-path.h"
#include "rvg-i-svg-path.h"
#include "rvg-xform.h"
#include "rvg-xform-svd.h"

namespace rvg {

template <typename SINK>
class input_path_f_to_svg_path final:
    public i_input_path<input_path_f_to_svg_path<SINK>> {

    SINK m_sink;

public:

    explicit input_path_f_to_svg_path(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_svg_path<SINK>::value,
            "sink is not an i_svg_path");
    }

private:

friend i_input_path<input_path_f_to_svg_path<SINK>>;

    void do_begin_contour(rvgf x0, rvgf y0) {
        return m_sink.move_to_abs(x0, y0);
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        (void) x0; (void) y0;
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        (void) x0; (void) y0;
        return m_sink.close_path();
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        (void) x0; (void) y0;
        return m_sink.line_to_abs(x1, y1);
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        (void) x0; (void) y0;
        return m_sink.quad_to_abs(x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        using namespace ::rvg::util;
        // we start by computing the projective transformation that
        // maps the unit circle to the ellipse described by the control points
        rvgf s2 = 1.f-w1*w1;
        if (is_almost_zero(s2)) {
            if (is_almost_equal(x0, x1) && is_almost_equal(x1, x2) &&
               is_almost_equal(y0, y1) && is_almost_equal(y1, y2)) {
                // degenerate to a single point?
                return m_sink.line_to_abs(x2, y2);
            } else {
                // degenerate to a parabola?
                return m_sink.quad_to_abs(x1, y1, x2, y2);
            }
        }
        rvgf s = (s2 < 0.f? -1.f: 1.f)*std::sqrt(std::fabs(s2));
        // this is the linear part of the transformation;
        linearity L{2.f*x1-w1*(x0+x2), s*(x2-x0), 2.f*y1-w1*(y0+y2), s*(y2-y0)};
        // from it, we get the SVD
        rotation U; scaling S;
        asvd(L, U, S);
        // the sign of the middle weight gives the large/small angle flag
        rvgf fa = (w1 < 0.f)? 1.f: 0.f;
        // the sign of the area of the control point triangle gives the orientation
        projectivity T{x0, y0, 1., x1, y1, w1, x2, y2, 1.};
        rvgf fs = det(T) > 0.f? 1.f: 0.f;
        rvgf rx = S.get_sx()/(2.f*s2);
        rvgf ry = S.get_sy()/(2.f*s2);
        if (is_almost_zero(rx) || is_almost_zero(ry)) {
            m_sink.line_to_abs(x2, y2);
        } else {
            // the rotation and the scaling parts from SVD give the angle and axes
            return m_sink.svg_arc_to_abs(S.get_sx()/(2.f*s2), S.get_sy()/(2.f*s2),
                deg(std::atan2(U.get_sin(), U.get_cos())), fa, fs, x2, y2);
        }
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        (void) x0; (void) y0;
        return m_sink.cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

};

template <typename SINK>
auto make_input_path_f_to_svg_path(SINK &&sink) {
    return input_path_f_to_svg_path<SINK>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
