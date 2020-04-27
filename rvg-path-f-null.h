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
#ifndef RVG_PATH_F_NULL_H
#define RVG_PATH_F_NULL_H

#include "rvg-i-path.h"

namespace rvg {

class path_f_null final: public i_path<path_f_null> {

private:

friend i_input_path<path_f_null>;

    void do_begin_contour(rvgf, rvgf) { ; }
    void do_end_open_contour(rvgf, rvgf) { ; }
    void do_end_closed_contour(rvgf, rvgf) { ; }
    void do_linear_segment(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_quadratic_segment(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_rational_quadratic_segment(rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf) { ; }
    void do_cubic_segment(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf) { ; }

friend i_regular_path<path_f_null>;

    void do_begin_regular_contour(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_end_regular_open_contour(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_end_regular_closed_contour(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_degenerate_segment(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_cusp(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_inner_cusp(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_begin_segment_piece(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_end_segment_piece(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_linear_segment_piece(rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf) { ; }
    void do_quadratic_segment_piece(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf) { ; }
    void do_rational_quadratic_segment_piece(rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_cubic_segment_piece(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf,
        rvgf, rvgf, rvgf, rvgf) { ; }

friend i_offsetting_parameters<path_f_null>;

    void do_offset_cusp_parameter(rvgf) { ; }
    void do_evolute_cusp_parameter(rvgf) { ; }

friend i_join_parameters<path_f_null>;

    void do_join_tangent_parameter(rvgf) { ; }
    void do_join_vertex_parameter(rvgf) { ; }

friend i_cubic_parameters<path_f_null>;

    void do_inflection_parameter(rvgf) { ; }
    void do_double_point_parameter(rvgf) { ; }

friend i_monotonic_parameters<path_f_null>;

    void do_root_dx_parameter(rvgf) { ; }
    void do_root_dy_parameter(rvgf) { ; }
    void do_root_dw_parameter(rvgf) { ; }

friend i_dashing_parameters<path_f_null>;

    void do_begin_dash_parameter(rvgf) { ; }
    void do_end_dash_parameter(rvgf) { ; }
    void do_backward_begin_dash_parameter(rvgf) { ; }
    void do_backward_end_dash_parameter(rvgf) { ; }

friend i_decorated_path<path_f_null>;

    void do_initial_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_terminal_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_initial_butt_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_terminal_butt_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_backward_initial_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_backward_terminal_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_backward_initial_butt_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_backward_terminal_butt_cap(rvgf, rvgf, rvgf, rvgf) { ; }
    void do_join(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
    void do_inner_join(rvgf, rvgf, rvgf, rvgf, rvgf, rvgf, rvgf) { ; }
};

static inline path_f_null make_path_f_null(void) {
    return path_f_null{};
}

} // namespace rvg

#endif
