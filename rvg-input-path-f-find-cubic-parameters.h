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
#ifndef RVG_INPUT_PATH_F_FIND_CUBIC_PARAMETER_H
#define RVG_INPUT_PATH_F_FIND_CUBIC_PARAMETER_H

#include "rvg-bezier.h"
#include "rvg-util.h"
#include "rvg-i-point-input-path-f-forwarder.h"
#include "rvg-i-cubic-parameters.h"
#include "rvg-i-parameters-f-forwarder.h"

namespace rvg {

template <typename SINK>
class input_path_f_find_cubic_parameters final:
    public i_sink<input_path_f_find_cubic_parameters<SINK>>,
    public i_point_input_path_f_forwarder<
        input_path_f_find_cubic_parameters<SINK>>,
    public i_parameters_f_forwarder<SINK,
        input_path_f_find_cubic_parameters<SINK>> {

    SINK m_sink;

public:

    explicit input_path_f_find_cubic_parameters(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_i_cubic_parameters<SINK>::value,
            "sink is not an i_cubic_parameters");
        static_assert(meta::is_an_i_input_path<SINK>::value,
            "sink is not an i_input_path");
    }

private:

friend i_sink<input_path_f_find_cubic_parameters<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_input_path<input_path_f_find_cubic_parameters<SINK>>;

    void do_cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        using util::det;
        using util::is_almost_equal;
        // Shift points so p0 is at the origin
        R2 u0 = R2{0,0};
        R2 u1 = p1-p0;
        R2 u2 = p2-p0;
        R2 u3 = p3-p0;
		// Compute det of minors and from them the
        // discriminant of the inflection polynomial
		rvgf b3 = 3*det(u1, u2, u3);
		rvgf b2 = det(u0, u2, u3);
		rvgf b1 = det(u0, u1, u3);
		rvgf b0 = 3*det(u0, u1, u2);
        rvgf d1 = 2*det(b1, b0, b2, b1);
        rvgf d2 = det(b1, b0, b3, b2);
        rvgf d3 = 2*det(b2, b1, b3, b2);
        rvgf d1d3 = d1*d3;
        rvgf d2d2 = d2*d2;
        bool almost_zero = is_almost_equal(d1d3, d2d2);
        rvgf d = d1d3-d2d2; // Finally, the discriminant
        // If the discriminant is positive, we have a serpentine
        // We simply find the parameters for the inflections
        if (d > 0 || almost_zero) {
            // We know the cubic inflection polynomial given by
            // std::make_tuple(b0, b1, b2, b3) really is a
            // quadratic because the curve is integral.
            // So we lower the degree before looking for roots
            auto infls = std::make_tuple(b0, rvgf{.25}*(3*(b1+b2)-(b0+b3)), b3);
            auto ts = bezier_roots<rvgf>(infls);
            if (ts.size() > 2) {
                for (unsigned i = 1; i < ts.size()-1; i++) {
                    m_sink.inflection_parameter(ts[i]);
                }
            }
        }
        // If the discriminant is negative, we have a loop
        // We simply find the parameters for the double-point
        if (d < 0 || almost_zero) {
            auto doubpts = std::make_tuple(d1, d2, d3);
            auto ts = bezier_roots<rvgf>(doubpts);
            if (ts.size() > 2) {
                for (unsigned i = 1; i < ts.size()-1; i++) {
                    m_sink.double_point_parameter(ts[i]);
                }
            }
        }
        m_sink.cubic_segment(p0, p1, p2, p3);
    }

};

template <typename SINK>
inline auto make_input_path_f_find_cubic_parameters(SINK &&sink) {
    return input_path_f_find_cubic_parameters<SINK>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
