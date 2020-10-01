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
#ifndef RVG_CUBIC_BEZIER_EVOLUTE_APPROXIMATOR_H
#define RVG_CUBIC_BEZIER_EVOLUTE_APPROXIMATOR_H

#include "rvg-i-cubic-bezier-approximator.h"
#include "rvg-bezier.h"

namespace rvg {

template <
    size_t N,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename SINK
>
class cubic_bezier_evolute_approximator: public i_cubic_bezier_approximator<
    N,
    cubic_bezier_evolute_approximator<
        N,
        BEZIER_TUPLE,
        DBEZIER_TUPLE,
        DDBEZIER_TUPLE,
        SINK
    >
> {

	rvgf m_max_radius;
    BEZIER_TUPLE m_s;
    DBEZIER_TUPLE m_ds;
    DDBEZIER_TUPLE m_dds;
    SINK m_sink;

    using base = i_cubic_bezier_approximator<
        N,
        cubic_bezier_evolute_approximator<
            N,
            BEZIER_TUPLE,
            DBEZIER_TUPLE,
            DDBEZIER_TUPLE,
            SINK
        >
    >;

public:

    cubic_bezier_evolute_approximator(rvgf max_radius, BEZIER_TUPLE &&s,
        DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds, SINK &sink):
		m_max_radius{max_radius},
        m_s{std::forward<BEZIER_TUPLE>(s)},
        m_ds{std::forward<DBEZIER_TUPLE>(ds)},
        m_dds{std::forward<DDBEZIER_TUPLE>(dds)},
        m_sink{std::forward<SINK>(sink)} { }

private:

    friend base;

    bool do_sample(rvgf ta, const R2 &da, rvgf tb,
        const R2 &db, std::array<rvgf, N+1> &us, std::array<R2, N+1> &qs) {
        (void) da; (void) db;
        auto s_a = bezier_evaluate_horner(m_s, ta);
        auto ds_a = bezier_evaluate_horner(m_ds, ta);
        auto dds_a = bezier_evaluate_horner(m_dds, ta);
        qs[0] = center_of_curvature(m_max_radius, s_a, ds_a, dds_a);
        us[0] = rvgf{0};
        const rvgf du = rvgf{1}/N;
        const rvgf dt = tb-ta;
        for (unsigned i = 1; i < N; i++) {
			rvgf ui = i*du;
			rvgf ti = ta + i*dt/N;
            // ??D maybe create a function that evaluates both the
            // curve and its derivative? It would be faster.
            auto s_i = bezier_evaluate_horner(m_s, ti);
            auto ds_i = bezier_evaluate_horner(m_ds, ti);
            auto dds_i = bezier_evaluate_horner(m_dds, ti);
            qs[i] = center_of_curvature(m_max_radius, s_i, ds_i, dds_i);
            us[i] = ui;
        }
        auto s_b = bezier_evaluate_horner(m_s, tb);
        auto ds_b = bezier_evaluate_horner(m_ds, tb);
        auto dds_b = bezier_evaluate_horner(m_dds, tb);
        qs[N] = center_of_curvature(m_max_radius, s_b, ds_b, dds_b);
        us[N] = rvgf{1};
        // ??D we should check if the angle between samples is too
        // large and return false to force a subdivision. This is more
        // of a problem when we are dealing with offsets (where we do
        // perform this the test), but there is probably a contrived
        // example where this could mess up the evolute as well
        return true;
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        m_sink.cubic_segment(p0, p1, p2, p3);
    }

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        m_sink.linear_segment(p0, p1);
    }

    R2 do_get_tangent_direction(rvgf t) {
        auto st = bezier_evaluate_horner(m_s, t);
        auto dst = bezier_evaluate_horner(m_ds, t);
        auto ddst = bezier_evaluate_horner(m_dds, t);
        return perp(tangent_direction(st, dst, ddst));
    }
};

template <
    size_t N,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename SINK>
auto make_cubic_bezier_evolute_approximator(
	rvgf max_radius,
    BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds,
    SINK &&sink
) {
    return cubic_bezier_evolute_approximator<
        N,
        BEZIER_TUPLE,
        DBEZIER_TUPLE,
        DDBEZIER_TUPLE,
        SINK
    >{
		max_radius,
        std::forward<BEZIER_TUPLE>(s),
        std::forward<DBEZIER_TUPLE>(ds),
        std::forward<DDBEZIER_TUPLE>(dds),
        std::forward<SINK>(sink)
    };
}

} // namespace rvg

#endif
