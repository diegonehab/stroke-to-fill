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
#ifndef RVG_INPUT_PATH_F_FIND_JOIN_PARAMETERS_H
#define RVG_INPUT_PATH_F_FIND_JOIN_PARAMETERS_H

#include <type_traits>
#include <utility>

#include <boost/container/static_vector.hpp>

#include "rvg-tuple.h"
#include "rvg-bezier.h"
#include "rvg-i-point-regular-path-f-forwarder.h"
#include "rvg-i-join-parameters.h"
#include "rvg-i-parameters-f-forwarder.h"

namespace rvg {


// Take a segment piece and draw a circle of a given radius centered at its
// first endpoint.
// The join_tangent_parameter gives the parameters such that the normal
// to the segment is tangent to the circle.
//
// Given a curve s(t), center c, and radius r, we need parameters such that
//
// 1) dot(s-c, s'/|s'|) = +- r.
//
// To find the roots, we first partition into monotonic intervals. So we
// differentiate and equate to zero. This results a polynomial equation
//
// 2) |s'|^2 ( |s'|^2 + dot(s-c, s'') ) - dot(s-c, s') dot(s', s'') = 0
//
// After we solve for the roots of (2), we use them to bracket the roots of (1)
//
// Now take a vertex. The join_vertex_parameter gives the parameters such that
// the perpendincular to the segment goes through the vertex.
//
// Given a curve s(t) and a vertex v, we need parameters such that
//
// 3) dot(s-v, s') = 0
//
// We solve this polynomial equation directly
//
template <typename SINK>
class regular_path_f_find_join_parameters final:
    public i_sink<regular_path_f_find_join_parameters<SINK>>,
    public i_point_regular_path_f_forwarder<
        regular_path_f_find_join_parameters<SINK>>,
    public i_parameters_f_forwarder<SINK,
        regular_path_f_find_join_parameters<SINK>> {

    rvgf m_radius;
    R2 m_vertex;

    SINK m_sink;

public:

    explicit regular_path_f_find_join_parameters(rvgf radius, const R2 &vertex,
        SINK &&sink):
        m_radius(radius),
        m_vertex(vertex),
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_join_parameters<SINK>::value,
            "sink is not an i_join_parameters");
        static_assert(meta::is_an_i_regular_path<SINK>::value,
            "sink is not an i_regular_path");
    }

private:

friend i_sink<regular_path_f_find_join_parameters<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_regular_path<regular_path_f_find_join_parameters<SINK>>;

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        // The linear case is simple because the segment piece
        // intersects the circle iff it's length is greater than the radius
        auto len2_01 = len2(p1-p0);
        auto dt = tf-ti;
        auto len2_titf = len2_01*dt*dt;
        if (len2_titf > m_radius*m_radius) {
            rvgf u = m_radius/std::sqrt(len2_titf);
            m_sink.join_tangent_parameter(ti+(tf-ti)*u);
        }
        // For the vertex case, we find the parameter for the projection of the
        // vertex into the line segment and check if it is inside the range
        auto u = dot(m_vertex-p0, p1-p0); // avoid division by len2_01
        if ((u-ti*len2_01)*(u-tf*len2_01) < 0) {
            m_sink.join_vertex_parameter(u/len2_01);
        }
        m_sink.linear_segment_piece(ti, tf, p0, p1);
    }

    template <typename S, typename DS>
    void integral_tangent_parameter(rvgf ti, rvgf tf, const S &s,
        const DS &ds) {
        auto center = bezier_evaluate_horner(s, ti);
        auto d2s = bezier_derivative(ds);
        auto sc = tuple_map(s, [&](const R2 &p) { return p-center; }); // s-c
        auto ds2 = bezier_dot<rvgf, R2>(ds, ds); // |s'|^2
        // Compute the Bernstein representation for the polynomial
        // |s'|^2 ( |s'|^2 + dot(s-c, s'') ) + dot(s-c, s') dot(s', s'')
        auto p = tuple_zip_map(
            bezier_product<rvgf>(
                ds2,
                tuple_zip_map(
                    ds2,
                    bezier_dot<rvgf, R2>(sc, d2s),
                    std::plus<rvgf>{}
                )
            ),
            bezier_product<rvgf>(bezier_dot<rvgf, R2>(sc, ds),
                bezier_dot<rvgf, R2>(ds, d2s)),
            std::minus<rvgf>{}
        );
        // Find roots
        auto critical = bezier_roots<rvgf>(p, ti, tf);
        // Use to bracket roots of dot(s-c, s'/|s'|) = +r.
        auto radius = m_radius;
        auto fp = [&](rvgf t) -> rvgf {
            auto dst = bezier_evaluate_horner<rvgf>(ds, t);
            auto sct = bezier_evaluate_horner<rvgf>(sc, t);
            return dot(sct, dst) - std::sqrt(dot(dst, dst))*radius;
        };
        auto rp = refine_roots(fp, critical);
        // Use to bracket roots of dot(s-c, s'/|s'|) = -r.
        auto fm = [&](rvgf t) -> rvgf {
            auto dst = bezier_evaluate_horner<rvgf>(ds, t);
            auto sct = bezier_evaluate_horner<rvgf>(sc, t);
            return dot(sct, dst) + std::sqrt(dot(dst, dst))*radius;
        };
        auto rm = refine_roots(fm, critical);
        // Merge roots in sorted order and return
        // we don't need ti and tf, so the don't need all the slots
        static_vector<rvgf, rp.static_capacity+rm.static_capacity-4> r;
        std::merge(rp.begin()+1, rp.end()-1, rm.begin()+1, rm.end()-1,
            std::back_inserter(r));
        if (r.size() > 0) {
            m_sink.join_tangent_parameter(r[0]);
        }
    }

    template <typename S, typename DS>
    void integral_vertex_parameter(rvgf ti, rvgf tf, const S &s, const DS &ds) {
        auto vertex = m_vertex;
        auto sv = tuple_map(s, [&](const R2 &p) { return p-vertex; }); // s-v
        auto p = bezier_dot<rvgf, R2>(sv, ds);
        auto r = bezier_roots<rvgf>(p, ti, tf);
        if (r.size() > 2) {
            m_sink.join_vertex_parameter(r[1]);
        }
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        auto s = std::make_tuple(p0, p1, p2);
        auto ds = bezier_derivative(s);
        integral_tangent_parameter(ti, tf, s, ds);
        integral_vertex_parameter(ti, tf, s, ds);
        m_sink.quadratic_segment_piece(ti, tf, p0, p1, p2);
    }

    template <typename B>
    auto bezier_elevate_3_degrees(const B &b) {
        return bezier_elevate_degree<rvgf>(
            bezier_elevate_degree<rvgf>(
                bezier_elevate_degree<rvgf>(b)));
    }

    template <typename W, typename D0, typename D1>
    void rational_vertex_parameter(rvgf ti, rvgf tf, const W &w,
        const D0 &d0, const D1 &d1) {
        // Compute d0 - wv
        auto vertex = m_vertex;
        auto d0mwv = tuple_zip_map(
            d0,
            tuple_map(w, [vertex](rvgf wi) {
                return wi*vertex;
            }),
            [](const R2 &a, const R2 &b) { return a - b; }
        );
        auto p = bezier_dot<rvgf, R2>(d0mwv, d1);
        auto r = bezier_roots<rvgf>(p, ti, tf);
        if (r.size() > 2) {
            m_sink.join_vertex_parameter(r[1]);
        }
    }

    template <typename W, typename DW, typename D0, typename D1, typename D2>
    void rational_tangent_parameter(rvgf ti, rvgf tf, const R2 &center,
        const W &w, const DW &dw, const D0 &d0, const D1 &d1, const D2 &d2) {
        // Compute d0 - wc
        auto d0mwc = tuple_zip_map(
            d0,
            tuple_map(w, [center](rvgf wi) {
                return wi*center;
            }),
            [](const R2 &a, const R2 &b) { return a - b; }
        );
        // Compute dot(d1, d1)
        auto d1d1 = bezier_dot<rvgf, R2>(d1, d1);
        // Compute d3 = w d2 - 2 w' d1
        auto d3 = tuple_zip_map(
            bezier_product<rvgf>(w, d2),
            bezier_product<rvgf>(dw, d1),
            [](const R2 &a, const R2 &b) -> R2 {
                return a - 2*b;
            }
        );

        // Finally, compute
        // dot(d1, d1) ( dot(d1, d1) + dot(d0 - w c, d3) ) -
        //   dot(d0 - w c, d1) dot(d1, d3) = 0
        auto p = tuple_zip_map(
            bezier_product<rvgf>(
                d1d1,
                tuple_zip_map(
                    bezier_elevate_degree<rvgf>(d1d1),
                    bezier_dot<rvgf, R2>(d0mwc, d3),
                    std::plus<rvgf>{}
                )
            ),
            bezier_product<rvgf>(
                bezier_dot<rvgf, R2>(d0mwc, d1),
                bezier_dot<rvgf, R2>(d1, d3)
            ),
            std::minus<rvgf>{}
        );
        auto critical = bezier_roots<rvgf>(p, ti, tf);
        auto radius = m_radius;
        // Use to bracket roots of dot(d0 - w c, d1) = + r w |d1|
        auto fp = [&](rvgf t) -> rvgf {
            auto d0mwct = bezier_evaluate_horner<rvgf>(d0mwc, t);
            auto wt = bezier_evaluate_horner(w, t);
            auto d1t = bezier_evaluate_horner<rvgf>(d1, t);
            return dot(d0mwct, d1t) - radius*wt*len(d1t);
        };
        auto rp = refine_roots(fp, critical);
        // Use to bracket roots of dot(d0 - w c, d1) = - r w |d1|
        auto fm = [&](rvgf t) -> rvgf {
            auto d0mwct = bezier_evaluate_horner<rvgf>(d0mwc, t);
            auto wt = bezier_evaluate_horner(w, t);
            auto d1t = bezier_evaluate_horner<rvgf>(d1, t);
            return dot(d0mwct, d1t) + radius*wt*len(d1t);
        };
        auto rm = refine_roots(fm, critical);
        // Merge roots in sorted order and return
        // we don't need ti and tf, so the don't need all the slots
        static_vector<rvgf, rp.static_capacity+rm.static_capacity-4> r;
        std::merge(rp.begin()+1, rp.end()-1, rm.begin()+1, rm.end()-1,
            std::back_inserter(r));
        if (r.size() > 0) {
            m_sink.join_tangent_parameter(r[0]);
        }
    }

    template <typename U, typename DU, typename W, typename DW>
    auto bezier_cr(const U &u, const DU &du, const W &w, const DW &dw) {
        return tuple_zip_map(
            bezier_product<rvgf>(w, du),
            bezier_product<rvgf>(u, dw),
            std::minus<rvgf>{}
        );
    }

    // In the case of rational quadratics, we have
    //
    //   r(t) = [u(t), v(t), w(t)]
    //
    //   x(t) = u(t)/w(t) and y(t) = v(t)/w(t)
    //
    // Let
    //
    //   d0 = [u, v]
    //   d1 = [w u'- u w', w v' - v w']
    //   d2 = [w u''- u w'', w v'' - v w'']
    //   d3 = w d2 - 2 w' d1
    //
    // Then, for the tangent case,
    //
    // |s'|^2 ( |s'|^2 + dot(s-c, s'') ) + dot(s-c, s') dot(s', s'') = 0
    //
    // becomes
    //
    // dot(d1, d1) ( dot(d1, d1) + dot(d0 - w c, d3) ) +
    //   dot(d0 - w c, d1) dot(d1, d3) = 0
    //
    // and
    //
    //   dot(s-c, s'/|s'|) = +- r.
    //
    // becomes
    //
    //   dot(d0 - w c, d1) = +- r w |d1|
    //
    // For the vertex case,
    //
    //   dot(s-v, s') = 0
    //
    // becomes
    //
    //   dot(d0 - w v, d1) = 0
    //
    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        auto u = std::make_tuple(p0[0], p1[0], p2[0]);
        auto du = bezier_derivative(u);
        auto d2u = bezier_derivative(du);
        auto v = std::make_tuple(p0[1], p1[1], p2[1]);
        auto dv = bezier_derivative(v);
        auto d2v = bezier_derivative(dv);
        auto w = std::make_tuple(p0[2], p1[2], p2[2]);
        auto dw = bezier_derivative(w);
        auto d2w = bezier_derivative(dw);
        auto d0 = tuple_zip_map(u, v, [](rvgf ui, rvgf vi) {
            return R2{ui, vi};
        });
        auto d1 = bezier_lower_degree(
            tuple_zip_map(
                this->bezier_cr(u, du, w, dw),
                this->bezier_cr(v, dv, w, dw),
                [](rvgf ui, rvgf vi) {
                    return R2{ui, vi};
                }
            )
        );
        auto d2 = bezier_lower_degree(
            tuple_zip_map(
                this->bezier_cr(u, d2u, w, d2w),
                this->bezier_cr(v, d2v, w, d2w),
                [](rvgf ui, rvgf vi) {
                    return R2{ui, vi};
                }
            )
        );
        R2 center = project<R2>(R3{
            bezier_evaluate_horner(u, ti),
            bezier_evaluate_horner(v, ti),
            bezier_evaluate_horner(w, ti)
        });
        rational_tangent_parameter(ti, tf, center, w, dw, d0, d1, d2);
        rational_vertex_parameter(ti, tf, w, d0, d1);
        m_sink.rational_quadratic_segment_piece(ti, tf, p0, p1, p2);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        auto s = std::make_tuple(p0, p1, p2, p3);
        auto ds = bezier_derivative(s);
        integral_tangent_parameter(ti, tf, s, ds);
        integral_vertex_parameter(ti, tf, s, ds);
        m_sink.cubic_segment_piece(ti, tf, p0, p1, p2, p3);
    }
};

template <typename SINK>
inline auto make_regular_path_f_find_join_parameters(rvgf radius, const R2 &vertex, SINK &&sink) {
    return regular_path_f_find_join_parameters<SINK>{radius, vertex,
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
