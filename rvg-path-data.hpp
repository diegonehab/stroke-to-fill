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
template <typename DCMP>
bool path_data::is_equal(const path_data &other, const DCMP &cmp) {
    if (m_instructions.size() != other.m_instructions.size()) return false;
    if (m_offsets.size() != other.m_offsets.size()) return false;
    for (unsigned i = 0; i < m_instructions.size(); i++) {
        if (m_instructions[i] != other.m_instructions[i]) return false;
        switch (m_instructions[i]) {
			case path_instruction::inflection_parameter:
			case path_instruction::double_point_parameter:
			case path_instruction::root_dx_parameter:
			case path_instruction::root_dy_parameter:
			case path_instruction::root_dw_parameter:
			case path_instruction::offset_cusp_parameter:
			case path_instruction::evolute_cusp_parameter:
			case path_instruction::join_tangent_parameter:
			case path_instruction::join_vertex_parameter:
			case path_instruction::begin_dash_parameter:
			case path_instruction::end_dash_parameter:
			case path_instruction::backward_begin_dash_parameter:
			case path_instruction::backward_end_dash_parameter:
                if (!cmp(m_offsets[i].f, m_offsets[i].f)) return false;
                break;
            default:
                if (m_offsets[i].i != m_offsets[i].i) return false;
                break;
        }
    }
    return std::equal(m_data.begin(), m_data.end(), other.m_data.begin(),
       other.m_data.end(), cmp);
}

template <typename PF, typename>
void path_data::iterate(PF &sink, int first, int last) const {
    // These must be references becaues the forward_if
    // functions return either a reference to the sink
    // itself, or a reference to a static null sink
    auto &input = make_input_path_f_forward_if(sink);
    auto &regular = make_regular_path_f_forward_if(sink);
    auto &decorated = make_decorated_path_f_forward_if(sink);
    auto &roots = make_monotonic_parameters_f_forward_if(sink);
    auto &cubic = make_cubic_parameters_f_forward_if(sink);
    auto &offsetting = make_offsetting_parameters_f_forward_if(sink);
    auto &join = make_join_parameters_f_forward_if(sink);
    auto &dashing = make_dashing_parameters_f_forward_if(sink);
    first = std::max(0, first);
    last = std::min(static_cast<int>(m_instructions.size()), last);
    for (int index = first; index < last; ++index) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::begin_contour:
                input.begin_contour(m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::end_open_contour:
                input.end_open_contour(m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::end_closed_contour:
                input.end_closed_contour(m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::linear_segment:
                input.linear_segment(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::quadratic_segment:
                input.quadratic_segment(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5]);
                break;
            case path_instruction::rational_quadratic_segment:
                input.rational_quadratic_segment(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6]);
                break;
            case path_instruction::cubic_segment:
                input.cubic_segment(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6],
                    m_data[o.i+7]);
                break;

            case path_instruction::begin_regular_contour:
                regular.begin_regular_contour(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::end_regular_open_contour:
                regular.end_regular_open_contour(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::end_regular_closed_contour:
                regular.end_regular_closed_contour(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::degenerate_segment:
                regular.degenerate_segment(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5]);
                break;
            case path_instruction::cusp:
                regular.cusp(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6]);
                break;
            case path_instruction::inner_cusp:
                regular.inner_cusp(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6]);
                break;
            case path_instruction::begin_segment_piece:
                regular.begin_segment_piece(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::end_segment_piece:
                regular.end_segment_piece(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::linear_segment_piece:
                regular.linear_segment_piece(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5]);
                break;
            case path_instruction::quadratic_segment_piece:
                regular.quadratic_segment_piece(
                    m_data[o.i], m_data[o.i+1],
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+6], m_data[o.i+7]);
                break;
            case path_instruction::rational_quadratic_segment_piece:
                regular.rational_quadratic_segment_piece(
                    m_data[o.i], m_data[o.i+1],
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i+4], m_data[o.i+5], m_data[o.i+6],
                    m_data[o.i+7], m_data[o.i+8]);
                break;
            case path_instruction::cubic_segment_piece:
                regular.cubic_segment_piece(
                    m_data[o.i], m_data[o.i+1],
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+6], m_data[o.i+7],
                    m_data[o.i+8], m_data[o.i+9]);
                break;
            case path_instruction::inflection_parameter:
                cubic.inflection_parameter(o.f);
                break;
            case path_instruction::double_point_parameter:
                cubic.double_point_parameter(o.f);
                break;
            case path_instruction::root_dx_parameter:
                roots.root_dx_parameter(o.f);
                break;
            case path_instruction::root_dy_parameter:
                roots.root_dy_parameter(o.f);
                break;
            case path_instruction::root_dw_parameter:
                roots.root_dw_parameter(o.f);
                break;
            case path_instruction::join_tangent_parameter:
                join.join_tangent_parameter(o.f);
                break;
            case path_instruction::join_vertex_parameter:
                join.join_vertex_parameter(o.f);
                break;
            case path_instruction::offset_cusp_parameter:
                offsetting.offset_cusp_parameter(o.f);
                break;
            case path_instruction::evolute_cusp_parameter:
                offsetting.evolute_cusp_parameter(o.f);
                break;

            case path_instruction::initial_cap:
                decorated.initial_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::terminal_cap:
                decorated.terminal_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::initial_butt_cap:
                decorated.initial_butt_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::terminal_butt_cap:
                decorated.terminal_butt_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::backward_initial_cap:
                decorated.backward_initial_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::backward_terminal_cap:
                decorated.backward_terminal_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::backward_initial_butt_cap:
                decorated.backward_initial_butt_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;
            case path_instruction::backward_terminal_butt_cap:
                decorated.backward_terminal_butt_cap(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3]);
                break;

            case path_instruction::join:
                decorated.join(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6]);
                break;
            case path_instruction::inner_join:
                decorated.inner_join(m_data[o.i],
                    m_data[o.i+1], m_data[o.i+2],
                    m_data[o.i+3], m_data[o.i+4],
                    m_data[o.i+5], m_data[o.i+6]);
                break;

            case path_instruction::begin_dash_parameter:
                dashing.begin_dash_parameter(o.f);
                break;
            case path_instruction::end_dash_parameter:
                dashing.end_dash_parameter(o.f);
                break;
            case path_instruction::backward_begin_dash_parameter:
                dashing.backward_begin_dash_parameter(o.f);
                break;
            case path_instruction::backward_end_dash_parameter:
                dashing.backward_end_dash_parameter(o.f);
                break;
        }
    }
}

template <typename PF>
int path_data::
riterate_parameters(PF &sink, int first, int last) const {
    // These must be references because the forward_if
    // functions return either a reference to the sink
    // itself, or a reference to a static null sink
    auto &roots = make_monotonic_parameters_f_forward_if(sink);
    auto &cubic = make_cubic_parameters_f_forward_if(sink);
    auto &offsetting = make_offsetting_parameters_f_forward_if(sink);
    auto &join = make_join_parameters_f_forward_if(sink);
    auto &dashing = make_dashing_parameters_f_forward_if(sink);
    int index = last-1;
    while (index >= first) {
        auto o = m_offsets[index];
        switch (m_instructions[index]) {
            case path_instruction::inflection_parameter:
                cubic.inflection_parameter(1.f-o.f);
                break;
            case path_instruction::double_point_parameter:
                cubic.double_point_parameter(1.f-o.f);
                break;
            case path_instruction::root_dx_parameter:
                roots.root_dx_parameter(1.f-o.f);
                break;
            case path_instruction::root_dy_parameter:
                roots.root_dy_parameter(1.f-o.f);
                break;
            case path_instruction::root_dw_parameter:
                roots.root_dw_parameter(1.f-o.f);
                break;
            case path_instruction::offset_cusp_parameter:
                offsetting.offset_cusp_parameter(1.f-o.f);
                break;
            case path_instruction::evolute_cusp_parameter:
                offsetting.evolute_cusp_parameter(1.f-o.f);
                break;
            case path_instruction::join_tangent_parameter:
                join.join_tangent_parameter(1.f-o.f);
                break;
            case path_instruction::join_vertex_parameter:
                join.join_vertex_parameter(1.f-o.f);
                break;
            case path_instruction::begin_dash_parameter:
                dashing.backward_end_dash_parameter(1.f-o.f);
                break;
            case path_instruction::end_dash_parameter:
                dashing.backward_begin_dash_parameter(1.f-o.f);
                break;
            case path_instruction::backward_begin_dash_parameter:
                dashing.end_dash_parameter(1.f-o.f);
                break;
            case path_instruction::backward_end_dash_parameter:
                dashing.begin_dash_parameter(1.f-o.f);
                break;
            default:
                return last-1-index;
        }
        --index;
    }
    return last-1-index;
}

template <typename PF, typename>
void path_data::riterate(PF &sink, int last, int first) const {
    // These must be references becaues the forward_if
    // functions return either a reference to the sink
    // itself, or a reference to a static null sink
    auto &input = make_input_path_f_forward_if(sink);
    auto &regular = make_regular_path_f_forward_if(sink);
    auto &decorated = make_decorated_path_f_forward_if(sink);
    first = std::max(0, first);
    last = std::min(static_cast<int>(m_instructions.size()), last);
    int index = last-1;
    bool open_contour = true;
    while (index >= first) {
        auto o = m_offsets[index];
        auto skip_parameters = this->riterate_parameters(sink, first, index);
        switch (m_instructions[index]) {
            case path_instruction::begin_contour:
                if (open_contour) {
                    input.end_open_contour(m_data[o.i], m_data[o.i+1]);
                } else {
                    input.end_closed_contour(m_data[o.i], m_data[o.i+1]);
                }
                break;
            case path_instruction::end_open_contour:
                input.begin_contour(m_data[o.i], m_data[o.i+1]);
                open_contour = true;
                break;
            case path_instruction::end_closed_contour:
                input.begin_contour(m_data[o.i], m_data[o.i+1]);
                open_contour = false;
                break;
            case path_instruction::linear_segment:
                input.linear_segment(
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::quadratic_segment:
                input.quadratic_segment(
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::rational_quadratic_segment:
                input.rational_quadratic_segment(
                    m_data[o.i+5], m_data[o.i+6],
                    m_data[o.i+2], m_data[o.i+3],
                        m_data[o.i+4],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::cubic_segment:
                input.cubic_segment(
                    m_data[o.i+6], m_data[o.i+7],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;

            case path_instruction::begin_regular_contour:
                if (open_contour) {
                    regular.end_regular_open_contour(
                        -m_data[o.i+2], -m_data[o.i+3],
                        m_data[o.i], m_data[o.i+1]);
                } else {
                    regular.end_regular_closed_contour(
                        -m_data[o.i+2], -m_data[o.i+3],
                        m_data[o.i], m_data[o.i+1]);
                }
                break;
            case path_instruction::end_regular_open_contour:
                regular.begin_regular_contour(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                open_contour = true;
                break;
            case path_instruction::end_regular_closed_contour:
                regular.begin_regular_contour(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                open_contour = false;
                break;
            case path_instruction::degenerate_segment:
                regular.degenerate_segment(
                    m_data[o.i+4], m_data[o.i+5],
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::cusp:
                regular.inner_cusp(
                    -m_data[o.i+4], -m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1],
                    m_data[o.i+6]);
                break;
            case path_instruction::inner_cusp:
                regular.cusp(
                    -m_data[o.i+4], -m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1],
                    m_data[o.i+6]);
                break;
            case path_instruction::begin_segment_piece:
                regular.end_segment_piece(
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::end_segment_piece:
                regular.begin_segment_piece(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                break;

            case path_instruction::linear_segment_piece:
                regular.linear_segment_piece(
                    1-m_data[o.i+1], 1-m_data[o.i],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3]);
                break;
            case path_instruction::quadratic_segment_piece:
                regular.quadratic_segment_piece(
                    1-m_data[o.i+1], 1-m_data[o.i],
                    m_data[o.i+6], m_data[o.i+7],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3]);
                break;
            case path_instruction::rational_quadratic_segment_piece:
                regular.rational_quadratic_segment_piece(
                    1-m_data[o.i+1], 1-m_data[o.i],
                    m_data[o.i+7], m_data[o.i+8],
                    m_data[o.i+4], m_data[o.i+5], m_data[o.i+6],
                    m_data[o.i+2], m_data[o.i+3]);
                break;
            case path_instruction::cubic_segment_piece:
                regular.cubic_segment_piece(
                    1-m_data[o.i+1], 1-m_data[o.i],
                    m_data[o.i+8], m_data[o.i+9],
                    m_data[o.i+6], m_data[o.i+7],
                    m_data[o.i+4], m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3]);
                break;

            case path_instruction::initial_cap:
                decorated.backward_terminal_cap(
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::terminal_cap:
                decorated.backward_initial_cap(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                break;
            case path_instruction::initial_butt_cap:
                decorated.backward_terminal_butt_cap(
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::terminal_butt_cap:
                decorated.backward_initial_butt_cap(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                break;
            case path_instruction::backward_initial_cap:
                decorated.terminal_cap(
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::backward_terminal_cap:
                decorated.initial_cap(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                break;
            case path_instruction::backward_initial_butt_cap:
                decorated.terminal_butt_cap(
                    -m_data[o.i+2], -m_data[o.i+3],
                    m_data[o.i], m_data[o.i+1]);
                break;
            case path_instruction::backward_terminal_butt_cap:
                decorated.initial_butt_cap(
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1]);
                break;
            case path_instruction::join:
                decorated.inner_join(
                    -m_data[o.i+4], -m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1],
                    m_data[o.i+6]);
                break;
            case path_instruction::inner_join:
                decorated.join(
                    -m_data[o.i+4], -m_data[o.i+5],
                    m_data[o.i+2], m_data[o.i+3],
                    -m_data[o.i], -m_data[o.i+1],
                    m_data[o.i+6]);
                break;
            default:
                break;
        }
        index -= skip_parameters;
        --index;
    }
}

template <typename PF, typename>
void path_data::iterate(PF &sink) const {
    this->iterate(sink, 0, static_cast<int>(m_instructions.size()));
}

template <typename PF, typename>
void path_data::riterate(PF &sink) const {
    this->riterate(sink, static_cast<int>(m_instructions.size()), 0);
}

template <typename ...REST>
void path_data::push_data(rvgf first, REST ...rest) {
    m_data.push_back(first);
    push_data(rest...);
}
