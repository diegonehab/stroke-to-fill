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
#ifndef RVG_I_REGULAR_PATH_H
#define RVG_I_REGULAR_PATH_H

#include "rvg-floatint.h"
#include "rvg-meta.h"
#include "rvg-point.h"
#include "rvg-canonize-rational-quadratic-bezier.h"

namespace rvg {

template <typename DERIVED>
class i_regular_path {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    void begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().do_begin_regular_contour(xi, yi, dxi, dyi);
    }

    void end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().do_end_regular_open_contour(dxf, dyf, xf, yf);
    }

    void end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().do_end_regular_closed_contour(dxf, dyf, xf, yf);
    }

    void degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
        rvgf xf, rvgf yf) {
        return derived().do_degenerate_segment(xi, yi, dx, dy, xf, yf);
    }

    void cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf, rvgf w) {
        return derived().do_cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return derived().do_inner_cusp(dxi, dyi, x, y, dxf, dyf, w);
    }

    void begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().do_begin_segment_piece(xi, yi, dxi, dyi);
    }

    void end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().do_end_segment_piece(dxf, dyf, xf, yf);
    }

    void linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1) {
        return derived().do_linear_segment_piece(ti, tf, x0, y0, x1, y1);
    }

    void quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return derived().do_quadratic_segment_piece(ti, tf, x0, y0, x1, y1,
            x2, y2);
    }

    void rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rational_quadratic_segment_piece(ti, tf, x0, y0,
            x1, y1, w1, x2, y2);
    }

    void cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_cubic_segment_piece(ti, tf, x0, y0,
            x1, y1, x2, y2, x3, y3);
    }

    void begin_regular_contour(const R2 &pi, const R2 &di) {
        return begin_regular_contour(pi[0], pi[1], di[0], di[1]);
    }

    void end_regular_open_contour(const R2 &df, const R2 &pf) {
        return end_regular_open_contour(df[0], df[1], pf[0], pf[1]);
    }

    void end_regular_closed_contour(const R2 &df, const R2 &pf) {
        return end_regular_closed_contour(df[0], df[1], pf[0], pf[1]);
    }

    void degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        return degenerate_segment(pi[0], pi[1], d[0], d[1],
            pf[0], pf[1]);
    }

    void cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return cusp(di[0], di[1], p[0], p[1], df[0], df[1], w);
    }

    void inner_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return inner_cusp(di[0], di[1], p[0], p[1], df[0], df[1], w);
    }

    void begin_segment_piece(const R2 &pi, const R2 &di) {
        return begin_segment_piece(pi[0], pi[1], di[0], di[1]);
    }

    void end_segment_piece(const R2 &di, const R2 &pi) {
        return end_segment_piece(di[0], di[1], pi[0], pi[1]);
    }

    void linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        return linear_segment_piece(ti, tf, p0[0], p0[1], p1[0], p1[1]);
    }

    void quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2) {
        return quadratic_segment_piece(ti, tf, p0[0], p0[1], p1[0], p1[1],
            p2[0], p2[1]);
    }

    void rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        R2 q0, q2;
        R3 q1;
        std::tie(q0, q1, q2) = canonize_rational_quadratic_bezier(p0, p1, p2);
        return rational_quadratic_segment_piece(ti, tf, q0[0], q0[1],
            q1[0], q1[1], q1[2], q2[0], q2[1]);
    }

    void cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        return cubic_segment_piece(ti, tf, p0[0], p0[1], p1[0], p1[1],
            p2[0], p2[1], p3[0], p3[1]);
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_regular_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_regular_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

namespace detail {
	class not_an_i_regular_path { };
}

template <typename U, typename T>
using inherit_if_i_regular_path =
	typename std::conditional<
		is_an_i_regular_path<U>::value,
		T,
		detail::not_an_i_regular_path
	>::type;

} } // namespace rvg::meta

#endif
