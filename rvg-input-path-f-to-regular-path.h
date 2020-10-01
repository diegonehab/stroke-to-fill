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
#ifndef RVG_INPUT_PATH_F_TO_REGULAR_PATH_H
#define RVG_INPUT_PATH_F_TO_REGULAR_PATH_H

#include <tuple>
#include <boost/container/small_vector.hpp>
#include <iomanip>

#include "rvg-util.h"
#include "rvg-bezier.h"
#include "rvg-i-point-input-path.h"
#include "rvg-i-parameters-f-hold.h"
#include "rvg-i-offsetting-parameters.h"
#include "rvg-adjacent-range.h"
#include "rvg-path-instruction.h"

#define RVG_REGULARITY_MIN_RADIUS (0.01)
#define RVG_REGULARITY_ANGULAR_TOLERANCE (0.5f)
#define RVG_REGULARITY_NUMERICAL_TOLERANCE (32)

namespace rvg {

using boost::container::small_vector;

template <typename SINK>
class input_path_f_to_regular_path final:
    public i_sink<input_path_f_to_regular_path<SINK>>,
    public i_parameters_f_hold<input_path_f_to_regular_path<SINK>>,
    public i_point_input_path<input_path_f_to_regular_path<SINK>> {

    rvgf m_min_radius;
    rvgf m_delta;
    SINK m_sink;
    bool m_begun;
    R2 m_pi, m_pf, m_df;
    rvgf m_gamma;

    small_vector<rvgf, 16> m_radius_crossings;

public:

    input_path_f_to_regular_path(rvgf min_radius, rvgf alpha,
        rvgf delta, SINK &&sink):
        m_min_radius(min_radius),
        m_delta(delta),
        m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_i_regular_path<SINK>::value,
            "sink is not an i_regular_path");
		m_gamma = std::sin(rvgf{ 0.5 }*util::rad(alpha));
        m_gamma *= m_gamma;
    }

private:

friend i_sink<input_path_f_to_regular_path<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

    void ensure_begun(const R2 &pi, const R2 &di,
        const R2 &df, const R2 &pf) {
        if (!m_begun) {
            m_sink.begin_regular_contour(pi, di);
            m_pi = pi;
            m_begun = true;
        }
        m_df = df;
        m_pf = pf;
    }

friend i_point_input_path<input_path_f_to_regular_path<SINK>>;

    void do_begin_contour(const R2 &pi) {
        m_begun = false;
        m_pi = m_pf = pi;
        m_df = R2{0,0};
    }

    void do_end_open_contour(const R2 &pf) {
        if (!m_begun) {
            do_linear_segment(m_pi, pf);
        }
        m_sink.end_regular_open_contour(m_df, m_pf);
        m_begun = false;
    }

    void do_end_closed_contour(const R2 &pf) {
        if (!m_begun) {
            do_linear_segment(m_pi, pf);
        }
        m_sink.end_regular_closed_contour(m_df, pf);
        m_begun = false;
    }

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        auto d = p1-p0;
        if (util::is_almost_zero(d[0]) && util::is_almost_zero(d[1])) {
            const R2 z{0,0};
            ensure_begun(p0, z, z, p1);
            m_sink.degenerate_segment(p0, z, p1);
        } else {
            ensure_begun(p0, d, d, p1);
            m_sink.begin_segment_piece(p0, d);
            m_sink.linear_segment_piece(rvgf{0}, rvgf{1}, p0, p1);
            m_sink.end_segment_piece(d, p1);
        }
    }

    template <
        typename S,
        typename DS,
        typename DDS,
        typename PREC_POL
    > bool is_regular(const S &s, const DS &ds, const DDS &dds,
        const PREC_POL& prec_pol, rvgf t) {
        bool precise_ds = bezier_evaluate_horner<rvgf>(prec_pol, t) > 0;
        auto st = bezier_evaluate_horner<rvgf>(s, t);
        auto dst = bezier_evaluate_horner<rvgf>(ds, t);
        auto ddst = bezier_evaluate_horner<rvgf>(dds, t);
        auto r = radius_of_curvature(st, dst, ddst);
        bool bounded_curv = r[0] > std::fabs(r[1]*m_min_radius);
        return bounded_curv && precise_ds;
    }

    template <
        typename S,
        typename DS,
        typename DDS,
        typename PREC_POL
    > bool is_regular(const S &s, const DS &ds, const DDS &dds,
        const PREC_POL& prec_pol, rvgf ti, rvgf tf) {
            return is_regular(s, ds, dds, prec_pol, 0.25*(3*ti+tf)) +
                   is_regular(s, ds, dds, prec_pol, 0.5*(ti+tf)) +
                   is_regular(s, ds, dds, prec_pol, 0.25*(ti+3*tf)) > 1;
    }

    template <
        typename T,
        size_t   N,
        typename S,
        typename DS,
        typename DDS,
        typename PREC_POL
    >
    small_vector<std::tuple<T, T, bool>, N>
    build_partition(const S &s, const DS &ds, const DDS &dds,
        const PREC_POL& prec_pol, small_vector<T, N> &ts) {
        small_vector<std::tuple<T, T, bool>, N> us;
        auto prev = std::make_pair(T{0}, T{0.f});
        for (const auto &curr: make_adjacent_range(ts)) {
//std::cerr << "[" << curr.first << ',' << curr.second << "] input\n";
            // If we don't yet have two consecutive non-empty intervals
            if (util::is_almost_equal(prev.first, prev.second) ||
                util::is_almost_equal(curr.first, curr.second)) {
                // Extend previous to include current
                prev.second = curr.second;
            // If we do have two consecutive non-empty intervals
            } else {
                // Classify both intervals
                bool prev_regular = is_regular(s, ds, dds, prec_pol,
                    prev.first, prev.second);
                bool curr_regular = is_regular(s, ds, dds, prec_pol,
                    curr.first, curr.second);
                // If interval regularities are different, output prev
                if (curr_regular != prev_regular) {
                    us.push_back(std::make_tuple(prev.first, prev.second,
                            prev_regular));
                    prev = curr;
                    prev_regular = curr_regular;
                // Otherwise, extend previous to include current
                } else {
                    prev.second = curr.second;
                }
            }
        }
        // Output final interval
        bool prev_regular = is_regular(s, ds, dds, prec_pol,
            prev.first, prev.second);
        us.push_back(std::make_tuple(prev.first, prev.second, prev_regular));
        return us;
    }

    template <typename S, typename DS, typename DDS>
    void inner_irregular_piece(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds) {
        auto qi = bezier_evaluate_horner(s, ti);
        auto di = tangent_direction(qi, bezier_evaluate_horner(ds, ti),
            bezier_evaluate_horner(dds, ti));
        auto pi = project<R2>(qi);
        auto qf = bezier_evaluate_horner(s, tf);
        auto df = tangent_direction(qf, bezier_evaluate_horner(ds, tf),
            bezier_evaluate_horner(dds, tf));
        auto pf = project<R2>(qf);
        // We leave the decision of inner/outer/omit to the
        // simplify joins filter
        m_sink.cusp(di, pi, df, 1);
        m_sink.degenerate_segment(pi, df, pf);
    }

    template <typename S, typename DS, typename DDS>
    void entirely_irregular_segment(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds) {
        (void) ds; (void) dds;
        assert(ti == rvgf{0} && tf == rvgf{1}); (void) ti; (void) tf;
        auto pi = project<R2>(bezier_evaluate_horner(s, rvgf{0}));
        auto pf = project<R2>(bezier_evaluate_horner(s, rvgf{1}));
        const R2 z{0,0};
        ensure_begun(pi, z, z, pf);
        m_sink.degenerate_segment(pi, z, pf);
    }

    template <typename S, typename DS, typename DDS, typename PREC_POL>
    void first_irregular_piece(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds, const PREC_POL &prec_pol) {
		assert(ti == rvgf{0}); (void)ti;
        auto qi = bezier_evaluate_horner<rvgf>(s, rvgf{0});
        auto pi = project<R2>(qi);
        auto qf = bezier_evaluate_horner(s, tf);
        auto pf = project<R2>(qf);
        auto df = tangent_direction(qf, bezier_evaluate_horner<rvgf>(ds, tf),
            bezier_evaluate_horner(dds, tf));
        bool precise_ds = bezier_evaluate_horner<rvgf>(prec_pol, rvgf{0}) > 0;
        if (precise_ds) {
            auto di = tangent_direction(qi, bezier_evaluate_horner(ds, rvgf{0}),
                bezier_evaluate_horner(dds, rvgf{0}));
            ensure_begun(pi, di, df, pf);
            m_sink.degenerate_segment(pi, di, pf);
            m_sink.cusp(di, pf, df, 0);
        } else {
            ensure_begun(pi, df, df, pf);
            m_sink.degenerate_segment(pi, df, pf);
        }
    }

    template <typename S, typename DS, typename DDS, typename PREC_POL>
    void last_irregular_piece(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds, const PREC_POL &prec_pol) {
		assert(tf == rvgf{1}); (void) tf;
        auto qf = bezier_evaluate_horner<rvgf>(s, rvgf{1});
        auto pf = project<R2>(qf);
        auto qi = bezier_evaluate_horner(s, ti);
        auto pi = project<R2>(qi);
        auto di = tangent_direction(qi, bezier_evaluate_horner(ds, ti),
            bezier_evaluate_horner(dds, ti));
        bool precise_ds = bezier_evaluate_horner<rvgf>(prec_pol, rvgf{1}) > 0;
        if (precise_ds) {
            auto df = tangent_direction(qf, bezier_evaluate_horner(ds, rvgf{1}),
                bezier_evaluate_horner(dds, rvgf{1}));
            ensure_begun(pi, di, df, pf);
            m_sink.cusp(di, pi, df, 0);
            m_sink.degenerate_segment(pi, df, pf);
        } else {
            ensure_begun(pi, di, di, pf);
            m_sink.degenerate_segment(pi, di, pf);
        }
    }

    template <typename S, typename DS, typename DDS, typename PIECE>
    void regular_piece(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds, const PIECE &segment_piece) {
        auto qi = bezier_evaluate_horner(s, ti);
        auto di = tangent_direction(qi, bezier_evaluate_horner(ds, ti),
            bezier_evaluate_horner(dds, ti));
        auto pi = project<R2>(qi);
        auto qf = bezier_evaluate_horner(s, tf);
        auto df = tangent_direction(qf, bezier_evaluate_horner(ds, tf),
            bezier_evaluate_horner(dds, tf));
        auto pf = project<R2>(qf);
        ensure_begun(pi, di, pf, df);
        m_sink.begin_segment_piece(pi, di);
        this->forward_parameters(ti, tf);
        segment_piece(ti, tf);
        m_sink.end_segment_piece(df, pf);
    }

    template <
        typename S,
        typename DS,
        typename DDS,
        typename PREC_POL,
        typename PIECE
    >
    void process_segment(
        const S &s,
        const DS &ds,
        const DDS &dds,
        const PREC_POL &prec_pol,
        const PIECE &segment_piece
    ) {
        std::sort(m_radius_crossings.begin(), m_radius_crossings.end());
        auto prec_pol_roots = bezier_roots<rvgf>(prec_pol);
        m_radius_crossings.reserve(m_radius_crossings.size() +
            prec_pol_roots.size());
        auto middle = m_radius_crossings.end();
        std::copy(prec_pol_roots.begin(), prec_pol_roots.end(),
            std::back_inserter(m_radius_crossings));
        std::inplace_merge(m_radius_crossings.begin(), middle,
            m_radius_crossings.end());
        auto us = build_partition<rvgf>(s, ds, dds, prec_pol,
            m_radius_crossings);
        int c = 0;
        int n = static_cast<int>(us.size());
        // Go over each partition of [0,1]
        for (const auto &u: us) {
            auto ti = std::get<0>(u);
            auto tf = std::get<1>(u);
            bool regular = std::get<2>(u);
            // Interval is irregular
            if (!regular) {
                // Inner interval is irregular
                // Output cusp followed by degenerate segment
                if (c != 0 && c != (n-1)) {
                    inner_irregular_piece(ti, tf, s, ds, dds);
                // Entire interval is irregular
                // Output single degenerate segment
                } else if (n == 1) {
                    entirely_irregular_segment(ti, tf, s, ds, dds);
                // First piece is irregular
                // Output degenerate segment
                } else if (c == 0) {
                    first_irregular_piece(ti, tf, s, ds, dds, prec_pol);
                // Last piece is irregular
                // Output degenerate segment
                } else if (c == n-1) {
                    last_irregular_piece(ti, tf, s, ds, dds, prec_pol);
                }
            // Interval is regular
            } else {
                // Output segment piece itself
                regular_piece(ti, tf, s, ds, dds, segment_piece);
            }
            c++;
        }
        m_radius_crossings.clear();
        this->clear_parameters();
    }

    template <typename DS>
    auto precision_polynomial(const DS &ds) {
        auto ads = tuple_map(ds, [](const R2 &q) -> R2 { return fabs(q); } );
        auto ds2 = bezier_product<rvgf>(ds, ds);
        auto ads2 = bezier_product<rvgf>(ads, ads);
        rvgf gamma = m_gamma;
        auto d = tuple_map(ds2, [gamma](const R2 &q) -> rvgf {
            return gamma*(q[0]+q[1]);
        });
        rvgf delta2 = m_delta*m_delta;
        auto r = tuple_map(ads2, [delta2](const R2 &q) -> rvgf {
            return delta2*(q[0]+q[1]);
        });
        constexpr size_t degree_r = std::tuple_size<decltype(r)>::value-1;
        std::get<0>(r) = std::max(std::get<0>(r), m_delta);
        std::get<degree_r>(r) = std::max(std::get<degree_r>(r), m_delta);
        return tuple_zip_map(d, r, std::minus<rvgf>{});
    }

    void do_quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        process_segment(s, ds, dds, precision_polynomial(ds),
            [&](rvgf ti, rvgf tf) {
                m_sink.quadratic_segment_piece(ti, tf, p0, p1, p2);
            }
        );
    }

    void do_rational_quadratic_segment(const R3 &p0, const R3 &p1,
        const R3 &p2) {
        auto w = std::tie(p0[2], p1[2], p2[2]);
        auto dw = bezier_derivative(w);
        auto u = std::tie(p0[0], p1[0], p2[0]);
        auto du = bezier_derivative(u);
        auto dx = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, du),
                bezier_product<rvgf>(u, dw),
                std::minus<rvgf>{}
            )
        );
        auto v = std::tie(p0[1], p1[1], p2[1]);
        auto dv = bezier_derivative(v);
        auto dy = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<rvgf>(w, dv),
                bezier_product<rvgf>(v, dw),
                std::minus<rvgf>{}
            )
        );
        auto ds = tuple_zip_map(dx, dy,
            [](rvgf x, rvgf y) -> R2 { return R2{x, y}; }
        );
        auto sr = std::tie(p0, p1, p2);
        auto dsr = bezier_derivative(sr);
        auto ddsr = bezier_derivative(dsr);
        process_segment(sr, dsr, ddsr, precision_polynomial(ds),
            [&](rvgf ti, rvgf tf) {
                m_sink.rational_quadratic_segment_piece(ti, tf, p0, p1, p2);
            }
        );
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        auto s = std::tie(p0, p1, p2, p3);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        process_segment(s, ds, dds, precision_polynomial(ds),
            [&](rvgf ti, rvgf tf) {
                m_sink.cubic_segment_piece(ti, tf, p0, p1, p2, p3);
            }
        );
    }


friend i_offsetting_parameters<input_path_f_to_regular_path<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        m_radius_crossings.push_back(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        (void) t;
    }

};

template <typename SINK>
inline auto make_input_path_f_to_regular_path(rvgf min_radius, rvgf alpha, rvgf delta,
    SINK &&sink) {
    return input_path_f_to_regular_path<SINK>{min_radius, alpha, delta,
        std::forward<SINK>(sink)};
}

template <typename SINK>
inline auto make_input_path_f_to_regular_path(SINK &&sink) {
    return input_path_f_to_regular_path<SINK>{
        RVG_REGULARITY_MIN_RADIUS,
        RVG_REGULARITY_ANGULAR_TOLERANCE,
        RVG_REGULARITY_NUMERICAL_TOLERANCE*
            std::numeric_limits<rvgf>::epsilon(),
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
