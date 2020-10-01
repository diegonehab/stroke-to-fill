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
#ifndef RVG_REGULAR_PATH_F_OFFSET_H
#define RVG_REGULAR_PATH_F_OFFSET_H

#include <algorithm>
#include <array>

#include "rvg-i-point-regular-path.h"
#include "rvg-i-monotonic-parameters.h"
#include "rvg-bezier.h"
#include "rvg-bezier-arc-length.h"
#include "rvg-cubic-bezier-offset-approximator.h"

namespace rvg {

template <typename SINK>
class regular_path_f_offset final:
    public i_point_regular_path<regular_path_f_offset<SINK>>,
    public i_monotonic_parameters<regular_path_f_offset<SINK>>,
    public i_offsetting_parameters<regular_path_f_offset<SINK>> {

    SINK m_sink;
    rvgf m_offset;
    rvgf m_tol;
    R2 m_p;
    boost::container::small_vector<rvgf, 16> m_splits;

public:

    explicit regular_path_f_offset(rvgf offset, rvgf tol, SINK &&sink):
        m_sink(std::forward<SINK>(sink)),
        m_offset(offset),
        m_tol(tol) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_point_regular_path<regular_path_f_offset<SINK>>;

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
    std::tuple<R2, R2, R2, R2> get_endpoints(rvgf ti, rvgf tf, const S &s,
        const DS &ds, const DDS &dds) {
        auto qi = bezier_evaluate_horner(s, ti);
        R2 di = tangent_direction(qi, bezier_evaluate_horner(ds, ti),
            bezier_evaluate_horner(dds, ti));
        R2 pi = project<R2>(qi);
        auto qf = bezier_evaluate_horner(s, tf);
        R2 df = tangent_direction(qf, bezier_evaluate_horner(ds, tf),
            bezier_evaluate_horner(dds, tf));
        R2 pf = project<R2>(qf);
        return std::make_tuple(pi, di, df, pf);
    }

    void do_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_inner_cusp(const R2 &, const R2 &, const R2 &, rvgf) {
        ;
    }

    void do_degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        auto n = (m_offset/len(d))*perp(d);
        auto qi = pi + n;
        //assert(qi == m_previous_p);
        auto qf = pf + n;
        m_sink.linear_segment(pi, qi);
        m_sink.linear_segment(qi, qf);
        m_sink.linear_segment(qf, pf);
    }

    void do_begin_segment_piece(const R2 &, const R2 &) {
        ;
    }

    void do_end_segment_piece(const R2 &, const R2 &) {
        ;
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        auto s = std::tie(p0, p1);
        R2 pi = bezier_evaluate_horner(s, ti);
        R2 pf = bezier_evaluate_horner(s, tf);
        auto d = p1-p0;
        auto n = (m_offset/len(d))*perp(d);
        auto qi = pi + n;
        auto qf = pf + n;
        m_sink.linear_segment(pi, qi);
        m_sink.linear_segment(qi, qf);
        m_sink.linear_segment(qf, pf);
        m_splits.clear();
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        std::tie(pi, di, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto offset = make_cubic_bezier_offset_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_offset, s, ds, dds,
            m_sink);
        auto ni = (m_offset/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (m_offset/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        std::sort(m_splits.begin(), m_splits.end());
        offset.approximate_partition(ti, qi, di, tf, qf, df, m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        std::tie(pi, di, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto offset = make_cubic_bezier_offset_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_offset, s, ds, dds,
                m_sink);
        auto ni = (m_offset/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (m_offset/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        std::sort(m_splits.begin(), m_splits.end());
        offset.approximate_partition(ti, qi, di, tf, qf, df, m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        auto s = std::tie(p0, p1, p2, p3);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        R2 pi, di, df, pf;
        std::tie(pi, di, df, pf) = get_endpoints(ti, tf, s, ds, dds);
        auto offset = make_cubic_bezier_offset_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_offset, s, ds, dds,
                m_sink);
        auto ni = (m_offset/len(di))*perp(di);
        auto qi = pi + ni;
        auto nf = (m_offset/len(df))*perp(df);
        auto qf = pf + nf;
        m_sink.linear_segment(pi, qi);
        std::sort(m_splits.begin(), m_splits.end());
        offset.approximate_partition(ti, qi, di, tf, qf, df, m_splits, m_tol);
        m_splits.clear();
        m_sink.linear_segment(qf, pf);
        m_sink.linear_segment(qf, pf);
    }

friend i_monotonic_parameters<regular_path_f_offset<SINK>>;

    void do_root_dx_parameter(rvgf t) {
        m_splits.push_back(t);
    }

    void do_root_dy_parameter(rvgf t) {
        m_splits.push_back(t);
    }

    void do_root_dw_parameter(rvgf t) {
        (void) t;
        assert(0);
    }

friend i_offsetting_parameters<regular_path_f_offset<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        m_splits.push_back(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        (void) t;
    }

};

template <typename SINK>
auto make_regular_path_f_offset(rvgf offset, rvgf tol, SINK &&sink) {
    return regular_path_f_offset<SINK>(offset, tol, std::forward<SINK>(sink));
}

template <typename SINK>
auto make_regular_path_f_offset(rvgf offset, SINK &&sink) {
    return make_regular_path_f_offset(offset,
        RVG_CUBIC_BEZIER_APPROXIMATION_TOLERANCE, std::forward<SINK>(sink));
}

} // namespace rvg

#endif
