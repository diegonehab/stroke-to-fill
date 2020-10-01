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
#ifndef RVG_PATH_F_FIND_OFFSETING_PARAMETERS_H
#define RVG_PATH_F_FIND_OFFSETING_PARAMETERS_H

#include <type_traits>
#include <cmath>
#include <utility>

#include <boost/range/adaptor/sliced.hpp>

#include "rvg-i-parameters-f-forwarder.h"
#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-i-regular-path-f-forwarder.h"
#include "rvg-i-offsetting-parameters.h"
#include "rvg-tuple.h"
#include "rvg-bezier.h"
#include "rvg-util.h"

// The radius of curvature of a curve a(t) = (x(t), y(t)) is
//
//   r(t) = p(t)^(3/2)/q(t), where
//   p(t) = x'(t)^2 + y'(t)^2 and
//   q(t) = x'(t) y''(t) - x''(t) y'(t)
//
// The offset at distance +/- s from the curve will cross the
// evolute when r(t) = +/- s. At first sight, we could look
// for the roots by solving
//
//   p(t)^(3/2)/q(t) = +/- s, or
//   p(t)^3 - s^2 q(t)^2 = 0.
//
// Even though the last equation is a polynomial, its high
// degree is misleading. The equation can't have that many
// roots. To see this, look at the derivative of r(t)
//
//   r'(t) = p(t)^(1/2) (3 q(t) p'(t) - 2 q'(t) p(t)) /  q(t)^ 2
//
// The derivative only changes sign when
//
//   3 q(t) p'(t) - 2 q'(t) p(t) = 0.
//
// This polynomial is of much lower degree than p(t)^3. So,
// instead of directly solving for the roots of
//
//   p(t)^3 - s^2 q(t)^2 = 0
//
// we use the roots of
//
//   3 q(t) p'(t) - 2 q'(t) p(t) = 0.
//
// to split p(t)^(3/2)/q(t) into monotonic intervals. Within
// each interval, we use bisection to find the single potential root of
//
//   p(t)^(3/2) - s q(t) = 0 or
//   p(t)^(3/2) + s q(t) = 0

namespace rvg {

template <typename SINK>
class path_f_find_offsetting_parameters final:
    public i_sink<path_f_find_offsetting_parameters<SINK>>,
    public meta::inherit_if_i_regular_path<SINK,
        i_regular_path_f_forwarder<
            path_f_find_offsetting_parameters<SINK>>>,
    public meta::inherit_if_i_input_path<SINK,
        i_input_path_f_forwarder<
            path_f_find_offsetting_parameters<SINK>>>,
    public i_parameters_f_forwarder<SINK,
        path_f_find_offsetting_parameters<SINK>> {

    SINK m_sink;
    rvgf m_offset;

public:

    explicit path_f_find_offsetting_parameters(
        rvgf offset, SINK &&sink):
        m_sink(std::forward<SINK>(sink)), m_offset(offset) {
        static_assert(
            meta::is_an_i_regular_path<SINK>::value ||
            meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_regular_path and not an i_input_path");
        static_assert(meta::is_an_i_offsetting_parameters<SINK>::value,
            "sink is not an i_offsetting_parameters");
    }

private:

friend i_sink<path_f_find_offsetting_parameters<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

    // In the case of quadratics, the polynomial
    //
    //   3 q(t) p'(t) - 2 q'(t) p(t) = 0
    //
    // is linear and has a single root at
    //
    //   t = -b/a with
    //   b = (x1-x0)(x0-2x1+x2) + (y1-y0)(y0-2y1+y2) and
    //   a = (x0-2x1+x2)^2 + (y0-2y1+y2)^2
    //
    // Moreover, the polynomial
    //
    //   q(t) = 4(x2(y0-y1) + x0(y1-y2) + x1(y2-y0))
    //
    // is constant. This means the curvature has constant sign and we do not
    // need to test for intersections against both offsets +/- s.
    //
    // Finally,
    //
    //   p(t) = ((1-t)*2(x1-x0)+t*2(x2-x1))^2 + ((1-t)*2(y1-y0)+t*2(y2-y1))^2
    //
    template <typename SEG>
    void process_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, SEG sink_segment) {
        rvgf b = (x1-x0)*(x0-rvgf{2}*x1+x2) + (y1-y0)*(y0-rvgf{2}*y1+y2);
        rvgf a = util::sq(x0-rvgf{2}*x1+x2) + util::sq(y0-rvgf{2}*y1+y2);
        rvgf q = rvgf{4.}*(x2*(y0-y1) + x0*(y1-y2) + x1*(y2-y0));
        rvgf dx0 = rvgf{2.}*(x1-x0), dx1 = rvgf{2.}*(x2-x1);
        rvgf dy0 = rvgf{2.}*(y1-y0), dy1 = rvgf{2.}*(y2-y1);
        auto f = [&](rvgf t) {
            rvgf p = util::sq((rvgf{1.}-t)*dx0 + t*dx1) +
                util::sq((rvgf{1.}-t)*dy0 + t*dy1);
            return p*std::sqrt(p);
        };
        rvgf z = m_offset*std::fabs(q);
        rvgf s = rvgf(util::sgn(a));
        b *= s; a *= s;
        // two intervals to test for roots
        if (a != 0 && -b >= ti*a && -b <= tf*a) {
            rvgf root = rvgf{0.};
            rvgf c = -b/a;
            m_sink.evolute_cusp_parameter(c);
            if (bisect<rvgf>(f, ti, c, z, &root)) {
                m_sink.offset_cusp_parameter(root);
            }
            if (bisect<rvgf>(f, c, tf, z, &root)) {
                m_sink.offset_cusp_parameter(root);
            }
        // one interval to test for roots
        } else {
            rvgf root = rvgf{0.};
            if (bisect(f, ti, tf, z, &root)) {
                m_sink.offset_cusp_parameter(root);
            }
        }
        return sink_segment();
    }

    // In the case of rational quadratics, we have
    //
    //   x(t) = u(t)/w(t) and y(t) = v(t)/w(t)
    //
    // Substituting in q(t), we obtain
    //
    //   q(t) = 4 Det / w(t)^3
    //
    // where Det is the determinant of the control-point matrix.
    //
    // This simplifies the polynomial
    //
    //   3 q(t) p'(t) - 2 q'(t) p(t)
    //
    // to
    //
    //   3 q(t) (p'(t) w(t) + 2 p(t) w'(t)) / w(t)
    //
    // Neither q(t) nor w(t) can vanish---since elliptic arcs have no
    // inflections or ideal points---the only changes in sign come from
    // the zeros of
    //
    //   p'(t) w(t) + 2 p(t) w'(t).
    //
    // (In the case of hyperbola with ideal points in the
    // interval [0,1], we do need to consider the roots of w(t) as well.)
    //
    // We can simplify this further to
    //
    //   [wu](t) ( w(t) [wu]'(t) - w'(t) [wu](t) ) +
    //   [wv](t) ( w(t) [wv]'(t) - w'(t) [wv](t) )
    //
    // where [ab](t) = a(t)b'(t) - b(t)a'(t) is a notational shortcut.
    //
    // Subtractions cause cancellations that lower the
    // degrees of [ab](t) and of a(t) [ab]'(t) - a'(t) [ab](t)
    // so that the final polynomial has degree 4
    //
    // These roots isolate the roots of
    //
    //   p(t)^(3/2) - s q(t) = 0   and p(t)^(3/2) + s q(t) = 0
    //
    // which we can find by bisection.
    //
    // For elliptical arcs, we can simplify this further because
    //
    //   p(t) >= 0 and
    //   q(t) > 0 or q(t) < 0
    //   (since q(t) = 4 Det / w(t)^3 and w(t) cannot change sign)
    //
    // Therefore, we only need to find the roots of
    //
    //   p(t)^(3/2) - s |q(t)| = 0
    //
    template <typename SEG>
    void process_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf u0, rvgf v0,
        rvgf u1, rvgf v1, rvgf w1, rvgf u2, rvgf v2, SEG sink_segment) {
        // ensure we are dealing with an elliptic arc
        assert(w1 > rvgf{-1.} && w1 < rvgf{1.});
        using namespace boost::adaptors;
        auto s = m_offset;
        // build wu(t) ( w(t) wu'(t) - w'(t) wu(t) ) +
        //   wv(t) ( w(t) wv'(t) - w'(t) wv(t) )
        auto u = std::make_tuple(u0, u1, u2);
        auto v = std::make_tuple(v0, v1, v2);
        auto w = std::make_tuple(rvgf{1.}, w1, rvgf{1.});
        auto du = bezier_derivative(u);
        auto dv = bezier_derivative(v);
        auto dw = bezier_derivative(w);
        auto wu = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, du),
                bezier_product<rvgf>(u, dw),
                std::minus<rvgf>{}
            )
        );
        auto dwu = bezier_derivative(wu);
        auto wv = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, dv),
                bezier_product<rvgf>(v, dw),
                std::minus<rvgf>{}
            )
        );
        auto dwv = bezier_derivative(wv);
        auto c = tuple_zip_map(
            bezier_product<rvgf>(
                wu,
                bezier_lower_degree(
                    tuple_zip_map(
                        bezier_product<rvgf>(w, dwu),
                        bezier_product<rvgf>(dw, wu),
                        std::minus<rvgf>{}
                    )
                )
            ),
            bezier_product<rvgf>(
                wv,
                bezier_lower_degree(
                    tuple_zip_map(
                        bezier_product<rvgf>(w, dwv),
                        bezier_product<rvgf>(dw, wv),
                        std::minus<rvgf>{}
                    )
                )
            ),
            std::plus<rvgf>{}
        );
        // find the roots of wu(t) ( w(t) wu'(t) - w'(t) wu(t) ) +
        //   wv(t) ( w(t) wv'(t) - w'(t) wv(t) )
        auto critical = bezier_roots<rvgf>(c, ti, tf);
        for (auto t: critical | sliced(1, critical.size()-1)) {
            m_sink.evolute_cusp_parameter(t);
        }
        // use these to bracket and solve for the roots of p(t)^(3/2) - s q(t)
        auto ts = refine_roots<rvgf>([&w, &wu, &wv, &dwu, &dwv, s](rvgf t) {
            rvgf wt = bezier_evaluate_horner<rvgf>(w, t);
            rvgf wut = bezier_evaluate_horner<rvgf>(wu, t);
            rvgf wvt = bezier_evaluate_horner<rvgf>(wv, t);
            rvgf dwut = bezier_evaluate_horner<rvgf>(dwu, t);
            rvgf dwvt = bezier_evaluate_horner<rvgf>(dwv, t);
            rvgf pt = wut*wut + wvt*wvt;
            rvgf qt = wut*dwvt - wvt*dwut;
            return pt*std::sqrt(pt) - std::fabs(s*wt*wt*qt);
        }, critical);
        for (auto t: ts | sliced(1, ts.size()-1)) {
            m_sink.offset_cusp_parameter(t);
        }
        return sink_segment();
    }

    // In the case of cubics, the polynomial
    //
    //   3 q(t) p'(t) - 2 q'(t) p(t)
    //
    // is a quintic and we have to solve for its roots.
    // Surprisingly, there are indeed cubics that lead to
    // this polynomial having 5 distinct real roots.
    //
    // (Its derivative is a quartic that can be explicitly factored into
    // two quadratics. This can be exploited to simplify root finding.)
    //
    // These roots isolate the roots of
    //
    //   p(t)^(3/2) - s q(t) = 0 and
    //   p(t)^(3/2) + s q(t) = 0,
    //
    // which we find by bisection
    //
    template <typename SEG>
    void process_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3, SEG sink_segment) {
        using namespace boost::adaptors;
        auto s = m_offset;
        // build 3 q(t) p'(t) - 2 q'(t) p(t)
        auto x = std::make_tuple(x0, x1, x2, x3);
        auto y = std::make_tuple(y0, y1, y2, y3);
        auto dx = bezier_derivative(x);
        auto dy = bezier_derivative(y);
        auto ddx = bezier_derivative(dx);
        auto ddy = bezier_derivative(dy);
        auto q = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(dx, ddy),
                bezier_product<rvgf>(dy, ddx),
                std::minus<rvgf>{}
            )
        );
        auto dq = bezier_derivative(q);
        auto p = tuple_zip_map(
            bezier_product<rvgf>(dx, dx),
            bezier_product<rvgf>(dy, dy),
            std::plus<rvgf>{}
        );
        auto dp = bezier_derivative(p);
        auto c = tuple_zip_map(
            bezier_product<rvgf>(q, dp),
            bezier_product<rvgf>(dq, p),
            [](rvgf a, rvgf b) {
                return rvgf{1.5}*a - b;
            }
        );
        // find roots of 3 q(t) p'(t) - 2 q'(t) p(t)
        // ??D we could compute its derivative, factor it
        // into the product of two quadratics (always possible) and use the
        // roots to isolate the roots of the polynomial we
        // care about. Here we skip this optimization.
        auto critical = bezier_roots<rvgf>(c, ti, tf);
        // ??D when looking for the roots of p(t)^(3/2) +/- s q(t)
        // we could find the roots of q(t) and use them
        // to separate the regions where we need to use + or -.
        // Here we skip this optimization.
        for (auto t: critical | sliced(1, critical.size()-1)) {
            m_sink.evolute_cusp_parameter(t);
        }
        // use these to bracket and solve for the roots of p(t)^(3/2) - s q(t)
        auto tp = refine_roots<rvgf>([&p, &q, s](rvgf t) {
            rvgf pt = bezier_evaluate_horner<rvgf>(p, t);
            rvgf qt = bezier_evaluate_horner<rvgf>(q, t);
            return pt*std::sqrt(pt) - s*qt;
        }, critical);
        for (auto t: tp | sliced(1, tp.size()-1)) {
            if (!util::is_almost_zero(t) && !util::is_almost_one(t))
                m_sink.offset_cusp_parameter(t);
        }
        // do the same for p(t)^(3/2) + s q(t)
        auto tm = refine_roots<rvgf>([&p, &q, s](rvgf t) {
            rvgf pt = bezier_evaluate_horner<rvgf>(p, t);
            rvgf qt = bezier_evaluate_horner<rvgf>(q, t);
            return pt*std::sqrt(pt) + s*qt;
        }, critical);
        for (auto t: tm | sliced(1, tm.size()-1)) {
            if (!util::is_almost_zero(t) && !util::is_almost_one(t))
                m_sink.offset_cusp_parameter(t);
        }
        return sink_segment();
    }

friend i_regular_path<path_f_find_offsetting_parameters<SINK>>;

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return process_quadratic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2,
            [&](void) {
                m_sink.quadratic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2);
            }
        );
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf u0, rvgf v0,
        rvgf u1, rvgf v1, rvgf w1, rvgf u2, rvgf v2) {
        return process_rational_quadratic_segment_piece(ti, tf, u0, v0, u1, v1, w1,
            u2, v2,
            [&](void) {
                m_sink.rational_quadratic_segment_piece(ti, tf, u0, v0, u1, v1, w1,
                    u2, v2);
            }
        );
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return process_cubic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2, x3, y3,
            [&](void) {
                m_sink.cubic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2, x3, y3);
            }
        );
    }

friend i_input_path<path_f_find_offsetting_parameters<SINK>>;

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return process_quadratic_segment_piece(0, 1, x0, y0, x1, y1, x2, y2,
            [&](void) { m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2); });
    }

    void do_rational_quadratic_segment(rvgf u0, rvgf v0, rvgf u1, rvgf v1, rvgf w1,
        rvgf u2, rvgf v2) {
        return process_rational_quadratic_segment_piece(0, 1, u0, v0, u1, v1, w1, u2, v2,
            [&](void) {
                m_sink.rational_quadratic_segment(u0, v0, u1, v1, w1, u2, v2);
            }
        );
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        return process_cubic_segment_piece(0, 1, x0, y0, x1, y1, x2, y2, x3, y3,
            [&](void) { m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3); });
    }

};

template <typename SINK>
inline auto
make_path_f_find_offsetting_parameters(rvgf offset, SINK &&sink) {
    return path_f_find_offsetting_parameters<SINK>{offset,
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
