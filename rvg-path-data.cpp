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
#include "rvg-path-data.h"

namespace rvg {

static bool is_irregular(rvgf dx, rvgf dy) {
    return util::is_almost_zero(dx) && util::is_almost_zero(dy);
}

static void propagate_orientation(rvgf &dx, rvgf &dy, rvgf &dx0, rvgf &dy0) {
    bool ir0 = is_irregular(dx0, dy0);
    // If segment orientation is degenerate, replace with current
    if (ir0) {
        dx0 = dx;
        dy0 = dy;
    // otherwise, replace current with it
    } else {
        dx = dx0;
        dy = dy0;
    }
}

void path_data::find_first_contour_orientation(int begin, int end,
    rvgf &dx, rvgf &dy) const {
    for (int index = begin; index < end; ++index) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::degenerate_segment:
            case path_instruction::begin_segment_piece: {
                dx = m_data[o.i+2];
                dy = m_data[o.i+3];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            case path_instruction::end_segment_piece: {
                dx = m_data[o.i+0];
                dy = m_data[o.i+1];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            case path_instruction::inner_cusp:
            case path_instruction::cusp: {
                dx = m_data[o.i+0];
                dy = m_data[o.i+1];
                if (!is_irregular(dx, dy)) return;
                dx = m_data[o.i+4];
                dy = m_data[o.i+5];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            default:
                break;
        }
    }
    dx = dy = 0.f;
}

void path_data::find_last_contour_orientation(int begin, int end,
    rvgf &dx, rvgf &dy) const {
    for (int index = end-1; index >= begin; --index) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::degenerate_segment:
            case path_instruction::begin_segment_piece: {
                dx = m_data[o.i+2];
                dy = m_data[o.i+3];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            case path_instruction::end_segment_piece: {
                dx = m_data[o.i+0];
                dy = m_data[o.i+1];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            case path_instruction::inner_cusp:
            case path_instruction::cusp: {
                dx = m_data[o.i+4];
                dy = m_data[o.i+5];
                if (!is_irregular(dx, dy)) return;
                dx = m_data[o.i+0];
                dy = m_data[o.i+1];
                if (!is_irregular(dx, dy)) return;
                break;
            }
            default:
                break;
        }
    }
    dx = dy = 0.f;
}

void path_data::propagate_contour_orientations_forward(rvgf dx, rvgf dy,
    int begin, int end) {
    for (int index = begin; index < end; ++index) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::degenerate_segment:
            case path_instruction::begin_segment_piece:
                propagate_orientation(dx, dy, m_data[o.i+2], m_data[o.i+3]);
                break;
            case path_instruction::end_segment_piece:
                propagate_orientation(dx, dy, m_data[o.i+0], m_data[o.i+1]);
                break;
            case path_instruction::inner_cusp:
            case path_instruction::cusp:
                propagate_orientation(dx, dy, m_data[o.i+0], m_data[o.i+1]);
                propagate_orientation(dx, dy, m_data[o.i+4], m_data[o.i+5]);
                break;
            default:
                break;
        }
    }
}

void path_data::propagate_contour_orientations_backward(rvgf dx, rvgf dy,
    int begin, int end) {
    for (int index = end-1; index >= begin; --index) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::degenerate_segment:
            case path_instruction::begin_segment_piece:
                propagate_orientation(dx, dy, m_data[o.i+2], m_data[o.i+3]);
                break;
            case path_instruction::end_segment_piece:
                propagate_orientation(dx, dy, m_data[o.i+0], m_data[o.i+1]);
                break;
            case path_instruction::inner_cusp:
            case path_instruction::cusp:
                propagate_orientation(dx, dy, m_data[o.i+4], m_data[o.i+5]);
                propagate_orientation(dx, dy, m_data[o.i+0], m_data[o.i+1]);
                break;
            default:
                break;
        }
    }
}

void path_data::propagate_contour_orientations(int begin, int end) {
    // Make sure contour is delimited by begin and end
    auto front = m_instructions[begin];
    assert(front == path_instruction::begin_regular_contour); (void) front;
    auto back = m_instructions[end-1];
    assert(back == path_instruction::end_regular_open_contour ||
        back == path_instruction::end_regular_closed_contour ); (void) back;
    // Orientations start undefined
    rvgf initial_dx = 1.f, initial_dy = 0.f;
    rvgf final_dx = 0.f, final_dy = 0.f;
    // If contour is closed, we define them before propagation starts
    if (m_instructions[end-1] == path_instruction::end_regular_closed_contour) {
        find_first_contour_orientation(begin, end, initial_dx, initial_dy);
        if (is_irregular(initial_dx, initial_dy)) {
            initial_dx = 1.f;
            initial_dy = 0.f;
        }
        find_last_contour_orientation(begin, end, final_dx, final_dy);
    }
    propagate_contour_orientations_forward(final_dx, final_dy, begin, end);
    propagate_contour_orientations_backward(initial_dx, initial_dy, begin, end);
}

void path_data::propagate_orientations(void) {
	if (m_instructions.empty()) return;
    int begin = 0;
    for (int index = 0; index < (int) m_instructions.size(); ++index) {
        auto insn = m_instructions[index];
        if (insn == path_instruction::begin_regular_contour) {
            begin = index;
        }
        if (insn == path_instruction::end_regular_open_contour ||
            insn == path_instruction::end_regular_closed_contour) {
            propagate_contour_orientations(begin, index+1);
        }
    }
}

void path_data::do_begin_contour(rvgf x0, rvgf y0) {
    push_instruction(path_instruction::begin_contour, 0);
    push_data(x0, y0);
}

void path_data::do_end_open_contour(rvgf x0, rvgf y0) {
    (void) x0; (void) y0;
    push_instruction(path_instruction::end_open_contour);
}

void path_data::do_end_closed_contour(rvgf x0, rvgf y0) {
    (void) x0; (void) y0;
    push_instruction(path_instruction::end_closed_contour);
}

void path_data::do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(path_instruction::linear_segment);
    push_data(x1, y1);
}

void path_data::do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(path_instruction::quadratic_segment);
    push_data(x1, y1, x2, y2);
}

void path_data::do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf w1, rvgf x2, rvgf y2) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(path_instruction::rational_quadratic_segment);
    push_data(x1, y1, w1, x2, y2);
}

void path_data::do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(path_instruction::cubic_segment);
    push_data(x1, y1, x2, y2, x3, y3);
}

void path_data::do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
    (void) xi; (void) yi;   // ignore, comes from next instruction
    (void) dxi; (void) dyi; // ignore, comes from next instruction
    push_instruction(path_instruction::begin_regular_contour, 0);
}

void path_data::do_end_regular_open_contour(rvgf dxf, rvgf dyf,
    rvgf xf, rvgf yf) {
    (void) dxf; (void) dyf; // ignore, comes from previous instruction
    (void) xf; (void) yf;   // ignore, comes from previous instruction
    push_instruction(path_instruction::end_regular_open_contour, -4);
}

void path_data::do_end_regular_closed_contour(rvgf dxf, rvgf dyf,
    rvgf xf, rvgf yf) {
    (void) dxf; (void) dyf; // ignore, comes from previous instruction
    (void) xf; (void) yf;   // ignore, comes from previous instruction
    push_instruction(path_instruction::end_regular_closed_contour, -4);
}

void path_data::do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
        rvgf xf, rvgf yf) {
    push_instruction(path_instruction::degenerate_segment, 0);
    push_data(xi, yi, dx, dy, xf, yf);
}

void path_data::do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
    rvgf w) {
    push_instruction(path_instruction::cusp, 0);
    push_data(dxi, dyi, x, y, dxf, dyf, w);
}

void path_data::do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf,
    rvgf dyf, rvgf w) {
    push_instruction(path_instruction::inner_cusp, 0);
    push_data(dxi, dyi, x, y, dxf, dyf, w);
}

void path_data::do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi) {
    push_instruction(path_instruction::begin_segment_piece, 0);
    push_data(xi, yi, dxi, dyi);
}

void path_data::do_end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf) {
    push_instruction(path_instruction::end_segment_piece, 0);
    push_data(dxf, dyf, xf, yf);
}

void path_data::do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
    rvgf x1, rvgf y1) {
    push_instruction(path_instruction::linear_segment_piece, 0);
    push_data(ti, tf, x0, y0, x1, y1);
}

void path_data::do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
    rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
    push_instruction(path_instruction::quadratic_segment_piece, 0);
    push_data(ti, tf, x0, y0, x1, y1, x2, y2);
}

void path_data::do_rational_quadratic_segment_piece(rvgf ti, rvgf tf,
    rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
    push_instruction(path_instruction::rational_quadratic_segment_piece, 0);
    push_data(ti, tf, x0, y0, x1, y1, w1, x2, y2);
}

void path_data::do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
    rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
    push_instruction(path_instruction::cubic_segment_piece, 0);
    push_data(ti, tf, x0, y0, x1, y1, x2, y2, x3, y3);
}

void path_data::do_inflection_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::inflection_parameter, t);
}

void path_data::do_double_point_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::double_point_parameter, t);
}

void path_data::do_root_dx_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::root_dx_parameter, t);
}

void path_data::do_root_dy_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::root_dy_parameter, t);
}

void path_data::do_root_dw_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::root_dw_parameter, t);
}

void path_data::do_offset_cusp_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::offset_cusp_parameter, t);
}

void path_data::do_join_tangent_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::join_tangent_parameter, t);
}

void path_data::do_join_vertex_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::join_vertex_parameter, t);
}

void path_data::do_evolute_cusp_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::evolute_cusp_parameter, t);
}

void path_data::do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
    push_instruction(path_instruction::initial_cap, 0);
    push_data(x, y, dx, dy);
}

void path_data::do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
    (void) x; (void) y; // ignored, comes from previous instruction
    push_instruction(path_instruction::terminal_cap, 0);
    push_data(dx, dy, x, y);
}

void path_data::do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
    push_instruction(path_instruction::initial_butt_cap, 0);
    push_data(x, y, dx, dy);
}

void path_data::do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
    (void) x; (void) y; // ignored, comes from previous instruction
    push_instruction(path_instruction::terminal_butt_cap, 0);
    push_data(dx, dy, x, y);
}

void path_data::do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
    push_instruction(path_instruction::backward_initial_cap, 0);
    push_data(x, y, dx, dy);
}

void path_data::do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
    (void) x; (void) y; // ignored, comes from previous instruction
    push_instruction(path_instruction::backward_terminal_cap, 0);
    push_data(dx, dy, x, y);
}

void path_data::do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
    push_instruction(path_instruction::backward_initial_butt_cap, 0);
    push_data(x, y, dx, dy);
}

void path_data::do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
    (void) x; (void) y; // ignored, comes from previous instruction
    push_instruction(path_instruction::backward_terminal_butt_cap, 0);
    push_data(dx, dy, x, y);
}

void path_data::do_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1, rvgf w) {
    push_instruction(path_instruction::join, 0);
    push_data(dx0, dy0, x, y, dx1, dy1, w);
}

void path_data::do_inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1, rvgf w) {
    push_instruction(path_instruction::inner_join, 0);
    push_data(dx0, dy0, x, y, dx1, dy1, w);
}

void path_data::do_begin_dash_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::begin_dash_parameter, t);
}

void path_data::do_end_dash_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::end_dash_parameter, t);
}

void path_data::do_backward_begin_dash_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::backward_begin_dash_parameter, t);
}

void path_data::do_backward_end_dash_parameter(rvgf t) {
    push_embeded_instruction(path_instruction::backward_end_dash_parameter, t);
}

void path_data::push_instruction(path_instruction instruction, int rewind) {
    m_instructions.push_back(instruction);
    m_offsets.push_back(static_cast<rvgi>(m_data.size()+rewind));
}

void path_data::push_embeded_instruction(path_instruction instruction,
    floatint datum) {
    m_instructions.push_back(instruction);
    m_offsets.push_back(datum);
}

void path_data::push_data(void) {
    ;
}

} // namespace rvg
