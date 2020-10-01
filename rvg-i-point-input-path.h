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
#ifndef RVG_I_POINT_INPUT_PATH_H
#define RVG_I_POINT_INPUT_PATH_H

#include "rvg-meta.h"
#include "rvg-i-input-path.h"
#include "rvg-point.h"

namespace rvg {

template <typename DERIVED>
class i_point_input_path:
    public i_input_path<i_point_input_path<DERIVED>> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

    using base = i_input_path<i_point_input_path<DERIVED>>;

public:

    void begin_contour(const R2 &p0) {
        return derived().do_begin_contour(p0);
    }

    void end_open_contour(const R2 &p0) {
        return derived().do_end_open_contour(p0);
    }

    void end_closed_contour(const R2 &p0) {
        return derived().do_end_closed_contour(p0);
    }

    void linear_segment(const R2 &p0, const R2 &p1) {
        return derived().do_linear_segment(p0, p1);
    }

    void quadratic_segment(const R2 &p0, const R2 &p1, const R2 &p2) {
        return derived().do_quadratic_segment(p0, p1, p2);
    }

    void rational_quadratic_segment(const R3 &p0, const R3 &p1, const R3 &p2) {
        return derived().do_rational_quadratic_segment(p0, p1, p2);
    }

    void cubic_segment(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        return derived().do_cubic_segment(p0, p1, p2, p3);
    }

    using base::begin_contour;
    using base::end_open_contour;
    using base::end_closed_contour;
    using base::linear_segment;
    using base::quadratic_segment;
    using base::rational_quadratic_segment;
    using base::cubic_segment;

protected:

friend base;

    void do_begin_contour(rvgf x0, rvgf y0) {
        return derived().do_begin_contour(make_R2(x0, y0));
    }

    void do_end_open_contour(rvgf x0, rvgf y0) {
        return derived().do_end_open_contour(make_R2(x0, y0));
    }

    void do_end_closed_contour(rvgf x0, rvgf y0) {
        return derived().do_end_closed_contour(make_R2(x0, y0));
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        return derived().do_linear_segment(make_R2(x0, y0), make_R2(x1, y1));
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        return derived().do_quadratic_segment(make_R2(x0, y0), make_R2(x1, y1),
            make_R2(x2, y2));
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rational_quadratic_segment(make_R3(x0, y0, 1),
            make_R3(x1, y1, w1), make_R3(x2, y2, 1));
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_cubic_segment(make_R2(x0, y0), make_R2(x1, y1),
            make_R2(x2, y2), make_R2(x3, y3));
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_point_input_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_point_input_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
