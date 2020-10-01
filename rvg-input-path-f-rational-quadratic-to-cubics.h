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
#ifndef RVG_INPUT_PATH_F_RATIONAL_QUADRATIC_TO_CUBICS_H
#define RVG_INPUT_PATH_F_RATIONAL_QUADRATIC_TO_CUBICS_H

#include <string>
#include <iosfwd>
#include <type_traits>
#include <utility>

#include "rvg-point.h"
#include "rvg-i-sink.h"
#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-bezier.h"
#include "rvg-bezier-arc-length.h"
#include "rvg-cubic-bezier-bezier-approximator.h"

namespace rvg {

// Eliminate rational quadratic segments by approximating them with cubics
template <typename SINK>
class input_path_f_rational_quadratic_to_cubics final:
    public i_sink<input_path_f_rational_quadratic_to_cubics<SINK>>,
    public i_input_path_f_forwarder<input_path_f_rational_quadratic_to_cubics<SINK>> {

    rvgf m_tol;
    SINK m_sink;

public:

    explicit input_path_f_rational_quadratic_to_cubics(rvgf tol, SINK &&sink):
        m_tol(tol),
        m_sink(std::forward<SINK>(sink)) {
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<input_path_f_rational_quadratic_to_cubics<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_input_path<input_path_f_rational_quadratic_to_cubics<SINK>>;

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
		R3 p0{x0,y0,1.f}, p1{x1,y1,w1}, p2{x2,y2,1.f};
		auto s = std::tie(p0, p1, p2);
		auto ds = bezier_derivative(s);
		auto dds = bezier_derivative(ds);
        auto ds2 = make_rational_quadratic_segment_ds2_from_tuples<rvgf>(
            s, ds);
        auto rational = make_cubic_bezier_bezier_approximator<
            RVG_CUBIC_BEZIER_APPROXIMATION_SAMPLES>(s, ds, dds, ds2, m_sink);
        rational.approximate(0.f, R2{x1-x0*w1,y1-y0*w1}, 1.f,
            R2{x2*w1-x1, y2*w1-y1}, m_tol);
    }

};

template <typename SINK>
inline auto
make_input_path_f_rational_quadratic_to_cubics(SINK &&sink,
    rvgf tol = RVG_CUBIC_BEZIER_APPROXIMATION_TOLERANCE) {
    return input_path_f_rational_quadratic_to_cubics<SINK>{tol,
        std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
