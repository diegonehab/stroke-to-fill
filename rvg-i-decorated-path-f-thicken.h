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
#ifndef RVG_I_DECORATED_PATH_F_THICKEN_H
#define RVG_I_DECORATED_PATH_F_THICKEN_H

#include "rvg-i-dashing-parameters.h"
#include "rvg-i-point-decorated-path.h"
#include "rvg-i-point-regular-path.h"
#include "rvg-util.h"
#include "rvg-stroke-style.h"

namespace rvg {

template <typename DERIVED>
class i_decorated_path_f_thicken:
    public i_point_decorated_path<DERIVED>,
    public i_dashing_parameters<DERIVED>,
    public i_point_regular_path<DERIVED>
{
    rvgf m_offset;
    stroke_style::const_ptr m_style;
    rvgf m_limit;
    rvgf m_limit2;
    rvgf m_t0, m_t1;
    bool m_dash_initial_cap, m_rdash_initial_cap, m_end_dash_contour;

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_decorated_path_f_thicken(rvgf width, stroke_style::const_ptr style):
        m_offset(0.5f*width),
        m_style(style),
        m_limit(0.5f*width*style->get_miter_limit()),
        m_limit2(0.5f*0.5f*
            width*width*
            style->get_miter_limit()*style->get_miter_limit()),
        m_t0(0.f), m_t1(1.f),
        m_dash_initial_cap(false),
        m_rdash_initial_cap(false),
        m_end_dash_contour(false) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

protected:

    rvgf get_t0(void) const {
        return m_t0;
    }

    rvgf get_t1(void) const {
        return m_t1;
    }

    void prepare_for_next_segment(const R2 &p) {
        if (m_end_dash_contour) {
            derived().sink().end_closed_contour(p);
        }
        m_t0 = 0; m_t1 = 1;
        m_dash_initial_cap = m_rdash_initial_cap = m_end_dash_contour = false;
    }

    void prepare_for_current_segment(const R2 &p, const R2 &d) {
        assert(!(m_dash_initial_cap && m_rdash_initial_cap));
        if (m_dash_initial_cap) {
            sink_cap(m_style->get_dash_initial_cap(), true, p, d);
        }
        if (m_rdash_initial_cap) {
            sink_cap(m_style->get_dash_terminal_cap(), false, p, d);
        }

    }

private:

friend i_point_regular_path<DERIVED>;

    void do_begin_regular_contour(const R2 &, const R2 &) {
        assert(0); // removed by earlier filters
    }

    void do_end_regular_open_contour(const R2 &, const R2 &) {
        assert(0); // removed by earlier filters
    }

    void do_end_regular_closed_contour(const R2 &, const R2 &) {
        assert(0); // removed by earlier filters
    }

    void do_begin_segment_piece(const R2 &, const R2 &) {
        assert(0); // removed by earlier filters
    }

    void do_end_segment_piece(const R2 &, const R2 &) {
        assert(0); // removed by earlier filters
    }

    void do_cusp(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        (void) w;
        auto n0 = (1.f/len(d0))*perp(d0);
        auto n1 = (1.f/len(d1))*perp(d1);
        R2 q0 = p + m_offset*n0;
        R2 q1 = p + m_offset*n1;
        // decide if we need to complete the circle around the cusp
        if (0 && dot(-perp(n0), n1) > 0) {
            R2 r0 = p - m_offset*n1;
            R2 r1 = p - m_offset*n0;
            derived().sink().linear_segment(q0, p);
            derived().sink().linear_segment(p, r0);
            derived().sink().linear_segment(r0, q0);
            sink_arc(p, q0, n0, q1, n1);
            derived().sink().linear_segment(q1, r1);
            derived().sink().linear_segment(r1, p);
            derived().sink().linear_segment(p, q1);
        } else {
            sink_arc(p, q0, n0, q1, n1);
        }
    }

    void do_inner_cusp(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        auto n0 = (1.f/len(d0))*perp(d0);
        auto n1 = (1.f/len(d1))*perp(d1);
        R2 q0 = p + m_offset*n0;
        R2 q1 = p + m_offset*n1;
        sink_inner_join_round(p, q0, n0, q1, n1, w);
    }

friend i_point_decorated_path<DERIVED>;

    static rvgf get_cos_half(rvgf cos) {
        return std::sqrt(std::fabs(0.5f*(cos+1.f)));
    }

    void sink_arc(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        auto cos = std::max(rvgf{-1}, std::min(rvgf{1}, dot(n0, n2)));
        if (cos < rvgf{0.9995}) { // cos(2 deg)
            auto b = clockwise_bisector(n0, n2);
            rvgf w1 = get_cos_half(cos);
            derived().sink().rational_quadratic_segment(R3{q0},
                R3{m_offset*b+w1*c, w1}, R3{q2});
        } else {
            derived().sink().linear_segment(q0, q2);
        }
    }

    void sink_join_round(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        sink_arc(c, q0, n0, q2, n2);
    }

    void sink_inner_join_round(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2, rvgf w) {
        if (w > 1) {
            sink_inner_join_simple(c, q0, n0, q2, n2);
        } else if (w > 0) {
            sink_inner_join_pivot(c, q0, n0, q2, n2);
        } else {
            derived().sink().linear_segment(q0, c);
            derived().sink().linear_segment(c, q2);
            sink_arc(c, q2, n2, q0, n0);
            derived().sink().linear_segment(q0, c);
            derived().sink().linear_segment(c, q2);
        }
    }

    void sink_inner_join_bevel(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2, rvgf w) {
        if (w > 1) {
            sink_inner_join_simple(c, q0, n0, q2, n2);
        } else if (w > 0) {
            sink_inner_join_pivot(c, q0, n0, q2, n2);
        } else {
            derived().sink().linear_segment(q0, c);
            derived().sink().linear_segment(c, q2);
            derived().sink().linear_segment(q2, q0);
            derived().sink().linear_segment(q0, c);
            derived().sink().linear_segment(c, q2);
        }
    }

    void sink_inner_join_simple(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        (void) c; (void) n0; (void) n2;
        derived().sink().linear_segment(q0, q2);
    }

    void sink_inner_join_pivot(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        (void) n0; (void) n2;
        derived().sink().linear_segment(q0, c);
        derived().sink().linear_segment(c, q2);
    }

    void sink_join_bevel(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        (void) c; (void) n0; (void) n2;
        derived().sink().linear_segment(q0, q2);
    }

    void sink_join_miter_clip(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        auto sq = [](rvgf a) -> rvgf { return a*a; };
        auto d0 = perp(n0);
        auto d2 = perp(n2);
        auto v = intersection(q0, d0, q2, d2);
        // inside miter limit
        if (!util::is_almost_zero(v.get_w()) &&
            sq(v.get_x()-c.get_x()*v.get_w()) +
            sq(v.get_y()-c.get_y()*v.get_w()) < m_limit2*sq(v.get_w())) {
            auto m = project<R2>(v);
            derived().sink().linear_segment(q0, m);
            derived().sink().linear_segment(m, q2);
        // parallel case
        } else if (util::is_almost_zero(v.get_w())){
            // need join
            if (dot(n0, n2) < 0) {
                auto q10 = q0-d0*m_limit;
                auto q11 = q2+d2*m_limit;
                derived().sink().linear_segment(q0, q10);
                derived().sink().linear_segment(q10, q11);
                derived().sink().linear_segment(q11, q2);
            // no need for join
            } else {
                derived().sink().linear_segment(q0, q2);
            }
        // outside miter limit
        } else {
            // bevel direction
            auto d = q2-q0;
            // point away from center in direction
            // perpendicular to bevel direction, miter_limit
            // away from center
            auto m = c + (m_limit/len(d))*perp(d);
            // clip miter with line throgh m parallel to bevel
            auto q10 = project<R2>(intersection(m, d, q0, d0));
            auto q11 = project<R2>(intersection(m, d, q2, d2));
            derived().sink().linear_segment(q0, q10);
            derived().sink().linear_segment(q10, q11);
            derived().sink().linear_segment(q11, q2);
        }
    }

    void sink_join_miter_or_bevel(const R2 &c, const R2 &q0, const R2 &n0,
        const R2 &q2, const R2 &n2) {
        auto sq = [](rvgf a) -> rvgf { return a*a; };
        auto d0 = perp(n0);
        auto d2 = perp(n2);
        auto v = intersection(q0, d0, q2, d2);
        // inside miter limit
        if (!util::is_almost_zero(v.get_w())  &&
            sq(v.get_x()-c.get_x()*v.get_w()) +
            sq(v.get_y()-c.get_y()*v.get_w()) < m_limit2*sq(v.get_w())) {
            auto m = project<R2>(v);
            derived().sink().linear_segment(q0, m);
            derived().sink().linear_segment(m, q2);
        // outside miter limit
        } else {
            derived().sink().linear_segment(q0, q2);
        }
    }

    void do_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        (void) w;
        auto n0 = (1.f/len(d0))*perp(d0);
        auto n1 = (1.f/len(d1))*perp(d1);
        R2 q0 = p + m_offset*n0;
        R2 q1 = p + m_offset*n1;
        switch (m_style->get_join()) {
            case e_stroke_join::round:
                sink_join_round(p, q0, n0, q1, n1);
                break;
            case e_stroke_join::miter_clip:
                sink_join_miter_clip(p, q0, n0, q1, n1);
                break;
            case e_stroke_join::miter_or_bevel:
                sink_join_miter_or_bevel(p, q0, n0, q1, n1);
                break;
            case e_stroke_join::bevel:
            default:
                sink_join_bevel(p, q0, n0, q1, n1);
                break;
        }
    }

    //??D Maybe change parameters so less arithmetic needs
    //    to be done in each of these functions?
    void sink_round_cap(const R2 &q0, const R2 &q1, const R2 &n1) {
        derived().sink().rational_quadratic_segment(R3{q0}, R3{m_offset*perp(n1),0.f},
            R3{q1});
    }

    void sink_butt_cap(const R2 &q0, const R2 &q1, const R2 &n1) {
        (void) n1;
        derived().sink().linear_segment(q0, q1);
    }

    void sink_square_cap(const R2 &q0, const R2 &q1, const R2 &n1) {
        auto d = m_offset*perp(n1);
        auto p0 = q0+d;
        auto p1 = q1+d;
        derived().sink().linear_segment(q0, p0);
        derived().sink().linear_segment(p0, p1);
        derived().sink().linear_segment(p1, q1);
    }

    void sink_triangle_cap(const R2 &q0, const R2 &q1, const R2 &n1) {
        auto p = q1-m_offset*(n1-perp(n1));
        derived().sink().linear_segment(q0, p);
        derived().sink().linear_segment(p, q1);
    }

    void sink_fletching_cap(const R2 &q0, const R2 &q1, const R2 &n1) {
        auto d = m_offset*perp(n1);
        auto p0 = q0+d;
        auto p1 = q1-m_offset*n1;
        auto p2 = q1+d;
        derived().sink().linear_segment(q0, p0);
        derived().sink().linear_segment(p0, p1);
        derived().sink().linear_segment(p1, p2);
        derived().sink().linear_segment(p2, q1);
    }

    void sink_cap(e_stroke_cap cap, bool begin, const R2 &p, const R2 &d) {
        auto n1 = (1.f/len(d))*perp(d);
        auto q1 = p + m_offset*n1;
        auto q0 = p - m_offset*n1;
        if (begin) {
            derived().sink().begin_contour(q0);
        }
        switch (cap) {
            case e_stroke_cap::round:
                sink_round_cap(q0, q1, n1);
                break;
            case e_stroke_cap::square:
                sink_square_cap(q0, q1, n1);
                break;
            case e_stroke_cap::triangle:
                sink_triangle_cap(q0, q1, n1);
                break;
            case e_stroke_cap::fletching:
                sink_fletching_cap(q0, q1, n1);
                break;
            case e_stroke_cap::butt:
                sink_butt_cap(q0, q1, n1);
                break;
        }
    };

    void do_initial_cap(const R2 &p, const R2 &d) {
		sink_cap(m_style->get_initial_cap(), true, p, d);
    }

    void do_terminal_cap(const R2 &d, const R2 &p) {
        (void) d; (void) p;
    }

    void do_backward_initial_cap(const R2 &p, const R2 &d) {
		sink_cap(m_style->get_terminal_cap(), false, p, d);
    }

    void do_backward_terminal_cap(const R2 &d, const R2 &p) {
        (void) d; (void) p;
        derived().sink().end_closed_contour(p);
    }

    void do_initial_butt_cap(const R2 &p, const R2 &d) {
		sink_cap(e_stroke_cap::butt, true, p, d);
    }

    void do_terminal_butt_cap(const R2 &d, const R2 &p) {
        (void) d; (void) p;
    }

    void do_backward_initial_butt_cap(const R2 &p, const R2 &d) {
		sink_cap(e_stroke_cap::butt, false, p, d);
    }

    void do_backward_terminal_butt_cap(const R2 &d, const R2 &p) {
        (void) d;
        derived().sink().end_closed_contour(p);
    }

    void do_inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        (void) w;
        auto n0 = (1.f/len(d0))*perp(d0);
        auto n1 = (1.f/len(d1))*perp(d1);
        R2 q0 = p + m_offset*n0;
        R2 q1 = p + m_offset*n1;
        switch (m_style->get_inner_join()) {
            case e_stroke_join::bevel:
                sink_inner_join_bevel(p, q0, n0, q1, n1, w);
                break;
            default:
            case e_stroke_join::round:
                sink_inner_join_round(p, q0, n0, q1, n1, w);
                break;
        }
    }

friend i_dashing_parameters<DERIVED>;

    void do_begin_dash_parameter(rvgf t) {
        m_t0 = t;
        m_dash_initial_cap = true;
    }

    void do_end_dash_parameter(rvgf t) {
        m_t1 = t;
    }

    void do_backward_begin_dash_parameter(rvgf t) {
        m_t0 = t;
        m_rdash_initial_cap = true;
    }

    void do_backward_end_dash_parameter(rvgf t) {
        m_t1 = t;
        m_end_dash_contour = true;
    }

};

} // namespace rvg

#endif
