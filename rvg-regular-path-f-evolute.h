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
#ifndef RVG_POINT_PATH_F_EVOLUTE_H
#define RVG_POINT_PATH_F_EVOLUTE_H

#include <algorithm>
#include <boost/container/small_vector.hpp>

#include "rvg-i-point-regular-path-f-forwarder.h"
#include "rvg-i-monotonic-parameters.h"
#include "rvg-i-offsetting-parameters.h"
#include "rvg-bezier.h"
#include "rvg-bezier-arc-length.h"
#include "rvg-cubic-bezier-evolute-approximator.h"

namespace rvg {

template <typename SINK>
class point_path_f_evolute final:
    public i_point_regular_path<point_path_f_evolute<SINK>>,
    public i_monotonic_parameters<point_path_f_evolute<SINK>>,
    public i_offsetting_parameters<point_path_f_evolute<SINK>> {

    SINK m_sink;
    rvgf m_tol, m_max_radius;
    boost::container::small_vector<rvgf, 16> m_splits;

public:

    point_path_f_evolute(rvgf tol, rvgf max_radius, SINK &&sink):
        m_sink(std::forward<SINK>(sink)),
        m_tol(tol),
        m_max_radius(max_radius) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_point_regular_path<point_path_f_evolute<SINK>>;

    void do_begin_regular_contour(const R2 &p, const R2 &) {
        m_sink.begin_contour(p);
        m_splits.clear();
    }

    void do_end_regular_open_contour(const R2 &, const R2 &p) {
        m_sink.end_open_contour(p);
    }

    void do_end_regular_closed_contour(const R2 &, const R2 &p) {
        m_sink.end_closed_contour(p);
    }

    template <typename S, typename DS, typename DDS>
    auto get_endpoints(rvgf ti, rvgf tf, const S &s,
        const DS &ds, const DDS &dds) {
        auto qi = bezier_evaluate_horner(s, ti);
        auto dqi = bezier_evaluate_horner(ds, ti);
        auto ddqi = bezier_evaluate_horner(dds, ti);
        R2 di = tangent_direction(qi, dqi, ddqi);
        R2 pi = project<R2>(qi);
        auto ri = bounded_radius_of_curvature(m_max_radius, qi, dqi, ddqi);
        auto qf = bezier_evaluate_horner(s, tf);
        auto dqf = bezier_evaluate_horner(ds, tf);
        auto ddqf = bezier_evaluate_horner(dds, tf);
        R2 df = tangent_direction(qf, dqf, ddqf);
        R2 pf = project<R2>(qf);
        auto rf = bounded_radius_of_curvature(m_max_radius, qf, dqf, ddqf);
        return std::make_tuple(pi, di, ri, rf, df, pf);
    }

    void do_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_inner_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_degenerate_segment(const R2 &pi, const R2 &, const R2 &pf) {
        m_sink.linear_segment(pi, pf);
    }

    void do_begin_segment_piece(const R2 &, const R2 &) {
        ;
    }

    void do_end_segment_piece(const R2 &, const R2 &) {
        ;
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        // The evolute is at infinity, so we simply skip it
        auto s = std::tie(p0, p1);
        R2 pi = bezier_evaluate_horner(s, ti);
        R2 pf = bezier_evaluate_horner(s, tf);
        m_sink.linear_segment(pi, pf);
    }

    //??D The evolute of an integral quadratic is just an integral cubic.
    // Should we compute it explicitly?
    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        rvgf ri, rf;
        std::tie(pi, di, ri, rf, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto evolute = make_cubic_bezier_evolute_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_max_radius, s, ds, dds,
                m_sink);
        auto ni = (ri/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (rf/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        evolute.approximate_partition(ti, qi, perp(di), tf, qf, perp(df),
            m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        rvgf ri, rf;
        std::tie(pi, di, ri, rf, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto evolute = make_cubic_bezier_evolute_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_max_radius, s, ds, dds,
                m_sink);
        auto ni = (ri/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (rf/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        evolute.approximate_partition(ti, qi, perp(di), tf, qf, perp(df),
            m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2, const R2 &p3) {
        auto s = std::tie(p0, p1, p2, p3);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        rvgf ri, rf;
        std::tie(pi, di, ri, rf, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto evolute = make_cubic_bezier_evolute_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_max_radius, s, ds, dds,
                m_sink);
        auto ni = (ri/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (rf/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        evolute.approximate_partition(ti, qi, perp(di), tf, qf, perp(df),
            m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
    }

friend i_offsetting_parameters<point_path_f_evolute<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        (void) t;
    }

    void do_evolute_cusp_parameter(rvgf t) {
        m_splits.push_back(t);
    }

friend i_monotonic_parameters<point_path_f_evolute<SINK>>;

    void do_root_dx_parameter(rvgf t) {
        m_splits.push_back(t);
    }

    void do_root_dy_parameter(rvgf t) {
        m_splits.push_back(t);
    }

    void do_root_dw_parameter(rvgf t) {
        (void) t;
    }

};

template <typename SINK>
auto make_point_path_f_evolute(rvgf tol, rvgf max_radius, SINK &&sink) {
    return point_path_f_evolute<SINK>(tol, max_radius, std::forward<SINK>(sink));
}

template <typename SINK>
auto make_point_path_f_evolute(rvgf max_radius, SINK &&sink) {
    return point_path_f_evolute<SINK>(
        RVG_CUBIC_BEZIER_APPROXIMATION_TOLERANCE,
        max_radius,
        std::forward<SINK>(sink));
}

} // namespace rvg

#endif
