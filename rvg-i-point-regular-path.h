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
#ifndef RVG_I_POINT_REGULAR_PATH_H
#define RVG_I_POINT_REGULAR_PATH_H

#include "rvg-meta.h"
#include "rvg-i-regular-path.h"
#include "rvg-point.h"

namespace rvg {

template <typename DERIVED>
class i_point_regular_path:
    public i_regular_path<i_point_regular_path<DERIVED>> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

    using base = i_regular_path<i_point_regular_path<DERIVED>>;

public:

    void begin_regular_contour(const R2 &pi, const R2 &di) {
        return derived().do_begin_regular_contour(pi, di);
    }

    void end_regular_open_contour(const R2 &df, const R2 &pf) {
        return derived().do_end_regular_open_contour(df, pf);
    }

    void end_regular_closed_contour(const R2 &df, const R2 &pf) {
        return derived().do_end_regular_closed_contour(df, pf);
    }

    void degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        return derived().do_degenerate_segment(pi, d, pf);
    }

    void cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return derived().do_cusp(di, p, df, w);
    }

    void inner_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        return derived().do_inner_cusp(di, p, df, w);
    }

    void begin_segment_piece(const R2 &pi, const R2 &di) {
        return derived().do_begin_segment_piece(pi, di);
    }

    void end_segment_piece(const R2 &di, const R2 &pi) {
        return derived().do_end_segment_piece(di, pi);
    }

    void linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        return derived().do_linear_segment_piece(ti, tf, p0, p1);
    }

    void quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2) {
        return derived().do_quadratic_segment_piece(ti, tf, p0, p1, p2);
    }

    void rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        return derived().do_rational_quadratic_segment_piece(ti, tf,
            p0, p1, p2);
    }

    void cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        return derived().do_cubic_segment_piece(ti, tf, p0, p1, p2, p3);
    }

    using base::begin_regular_contour;
    using base::end_regular_open_contour;
    using base::end_regular_closed_contour;
    using base::degenerate_segment;
    using base::cusp;
    using base::inner_cusp;
    using base::begin_segment_piece;
    using base::end_segment_piece;
    using base::linear_segment_piece;
    using base::quadratic_segment_piece;
    using base::rational_quadratic_segment_piece;
    using base::cubic_segment_piece;

protected:

friend base;

    void do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().do_begin_regular_contour(make_R2(xi, yi),
            make_R2(dxi, dyi));
    }

    void do_end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
        return derived().do_end_regular_open_contour(make_R2(dxf, dyf),
            make_R2(xf, yf));
    }

    void do_end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf,
        rvgf yf) {
        return derived().do_end_regular_closed_contour(make_R2(dxf, dyf),
            make_R2(xf, yf));
    }

    void do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
        rvgf xf, rvgf yf) {
        return derived().do_degenerate_segment(make_R2(xi, yi), make_R2(dx, dy),
            make_R2(xf, yf));
    }

    void do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w) {
        return derived().do_cusp(make_R2(dxi, dyi), make_R2(x, y),
            make_R2(dxf, dyf), w);
    }

    void do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf,
        rvgf dyf, rvgf w) {
        return derived().do_inner_cusp(make_R2(dxi, dyi), make_R2(x, y),
            make_R2(dxf, dyf), w);
    }

    void do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
        return derived().do_begin_segment_piece(make_R2(xi, yi),
            make_R2(dxi, dyi));
    }

    void do_end_segment_piece(rvgf dxi, rvgf dyi, rvgf xi, rvgf yi) {
        return derived().do_end_segment_piece(make_R2(dxi, dyi),
            make_R2(xi, yi));
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1) {
        return derived().do_linear_segment_piece(ti, tf, make_R2(x0, y0),
            make_R2(x1, y1));
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return derived().do_quadratic_segment_piece(ti, tf, make_R2(x0, y0),
            make_R2(x1, y1), make_R2(x2, y2));
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rational_quadratic_segment_piece(ti, tf,
            make_R3(x0, y0, 1), make_R3(x1, y1, w1), make_R3(x2, y2, 1));
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_cubic_segment_piece(ti, tf, make_R2(x0, y0),
            make_R2(x1, y1), make_R2(x2, y2), make_R2(x3, y3));
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_point_regular_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_point_regular_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
