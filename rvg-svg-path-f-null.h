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
#ifndef RVG_SVG_PATH_F_NULL_H
#define RVG_SVG_PATH_F_NULL_H

#include <iostream>

#include "rvg-i-svg-path.h"

namespace rvg {

class svg_path_f_null final: public i_svg_path<svg_path_f_null> {

friend i_svg_path<svg_path_f_null>;

    void do_move_to_abs(rvgf, rvgf) { }
    void do_move_to_rel(rvgf, rvgf) { }

    void do_close_path(void) { }

    void do_line_to_abs(rvgf, rvgf) { }
    void do_line_to_rel(rvgf, rvgf) { }

    void do_hline_to_abs(rvgf) { }
    void do_hline_to_rel(rvgf) { }

    void do_vline_to_abs(rvgf) { }
    void do_vline_to_rel(rvgf) { }

    void do_quad_to_abs(rvgf, rvgf, rvgf, rvgf) { }
    void do_quad_to_rel(rvgf, rvgf, rvgf, rvgf) { }

    void do_squad_to_abs(rvgf, rvgf) { }
    void do_squad_to_rel(rvgf, rvgf) { }

    void do_rquad_to_abs(rvgf, rvgf, rvgf, rvgf, rvgf) { }
    void do_rquad_to_rel(rvgf, rvgf, rvgf, rvgf, rvgf) { }

    void do_svg_arc_to_abs(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { }
    void do_svg_arc_to_rel(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { }

    void do_cubic_to_abs(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { }
    void do_cubic_to_rel(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { }

    void do_scubic_to_abs(rvgf, rvgf, rvgf, rvgf) { }
    void do_scubic_to_rel(rvgf, rvgf, rvgf, rvgf) { }
};

inline svg_path_f_null& make_svg_path_f_null(void) {
    static svg_path_f_null n;
    return n;
}

} // namespace rvg

#endif
