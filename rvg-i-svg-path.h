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
#ifndef RVG_I_SVG_PATH_H
#define RVG_I_SVG_PATH_H

#include "rvg-meta.h"
#include "rvg-floatint.h"

namespace rvg {

template <typename DERIVED>
class i_svg_path {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }
    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    void move_to_abs(rvgf x0, rvgf y0) {
        return derived().do_move_to_abs(x0, y0);
    }

    void move_to_rel(rvgf x0, rvgf y0) {
        return derived().do_move_to_rel(x0, y0);
    }

    void close_path(void) {
        return derived().do_close_path();
    }

    void line_to_abs(rvgf x1, rvgf y1) {
        return derived().do_line_to_abs(x1, y1);
    }

    void line_to_rel(rvgf x1, rvgf y1) {
        return derived().do_line_to_rel(x1, y1);
    }

    void hline_to_abs(rvgf x1) {
        return derived().do_hline_to_abs(x1);
    }

    void hline_to_rel(rvgf x1) {
        return derived().do_hline_to_rel(x1);
    }

    void vline_to_abs(rvgf y1) {
        return derived().do_vline_to_abs(y1);
    }

    void vline_to_rel(rvgf y1) {
        return derived().do_vline_to_rel(y1);
    }

    void quad_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return derived().do_quad_to_abs(x1, y1, x2, y2);
    }

    void quad_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        return derived().do_quad_to_rel(x1, y1, x2, y2);
    }

    void squad_to_abs(rvgf x2, rvgf y2) {
        return derived().do_squad_to_abs(x2, y2);
    }

    void squad_to_rel(rvgf x2, rvgf y2) {
        return derived().do_squad_to_rel(x2, y2);
    }

    void rquad_to_abs(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rquad_to_abs(x1, y1, w1, x2, y2);
    }

    void rquad_to_rel(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        return derived().do_rquad_to_rel(x1, y1, w1, x2, y2);
    }

    void svg_arc_to_abs(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        return derived().do_svg_arc_to_abs(rx, ry, a, fa, fs, x2, y2);
    }

    void svg_arc_to_rel(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        return derived().do_svg_arc_to_rel(rx, ry, a, fa, fs, x2, y2);
    }

    void cubic_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        return derived().do_cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void cubic_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        return derived().do_cubic_to_rel(x1, y1, x2, y2, x3, y3);
    }

    void scubic_to_abs(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_scubic_to_abs(x2, y2, x3, y3);
    }

    void scubic_to_rel(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return derived().do_scubic_to_rel(x2, y2, x3, y3);
    }

};

} // namespace rvg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_svg_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_svg_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
