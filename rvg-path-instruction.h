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
#ifndef RVG_PATH_INSTRUCTION_H
#define RVG_PATH_INSTRUCTION_H

#include <cstdint>

namespace rvg {

// These are the instructions that live in a path
enum class path_instruction: uint8_t {
    // input path instructions
    begin_contour = 1,
    end_open_contour,
    end_closed_contour,
    linear_segment,
    quadratic_segment,
    rational_quadratic_segment,
    cubic_segment,

    // regular path instructions
    begin_regular_contour,
    end_regular_open_contour,
    end_regular_closed_contour,
    degenerate_segment,
    cusp,
	inner_cusp,
    begin_segment_piece,
    end_segment_piece,
	linear_segment_piece,
    quadratic_segment_piece,
    rational_quadratic_segment_piece,
    cubic_segment_piece,

    // cubic parameters
    inflection_parameter,
    double_point_parameter,

    // root parameters
    root_dx_parameter,
    root_dy_parameter,
    root_dw_parameter,

    // offsetting parameters
    offset_cusp_parameter,
    evolute_cusp_parameter,

    // join parameters
    join_tangent_parameter,
    join_vertex_parameter,

    // stroke path instructions
	initial_cap,
	terminal_cap,
	backward_initial_cap,
	backward_terminal_cap,
	initial_butt_cap,
	terminal_butt_cap,
	backward_initial_butt_cap,
	backward_terminal_butt_cap,
	join,
	inner_join,

    // dashing parameters
	begin_dash_parameter,
	end_dash_parameter,
	backward_begin_dash_parameter,
	backward_end_dash_parameter
};

} // namespace rvg

#endif
