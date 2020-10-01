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
#ifndef RVG_DECORATED_PATH_F_THICKEN_H
#define RVG_DECORATED_PATH_F_THICKEN_H

#include <utility>     // std::forward
#include <algorithm>   // std::sort, std::remove_if
#include <boost/container/small_vector.hpp>

#include "rvg-i-decorated-path-f-thicken.h"
#include "rvg-util.h"

#ifdef RVG_THICKEN_WITH_CUBICS
#include "rvg-cubic-bezier-offset-approximator.h"
#include "rvg-cubic-bezier-evolute-approximator.h"
#else
#include "rvg-quadratic-bezier-offset-approximator.h"
#include "rvg-quadratic-bezier-evolute-approximator.h"
#endif
#include "rvg-i-offsetting-parameters.h"
#include "rvg-i-cubic-parameters.h"
#include "rvg-i-sink.h"

#define RVG_BEZIER_APPROXIMATION_TOLERANCE (1.e-2f)

namespace rvg {

template <typename SINK>
class decorated_path_f_thicken final:
    public i_decorated_path_f_thicken<decorated_path_f_thicken<SINK>>,
    public i_offsetting_parameters<decorated_path_f_thicken<SINK>>,
    public i_cubic_parameters<decorated_path_f_thicken<SINK>>,
    public i_sink<decorated_path_f_thicken<SINK>>
{
    rvgf m_offset;
    rvgf m_ftol;
	SINK m_sink;
    boost::container::small_vector<rvgf, 16> m_offset_cusps;
    boost::container::small_vector<rvgf, 16> m_offset_partition;
    boost::container::small_vector<rvgf, 16> m_evolute_partition;

    using thickening_base = i_decorated_path_f_thicken<
        decorated_path_f_thicken<SINK>>;

public:

    decorated_path_f_thicken(rvgf width,
        stroke_style::const_ptr style, rvgf ftol, SINK &&sink):
        thickening_base(width, style),
        m_offset(0.5f*width),
        m_ftol(ftol),
		m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<decorated_path_f_thicken<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_regular_path<decorated_path_f_thicken<SINK>>;

    void regular_linear_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        auto n = (m_offset/len(d))*perp(d);
        auto qi = pi + n;
        auto qf = pf + n;
        thickening_base::prepare_for_current_segment(pi, d);
        m_sink.linear_segment(qi, qf);
        thickening_base::prepare_for_next_segment(qf);
        m_offset_cusps.clear();
        m_offset_partition.clear();
        m_evolute_partition.clear();
    }

    void do_degenerate_segment(const R2 &p0, const R2 &d, const R2 &p1) {
        regular_linear_segment(p0, d, p1);
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        ti = std::max(ti, thickening_base::get_t0());
        tf = std::min(tf, thickening_base::get_t1());
        auto d = p1-p0;
        auto s = std::tie(p0, p1);
        auto pi = bezier_evaluate_horner(s, ti);
        auto pf = bezier_evaluate_horner(s, tf);
        regular_linear_segment(pi, d, pf);
    }

    template <
        typename S,
        typename DS,
        typename DDS
    >
    R2 segment_subpiece(const S &s, const DS &ds, const DDS &dds,
        rvgf ta, const R2 &da, rvgf tb, const R2 &db,
        bool needs_evolute) {
#ifdef RVG_THICKEN_WITH_CUBICS
        auto offset = make_cubic_bezier_offset_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_offset, s, ds, dds,
                m_sink);
#else
        auto offset = make_quadratic_bezier_offset_approximator(m_offset,
            s, ds, dds, m_sink);
#endif
        // Evolute mode: radius is smaller than offset
        if (needs_evolute) {
//std::cerr.precision(16);
//std::cerr << "evolute [" << ta << "," << tb << "]\n";
#ifdef RVG_THICKEN_WITH_CUBICS
            auto evolute = make_cubic_bezier_evolute_approximator<
                RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(m_offset, s, ds,
                    dds, m_sink);
#else
            auto evolute = make_quadratic_bezier_evolute_approximator(
                m_offset, s, ds, dds, m_sink);
#endif
            auto sa = bezier_evaluate_horner(s, ta);
            auto na = (m_offset/len(da))*perp(da);
            auto oa = project<R2>(sa) + na;
            auto ea = center_of_curvature(m_offset, sa,
                bezier_evaluate_horner(ds, ta),
                bezier_evaluate_horner(dds, ta));
            auto sb = bezier_evaluate_horner(s, tb);
            auto nb = (m_offset/len(db))*perp(db);
            auto ob = project<R2>(sb) + nb;
            auto eb = center_of_curvature(m_offset, sb,
                bezier_evaluate_horner(ds, tb),
                bezier_evaluate_horner(dds, tb));
            m_sink.linear_segment(oa, ea);
            evolute.approximate_partition(ta, ea, perp(da), tb, eb, perp(db),
                m_evolute_partition, m_ftol);
            m_sink.linear_segment(eb, ob);
            offset.approximate_partition(tb, ob, db, ta, oa, da,
                m_offset_partition, m_ftol);
            m_sink.linear_segment(oa, ea);
            evolute.approximate_partition(ta, ea, perp(da), tb, eb, perp(db),
                m_evolute_partition, m_ftol);
            m_sink.linear_segment(eb, ob);
            return ob;
        // Straight-forward offset mode: radius is larger than offset
        } else {
//std::cerr.precision(16);
//std::cerr << "offset [" << ta << "," << tb << "]\n";
            auto sa = bezier_evaluate_horner(s, ta);
            auto na = (m_offset/len(da))*perp(da);
            auto oa = project<R2>(sa) + na;
            auto sb = bezier_evaluate_horner(s, tb);
            auto nb = (m_offset/len(db))*perp(db);
            auto ob = project<R2>(sb) + nb;
            return offset.approximate_partition(ta, oa, da, tb, ob, db,
                m_offset_partition, m_ftol);
        }
    }

    template <typename S, typename DS, typename DDS>
    static bool needs_evolute(const S &s, const DS &ds, const DDS &dds,
        rvgf t, rvgf offset) {
        auto st = bezier_evaluate_horner(s, t);
        auto dst = bezier_evaluate_horner(ds, t);
        auto ddst = bezier_evaluate_horner(dds, t);
        auto r = radius_of_curvature(st, dst, ddst);
        // Evolute mode: radius is smaller than offset
        return (r[1] > 0 && r[0] < r[1]*offset);
    }

    template <typename S, typename DS, typename DDS>
    static bool needs_evolute(const S &s, const DS &ds, const DDS &dds,
        rvgf ti, rvgf tf, rvgf offset) {
            return needs_evolute(s, ds, dds, 0.25*(3*ti+tf), offset) +
                   needs_evolute(s, ds, dds, 0.5*(ti+tf), offset) +
                   needs_evolute(s, ds, dds, 0.25*(ti+3*tf), offset) > 1;
    }

    template <
        typename S,
        typename DS,
        typename DDS
    >
    void segment_piece(rvgf ti, rvgf tf, const S &s, const DS &ds,
        const DDS &dds) {
        ti = std::max(ti, thickening_base::get_t0());
        tf = std::min(tf, thickening_base::get_t1());
        // remove inner offset cusps from traversal of outer offset
        m_offset_cusps.erase(std::remove_if(m_offset_cusps.begin(),
            m_offset_cusps.end(), [&](rvgf t) {
            return !positive_curvature(bezier_evaluate_horner(s, t),
                bezier_evaluate_horner<rvgf>(ds, t),
				bezier_evaluate_horner<rvgf>(dds, t));
        }), m_offset_cusps.end());
        m_offset_cusps.insert(m_offset_cusps.begin(), ti);
        m_offset_cusps.push_back(tf);
        std::sort(m_offset_cusps.begin(), m_offset_cusps.end());
        std::sort(m_offset_partition.begin(), m_offset_partition.end());
        std::sort(m_evolute_partition.begin(), m_evolute_partition.end());
        auto pi = bezier_evaluate_horner<rvgf>(s, ti);
        auto di = tangent_direction(pi,
            bezier_evaluate_horner<rvgf>(ds, ti),
            bezier_evaluate_horner<rvgf>(dds, ti));
        thickening_base::prepare_for_current_segment(project<R2>(pi), di);
        auto prev = std::make_pair(ti, ti);
        auto prev_first_d = di;
        for (const auto &curr: make_adjacent_range(m_offset_cusps)) {
            // If we don't yet have two consecutive non-empty intervals
            if (util::is_almost_equal(prev.first, prev.second) ||
                util::is_almost_equal(curr.first, curr.second)) {
                // Extend previous to include current
                prev.second = curr.second;
            // If we do have two consecutive non-empty intervals
            } else {
                // Classify both intervals
                bool prev_needs_evolute = needs_evolute(s, ds, dds,
                    prev.first, prev.second, m_offset);
                bool curr_needs_evolute = needs_evolute(s, ds, dds,
                    curr.first, curr.second, m_offset);
                // If intervals are of different types, output prev
                if (prev_needs_evolute != curr_needs_evolute) {
                    auto prev_second_d = tangent_direction(
                        bezier_evaluate_horner<rvgf>(s, prev.second),
                        bezier_evaluate_horner<rvgf>(ds, prev.second),
                        bezier_evaluate_horner<rvgf>(dds, prev.second));
                    segment_subpiece(s, ds, dds,
                        prev.first, prev_first_d, prev.second, prev_second_d,
                        prev_needs_evolute);
                    prev = curr;
                    prev_first_d = prev_second_d;
                    prev_needs_evolute = curr_needs_evolute;
                // Otherwise, extend previous to include current
                } else {
                    prev.second = curr.second;
                }
            }
        }
        // Output last interval
        bool prev_needs_evolute = needs_evolute(s, ds, dds, prev.first,
            prev.second, m_offset);
        auto pf = bezier_evaluate_horner<rvgf>(s, tf);
        auto df = tangent_direction(pf,
            bezier_evaluate_horner<rvgf>(ds, tf),
            bezier_evaluate_horner<rvgf>(dds, tf));
        auto p = segment_subpiece(s, ds, dds,
            prev.first, prev_first_d, tf, df, prev_needs_evolute);
        // potentially add an end contour
        thickening_base::prepare_for_next_segment(p);
        m_offset_cusps.clear();
        m_offset_partition.clear();
        m_evolute_partition.clear();
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        segment_piece(ti, tf, s, ds, dds);
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        auto s = std::tie(p0, p1, p2);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        segment_piece(ti, tf, s, ds, dds);
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        auto s = std::tie(p0, p1, p2, p3);
        auto ds = bezier_derivative(s);
        auto dds = bezier_derivative(ds);
        segment_piece(ti, tf, s, ds, dds);
    }

friend i_offsetting_parameters<decorated_path_f_thicken<SINK>>;

    void do_offset_cusp_parameter(rvgf t) {
        m_offset_cusps.push_back(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        m_evolute_partition.push_back(t);
    }

friend i_cubic_parameters<decorated_path_f_thicken<SINK>>;

    void do_double_point_parameter(rvgf t) {
        (void) t;
    }

    void do_inflection_parameter(rvgf t) {
        m_offset_partition.push_back(t);
    }

};

template <typename SINK>
static auto make_decorated_path_f_thicken(rvgf width,
    stroke_style::const_ptr style, rvgf ftol, SINK &&sink) {
    return decorated_path_f_thicken<SINK>(width, style, ftol,
        std::forward<SINK>(sink));
}

template <typename SINK>
static auto make_decorated_path_f_thicken(rvgf width,
    stroke_style::const_ptr style, SINK &&sink) {
    return make_decorated_path_f_thicken(width, style,
        RVG_BEZIER_APPROXIMATION_TOLERANCE, std::forward<SINK>(sink));
}


} // namespace rvg

#endif
