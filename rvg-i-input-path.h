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
#ifndef RVG_I_INPUT_PATH_H
#define RVG_I_INPUT_PATH_H

#include "rvg-meta.h"
#include "rvg-floatint.h"
#include "rvg-point.h"
#include "rvg-canonize-rational-quadratic-bezier.h"

namespace rvg {

template <typename DERIVED>
class i_input_path {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    void begin_contour(rvgf x0, rvgf y0) {
        return derived().do_begin_contour(x0, y0);
    }

    void end_open_contour(rvgf x0, rvgf y0) {
        return derived().do_end_open_contour(x0, y0);
    }

    void end_closed_contour(rvgf x0, rvgf y0) {
        return derived().do_end_closed_contour(x0, y0);
    }

    void linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        return derived().do_linear_segment(x0, y0, x1, y1);
    }

    void quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        return derived().do_quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rational_quadratic_segment(x0, y0, x1, y1,
            w1, x2, y2);
    }

    void cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

// If any method is invoked with points instead of scalars, we delegate
// the calls to the corresponding scalar method

    void begin_contour(const R2 &p0) {
        return begin_contour(p0[0], p0[1]);
    }

    void end_open_contour(const R2 &p0) {
        return end_open_contour(p0[0], p0[1]);
    }

    void end_closed_contour(const R2 &p0) {
        return end_closed_contour(p0[0], p0[1]);
    }

    void linear_segment(const R2 &p0, const R2 &p1) {
        return linear_segment(p0[0], p0[1], p1[0], p1[1]);
    }

    void quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        return quadratic_segment(p0[0], p0[1], p1[0], p1[1],
            p2[0], p2[1]);
    }

    void rational_quadratic_segment(const R3 &p0, const R3 &p1,
        const R3 &p2) {
        R2 q0, q2;
        R3 q1;
        std::tie(q0, q1, q2) = canonize_rational_quadratic_bezier(p0, p1, p2);
        return rational_quadratic_segment(q0[0], q0[1],
            q1[0], q1[1], q1[2], q2[0], q2[1]);
    }

    void cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2,
        const R2 &p3) {
        return cubic_segment(p0[0], p0[1], p1[0], p1[1], p2[0],
            p2[1], p3[0], p3[1]);
    }
};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_input_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_input_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

namespace detail {
	class not_an_i_input_path { };
}

template <typename U, typename T>
using inherit_if_i_input_path =
	typename std::conditional<
		is_an_i_input_path<U>::value,
		T,
		detail::not_an_i_input_path
	>::type;

} } // namespace rvg::meta

#endif
