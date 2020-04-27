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
#ifndef RVG_CUBIC_BEZIER_OFFSET_APPROXIMATOR_H
#define RVG_CUBIC_BEZIER_OFFSET_APPROXIMATOR_H

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
class cubic_bezier_offset_approximator: public i_cubic_bezier_approximator<
    N,
    cubic_bezier_offset_approximator<
        N,
        BEZIER_TUPLE,
        DBEZIER_TUPLE,
        DDBEZIER_TUPLE,
        SINK
    >
> {

    rvgf m_offset;
    BEZIER_TUPLE m_s;
    DBEZIER_TUPLE m_ds;
    DDBEZIER_TUPLE m_dds;
    SINK m_sink;

    using base = i_cubic_bezier_approximator<
        N,
        cubic_bezier_offset_approximator<
            N,
            BEZIER_TUPLE,
            DBEZIER_TUPLE,
            DDBEZIER_TUPLE,
            SINK
        >
    >;

public:

    cubic_bezier_offset_approximator(rvgf offset, BEZIER_TUPLE &&s,
        DBEZIER_TUPLE &&ds, DDBEZIER_TUPLE &&dds, SINK &&sink):
        m_offset(offset),
        m_s(std::forward<BEZIER_TUPLE>(s)),
        m_ds(std::forward<DBEZIER_TUPLE>(ds)),
        m_dds(std::forward<DDBEZIER_TUPLE>(dds)),
        m_sink(std::forward<SINK>(sink)) {
            static_assert(meta::is_an_i_input_path<SINK>::value,
                "sink is not both i_input_path");
        }

private:

    friend base;

	// Compute exact offset at uniformly spaced parameters between ta and tb
    bool do_sample(rvgf ta, const R2 &da, rvgf tb,
        const R2 &db, std::array<rvgf, N+1> &us, std::array<R2, N+1> &qs){
		auto sa = bezier_evaluate_horner(m_s, ta);
		auto na = (m_offset/len(da))*perp(da);
		qs[0] = project<R2>(sa) + na;
        us[0] = rvgf{0};
        const rvgf du = rvgf{1}/N;
        const rvgf dt = tb-ta;
        R2 prev_ni = na;
        rvgf min_dot = m_offset*m_offset*cos(3.141592653589793/(2*N));
		for (unsigned i = 1; i < N; i++) {
            rvgf ui = i*du;
			rvgf ti = ta + i*dt/N;
            auto si = bezier_evaluate_horner(m_s, ti);
			auto di = tangent_direction(si, bezier_evaluate_horner(m_ds, ti),
                bezier_evaluate_horner(m_dds, ti));
			auto ni = (m_offset/len(di))*perp(di);
            if (dot(ni, prev_ni) < min_dot) return false;
			qs[i] = project<R2>(si) + ni;
            us[i] = ui;
            prev_ni = ni;
		}
		auto sb = bezier_evaluate_horner(m_s, tb);
		auto nb = (m_offset/len(db))*perp(db);
		qs[N] = project<R2>(sb) + nb;
        us[N] = rvgf{1};
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
        // ??D maybe create a function that evaluates both the
        // curve and its derivative? It would be faster.
        auto st = bezier_evaluate_horner(m_s, t);
        auto dst = bezier_evaluate_horner(m_ds, t);
        auto ddst = bezier_evaluate_horner(m_dds, t);
        return tangent_direction(st, dst, ddst);
    }
};

template <
    size_t N,
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename SINK
>
auto make_cubic_bezier_offset_approximator(rvgf offset,
    BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds,
    SINK &&sink) {
    return cubic_bezier_offset_approximator<
        N,
        BEZIER_TUPLE,
        DBEZIER_TUPLE,
        DDBEZIER_TUPLE,
        SINK
    >{
        offset, std::forward<BEZIER_TUPLE>(s),
        std::forward<DBEZIER_TUPLE>(ds),
        std::forward<DDBEZIER_TUPLE>(dds),
        std::forward<SINK>(sink)
    };
}

} // namespace rvg

#endif
