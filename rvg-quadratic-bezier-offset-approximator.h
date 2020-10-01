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
#ifndef RVG_QUADRATIC_BEZIER_OFFSET_APPROXIMATOR_H
#define RVG_QUADRATIC_BEZIER_OFFSET_APPROXIMATOR_H

#include "rvg-i-sink.h"
#include "rvg-i-quadratic-bezier-approximator.h"
#include "rvg-bezier.h"

namespace rvg {

template <
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename SINK
>
class quadratic_bezier_offset_approximator:
    public i_quadratic_bezier_approximator<
        quadratic_bezier_offset_approximator<
            BEZIER_TUPLE,
            DBEZIER_TUPLE,
            DDBEZIER_TUPLE,
            SINK
        >
	>,
    public i_sink<
       quadratic_bezier_offset_approximator<
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

public:

    quadratic_bezier_offset_approximator(rvgf offset, BEZIER_TUPLE &&s,
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

friend i_sink<
	quadratic_bezier_offset_approximator<
		BEZIER_TUPLE,
		DBEZIER_TUPLE,
		DDBEZIER_TUPLE,
		SINK
	>
>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_quadratic_bezier_approximator<
	quadratic_bezier_offset_approximator<
		BEZIER_TUPLE,
		DBEZIER_TUPLE,
		DDBEZIER_TUPLE,
		SINK
	>
>;

	// Compute exact offset at given parameter
    std::pair<R2, R2> do_sample_and_tangent_direction(rvgf t) {
		auto st = bezier_evaluate_horner(m_s, t);
		auto dt = tangent_direction(st, bezier_evaluate_horner(m_ds, t),
			bezier_evaluate_horner(m_dds, t));
		auto nt = (m_offset/len(dt))*perp(dt);
		return std::make_pair(project<R2>(st) + nt, dt);
    }

    R2 do_sample(rvgf t) {
		auto st = bezier_evaluate_horner(m_s, t);
		auto dt = tangent_direction(st, bezier_evaluate_horner(m_ds, t),
			bezier_evaluate_horner(m_dds, t));
		auto nt = (m_offset/len(dt))*perp(dt);
		return project<R2>(st) + nt;
    }

    R2 do_get_tangent_direction(rvgf t) {
		auto st = bezier_evaluate_horner(m_s, t);
		return tangent_direction(st, bezier_evaluate_horner(m_ds, t),
			bezier_evaluate_horner(m_dds, t));
    }
};

template <
    typename BEZIER_TUPLE,
    typename DBEZIER_TUPLE,
    typename DDBEZIER_TUPLE,
    typename SINK
>
auto make_quadratic_bezier_offset_approximator(rvgf offset,
    BEZIER_TUPLE &&s,
    DBEZIER_TUPLE &&ds,
    DDBEZIER_TUPLE &&dds,
    SINK &&sink) {
    return quadratic_bezier_offset_approximator<
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
