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
#ifndef RVG_SVG_PATH_F_TO_INPUT_PATH_H
#define RVG_SVG_PATH_F_TO_INPUT_PATH_H

#include <utility>

#include "rvg-i-input-path.h"
#include "rvg-i-svg-path.h"
#include "rvg-point.h"
#include "rvg-xform.h"
#include "rvg-util.h"

namespace rvg {

template <typename SINK>
class svg_path_f_to_input_path final:
    public i_svg_path<svg_path_f_to_input_path<SINK>> {

    SINK m_sink;
    rvgf m_current_x, m_current_y;
    rvgf m_previous_x, m_previous_y;
    rvgf m_start_x, m_start_y;
    bool m_begun;

public:

    explicit svg_path_f_to_input_path(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
		this->reset();
	}

    void end(void) {
        ensure_ended();
    }

private:

friend i_svg_path<svg_path_f_to_input_path<SINK>>;

    void do_move_to_abs(rvgf x0, rvgf y0) {
        ensure_ended();
        m_sink.begin_contour(x0, y0);
        m_begun = true;
        set_start(x0, y0);
        set_current(x0, y0);
        set_previous(x0, y0);
    }

    void do_move_to_rel(rvgf x0, rvgf y0) {
        x0 += m_current_x;
        y0 += m_current_y;
        this->move_to_abs(x0, y0);
    }

    void do_close_path(void) {
        ensure_begun();
        m_sink.end_closed_contour(m_current_x, m_current_y);
        m_begun = false;
        set_current(m_start_x, m_start_y);
        set_previous(m_start_x, m_start_y);
    }

    void do_line_to_abs(rvgf x1, rvgf y1) {
        ensure_begun();
        m_sink.linear_segment(m_current_x, m_current_y, x1, y1);
        set_current(x1, y1);
        set_previous(x1, y1);
    }

    void do_line_to_rel(rvgf x1, rvgf y1) {
        x1 += m_current_x;
        y1 += m_current_y;
        this->line_to_abs(x1, y1);
    }

    void do_hline_to_abs(rvgf x1) {
        this->line_to_abs(x1, m_current_y);
    }

    void do_hline_to_rel(rvgf x1) {
        x1 += m_current_x;
        this->hline_to_abs(x1);
    }

    void do_vline_to_abs(rvgf y1) {
        this->line_to_abs(m_current_x, y1);
    }

    void do_vline_to_rel(rvgf y1) {
        y1 += m_current_y;
        this->vline_to_abs(y1);
    }

    void do_quad_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        ensure_begun();
        m_sink.quadratic_segment(m_current_x, m_current_y, x1, y1, x2, y2);
        set_previous(x1, y1);
        set_current(x2, y2);
    }

    void do_quad_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        x1 += m_current_x;
        y1 += m_current_y;
        x2 += m_current_x;
        y2 += m_current_y;
        this->quad_to_abs(x1, y1, x2, y2);
    }

    void do_squad_to_abs(rvgf x2, rvgf y2) {
        rvgf x1 = 2.f*m_current_x - m_previous_x;
        rvgf y1 = 2.f*m_current_y - m_previous_y;
        this->quad_to_abs(x1, y1, x2, y2);
    }

    void do_squad_to_rel(rvgf x2, rvgf y2) {
        x2 += m_current_x;
        y2 += m_current_y;
        this->squad_to_abs(x2, y2);
    }

    void do_rquad_to_abs(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        ensure_begun();
        m_sink.rational_quadratic_segment(m_current_x, m_current_y,
            x1, y1, w1, x2, y2);
        set_previous(x2, y2);
        set_current(x2, y2);
    }

    void do_rquad_to_rel(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        x1 += m_current_x*w1;
        y1 += m_current_y*w1;
        x2 += m_current_x;
        y2 += m_current_y;
        this->rquad_to_abs(x1, y1, w1, x2, y2);
    }

    void do_svg_arc_to_abs(rvgf rx, rvgf ry, rvgf rot_deg,
        rvgf ffa, rvgf ffs, rvgf x2, rvgf y2) {
        using namespace rvg::util;
        bool fa = (ffa != 0.f), fs = (ffs != 0.f);
        rvgf x0 = m_current_x, y0 = m_current_y;
        // radii are assumed positive
        rx = std::fabs(rx); ry = std::fabs(ry);
        // if radii are too small, we degenerate to line connecting endpoints
        if (is_almost_zero(rx) || is_almost_zero(ry)) {
            return m_sink.linear_segment(x0, y0, x2, y2);
        }
        rvgf rot_rad = rad(rot_deg);
        rvgf cos_rot = std::cos(rot_rad);
        rvgf sin_rot = std::sin(rot_rad);
        R2 p0{x0, y0}, p2{x2, y2};
        scaling S{1.f/rx, 1.f/ry};
        rotation R{cos_rot, -sin_rot};
        // we solve the problem in a new coordinate system
        // where rx=ry=1 and rot_deg=0, then we move the solution
        // back to the original coordinate system
        R2 q0 = S.apply(R.apply(p0));
        R2 q2 = S.apply(R.apply(p2));
        // direction perpendicular to line connecting endpoints
        R2 q20p = perp(q2-q0);
        // if transformed endpoints are too close, degenerate to
        // line segment connecting endpoints
        rvgf el2 = len2(q20p); // perp doesn't change length
        if (is_almost_zero(el2)) {
            return m_sink.linear_segment(x0, y0, x2, y2);
        }
        R2 mq = .5f*(q0+q2); // midpoint between transformed endpoints
        rvgf radius; // circle radius
        rvgf inv_radius; // its reciprocal
        rvgf offset; // distance from midpoint to center
        // center of circle, endpoint, and midpoint form a right triangle
        // hypotenuse is the circle radius, which has length 1
        // it connects the endpoint to the center
        // the segment connecting the midpoint and endpoint is a cathetus
        // the segment connecting midpoint and the center is the other cathetus
        rvgf el = std::sqrt(el2);
        rvgf inv_el = 1.f/el;
        // the length of the hypothenuse must be at least
        // as large as the length of the catheti.
        if (el2 > 4.f) {
            // otherwise, we grow the circle isotropically until they are equal
            radius = .5f*el;
            inv_radius = 2.f*inv_el;
            // in which case, the midpoint *is* the center
            offset = 0.f;
        } else {
            // circle with radius 1 is large enough
            radius = 1.f;
            inv_radius = 1.f;
            // length of the cathetus connecting the midpoint and the center
            offset = .5f*std::sqrt(4.f-el2);
        }
        // there are two possible circles. flags decide which one
        rvgf sign = (fa != fs)? 1.f : -1.f; // offset sign
        // to find circle center in new coordinate system,
        // simply offset midpoint in the perpendicular direction
        R2 cq = mq + (sign*offset*inv_el)*q20p;
        // middle weight is the cosine of half the sector angle
        rvgf w1 = std::fabs(dot(q0-cq, q20p)*inv_el*inv_radius);
        // if center was at the origin, this would be the
        // intermediate control point for the rational quadratic
        // so we translate it by the center cq
        RP2 q1 = RP2{(-sign*radius*inv_el)*q20p, w1} + RP2{cq};
        // move control point back to original coordinate system
        scaling iS{rx, ry};
        rotation iR{cos_rot, sin_rot};
        q1 = iR.apply(iS.apply(q1));
        // this selects the small arc. to select the large arc,
        // negate all coordinates of intermediate control point
        // ??D this is not a good idea. We should instead
        // split the arc into pieces that subintend at most 2*Pi/3 radians
        if (fa) {
            m_sink.rational_quadratic_segment(x0, y0, -q1[0], -q1[1], -q1[2],
                x2, y2);
        } else {
            m_sink.rational_quadratic_segment(x0, y0, q1[0], q1[1], q1[2],
                x2, y2);
        }
        set_previous(x2, y2);
        set_current(x2, y2);
    }

    void do_svg_arc_to_rel(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        x2 += m_current_x;
        y2 += m_current_y;
        this->svg_arc_to_abs(rx, ry, a, fa, fs, x2, y2);
    }

    void do_cubic_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        ensure_begun();
        m_sink.cubic_segment(m_current_x, m_current_y, x1, y1, x2, y2, x3, y3);
        set_previous(x2, y2);
        set_current(x3, y3);
    }

    void do_cubic_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        x1 += m_current_x;
        y1 += m_current_y;
        x2 += m_current_x;
        y2 += m_current_y;
        x3 += m_current_x;
        y3 += m_current_y;
        this->cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        rvgf x1 = 2.f*m_current_x - m_previous_x;
        rvgf y1 = 2.f*m_current_y - m_previous_y;
        this->cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_rel(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        x2 += m_current_x;
        y2 += m_current_y;
        x3 += m_current_x;
        y3 += m_current_y;
        this->scubic_to_abs(x2, y2, x3, y3);
    }

    void reset(void) {
        m_begun = false;
        set_start(0.f, 0.f);
        set_current(0.f, 0.f);
        set_previous(0.f, 0.f);
    }

    void set_start(rvgf x, rvgf y) {
        m_start_x = x;
        m_start_y = y;
    }

    void set_current(rvgf x, rvgf y) {
        m_current_x = x;
        m_current_y = y;
    }

    void set_previous(rvgf x, rvgf y) {
        m_previous_x = x;
        m_previous_y = y;
    }

    void ensure_begun(void) {
        if (!m_begun) {
            m_sink.begin_contour(m_current_x, m_current_y);
            m_begun = true;
        }
    }

    void ensure_ended(void) {
        if (m_begun) {
            m_sink.end_open_contour(m_current_x, m_current_y);
            m_begun = false;
        }
    }
};

template <typename SINK>
auto make_svg_path_f_to_input_path(SINK &&sink) {
    return svg_path_f_to_input_path<SINK>(std::forward<SINK>(sink));
}

} // namespace rvg

#endif
