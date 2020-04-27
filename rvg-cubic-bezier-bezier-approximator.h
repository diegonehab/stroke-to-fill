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
#ifndef RVG_CUBIC_BEZIER_BEZIER_APPROXIMATOR_H
#define RVG_CUBIC_BEZIER_BEZIER_APPROXIMATOR_H

#include "rvg-i-cubic-bezier-approximator.h"
#include "rvg-point.h"
#include "rvg-bezier.h"

namespace rvg {

template <
    size_t N,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename DS2,
    typename SINK
>
class cubic_bezier_bezier_approximator:
    public i_cubic_bezier_approximator<
        N,
        cubic_bezier_bezier_approximator<
            N,
            BEZIER_TUPLE,
            DBEZIER_TUPLE,
            DDBEZIER_TUPLE,
            DS2,
            SINK
        >
    > {

    BEZIER_TUPLE m_s;
    DBEZIER_TUPLE m_ds;
    DDBEZIER_TUPLE m_dds;
    DS2 m_ds2;
    SINK m_sink;

    using base = i_cubic_bezier_approximator<
        N,
        cubic_bezier_bezier_approximator<
            N,
            BEZIER_TUPLE,
            DBEZIER_TUPLE,
            DDBEZIER_TUPLE,
            DS2,
            SINK
        >
    >;

public:

    cubic_bezier_bezier_approximator(BEZIER_TUPLE &&s,
        DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds, DS2 &&ds2, SINK &&sink):
        m_s(std::forward<BEZIER_TUPLE>(s)),
        m_ds(std::forward<DBEZIER_TUPLE>(ds)),
        m_dds(std::forward<DDBEZIER_TUPLE>(dds)),
        m_ds2(std::forward<DS2>(ds2)),
        m_sink(std::forward<SINK>(sink)) { }

private:

    friend base;

    rvgf do_sample(rvgf ta, const R2 &da, rvgf tb, const R2 &db,
        std::array<rvgf, N+1> &us, std::array<R2, N+1> &qs) {
        (void) da; (void) db;
        auto sa = bezier_evaluate_horner(m_s, ta);
        qs[0] = project<R2>(sa);
        us[0] = rvgf{0};
        auto el = make_arc_length<rvgf,RVG_ARC_LENGTH_INTERVALS>(
            ta, tb, m_ds2);
        const rvgf du = rvgf{1}/N;
        const rvgf dt = tb-ta;
        for (unsigned i = 1; i < N; i++) {
			rvgf ti = ta + el.get_relative_parameter_for_length_fraction(
                i*du)*dt;
            qs[i] = project<R2>(bezier_evaluate_horner(m_s, ti));
            us[i] = i*du;
        }
        auto sb = bezier_evaluate_horner(m_s, tb);
        qs[N] = project<R2>(sb);
        us[N] = rvgf{1};
        return rvgf{0.5}*(ta+tb);
    }

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        m_sink.cubic_segment(p0, p1, p2, p3);
    }

    void do_linear_segment(const R2 &p0, const R2 &p1) {
        m_sink.linear_segment(p0, p1);
    }

    R2 do_get_tangent_direction(rvgf t) {
        // ??D maybe create a function that evaluates both the
        // curve and its derivative? It would be faster.
        auto st = bezier_evaluate_horner(m_s, t);
        auto dst = bezier_evaluate_horner(m_ds, t);
        auto ddst = bezier_evaluate_horner(m_dds, t);
        return rvg::tangent_direction(st, dst, ddst);
    }

};

template <
    size_t N,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename DS2,
    typename SINK
>
auto make_cubic_bezier_bezier_approximator(
    BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds,
    DS2 &&ds2,
    SINK &&sink
) {
    return cubic_bezier_bezier_approximator<
        N,
        BEZIER_TUPLE,
        DBEZIER_TUPLE,
        DDBEZIER_TUPLE,
        DS2,
        SINK
    >{
        std::forward<BEZIER_TUPLE>(s),
        std::forward<DBEZIER_TUPLE>(ds),
        std::forward<DDBEZIER_TUPLE>(dds),
        std::forward<DS2>(ds2),
        std::forward<SINK>(sink)
    };
}

} // namespace rvg

#endif
