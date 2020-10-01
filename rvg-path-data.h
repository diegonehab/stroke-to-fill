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
#ifndef RVG_PATH_DATA_H
#define RVG_PATH_DATA_H

#include <vector>
#include <type_traits> // std::enable_if

#include "rvg-ptr.h"
#include "rvg-i-path.h"
#include "rvg-path-instruction.h"
#include "rvg-floatint.h"
#include "rvg-input-path-f-forward-if.h"
#include "rvg-regular-path-f-forward-if.h"
#include "rvg-decorated-path-f-forward-if.h"
#include "rvg-monotonic-parameters-f-forward-if.h"
#include "rvg-cubic-parameters-f-forward-if.h"
#include "rvg-offsetting-parameters-f-forward-if.h"
#include "rvg-join-parameters-f-forward-if.h"
#include "rvg-dashing-parameters-f-forward-if.h"

namespace rvg {

// Our internal representation for a path consists of an
// array with instructions, an array with offsets, and an
// array with data.  Each instruction entry has a
// corresponding offset entry.  For instructions requiring
// muitple data values, the offset points to the start of
// their data in the data array. Instructions requiring a
// single data value can embed the data value in the offset
// array itself to save an extra indirection and value
// storage. (This is the case for the parameter extension
// described below).
//
// Contours begin with a begin_contour instruction and end
// with either end_open_contour or end_closed_contour
// instructions. (This is related to PostScript's closepath
// or SVG's Z).
//
// Instructions corresponding to consecutive segments share
// a common endpoint in the data array.  In the table below,
// the endpoint implicit from the previous instruction is marked with '-'.
// The data to which the instruction's offset points is marked with '^'.
// Dada embedded into the offset array is marked with '*'.
//
// Input path
//   begin_contour                            ^x0 y0
//   end_open_contour                         ^-xn -yn
//   end_closed_contour                       ^-xn -yn
//   linear_segment                           ^-x0 -y0 x1 y1
//   quadratic_segment                        ^-x0 -y0 x1 y1 x2 y2
//   rational_quadratic_segment               ^-x0 -y0 x1 y1 w1 x2 y2
//   cubic_segment                            ^-x0 -y0 x1 y1 x2 y2 x3 y3
//
//??D Maybe remove x,y from begin/end_segment_piece?
//
// Regular path
//   begin_regular_contour                    ^-xi -yi -dxi -dyi
//   end_regular_open_contour                 ^-dxf -dyf -xf -yf
//   end_regular_closed_contour               ^-dxf -dyf -xf -yf
//   degenerate_segment                       ^xi yi dx dy xf yf
//   cusp                                     ^dxi dyi x y dxf dyf w
//   inner_cusp                               ^dxi dyi x y dxf dyf w
//   begin_segment_piece                      ^xi yi dxi dyi
//   end_segment_piece                        ^dxf dyf xf yf
//   linear_segment_piece                     ^ti tf x0 y0 x1 y1
//   quadratic_segment_piece                  ^ti tf x0 y0 x1 y1 x2 y2
//   rational_quadratic_segment_piece         ^ti tf x0 y0 x1 y1 w1 x2 y2
//   cubic_segment_piece                      ^ti tf x0 y0 x1 y1 x2 y2 x3 y3
//
// Cubic parameters
//   inflection_parameter                     *+t
//   double_point_parameter                   *+t
//
// Root parameters
//   root_dx_parameter                        *+t
//   root_dy_parameter                        *+t
//   root_dw_parameter                        *+t
//
// Offsetting parameters
//   offset_cusp_parameter                    *+t
//   evolute_cusp_parameter                   *+t
//
// Join parameters
//   join_tangent_parameter                 *+t
//   join_vertex_parameter                *+t
//
// Dashing parameters
//   begin_dash_parameter                     *+t
//   end_dash_parameter                       *+t
//   backward_begin_dash_parameter            *+t
//   backward_end_dash_parameter              *+t
//
// Decorated path
//   initial_cap                              ^x y dx dy
//   terminal_cap                             ^dx dy x y
//   initial_butt_cap                         ^x y dx dy
//   terminal_butt_cap                        ^dx dy x y
//   backward_initial_cap                     ^x y dx dy
//   backward_terminal_cap                    ^dx dy x y
//   backward_initial_butt_cap                ^x y dx dy
//   backward_terminal_butt_cap               ^dx dy x y
//   join                                     ^dx0 dy0 x y dx1 dy1 w
//   inner_join                               ^dx0 dy0 x y dx1 dy1 w
//
// The degenerate segment represents a segment with zero or
// very small length, for which there is a tangent direction dx dy
// obtained by other means.
//
// The idea is that the representation is reversible in the
// sense that traversing it forward or backward is (almost)
// equally easy. The datastructure also provide easy random
// access to the data for each instruction. This enables
// full parallelization in the future.

class path_data final:
    public i_path<path_data>,
    public boost::intrusive_ref_counter<path_data> {

    std::vector<path_instruction> m_instructions;
    std::vector<floatint> m_offsets;
    std::vector<rvgf> m_data;

public:

    using ptr = boost::intrusive_ptr<path_data>;
    using const_ptr = boost::intrusive_ptr<const path_data>;

    // default copy and move constructor and assignment operators
    path_data() = default;
    path_data(const path_data &other) = default;
    path_data(path_data &&other) = default;
    path_data &operator=(const path_data &other) = default;
    path_data &operator=(path_data &&other) = default;

    void propagate_orientations(void);
    void propagate_contour_orientations(int begin, int end);
    void find_first_contour_orientation(int begin, int end, rvgf &dx, rvgf &dy) const;
    void find_last_contour_orientation(int begin, int end, rvgf &dx, rvgf &dy) const;
    void propagate_contour_orientations_forward(rvgf dx, rvgf dy, int begin, int end);
    void propagate_contour_orientations_backward(rvgf dx, rvgf dy, int begin, int end);

    auto size(void) const {
        return m_instructions.size();
    }

    bool empty(void) const {
        return m_instructions.empty();
    }

    void clear(void) {
        m_instructions.clear();
        m_offsets.clear();
        m_data.clear();
    }

    void shrink_to_fit(void) {
        m_instructions.shrink_to_fit();
        m_offsets.shrink_to_fit();
        m_data.shrink_to_fit();
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void iterate(PF &sink, int first, int last) const;

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void iterate(PF &&sink, int first, int last) const {
        this->iterate(sink, first, last);
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void iterate(PF &sink) const;

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void iterate(PF &&sink) const {
        this->iterate(sink);
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void riterate(PF &sink, int last, int first) const;

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void riterate(PF &&sink, int last, int first) const {
        this->riterate(sink, last, first);
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void riterate(PF &sink) const;

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    void riterate(PF &&sink) const {
        this->riterate(sink);
    }

    template <typename DCMP>
    bool is_equal(const path_data &other, const DCMP &cmp);

private:

    template <typename PF>
    int riterate_parameters(PF &sink, int first, int last) const;

// i_input_path interface
friend i_input_path<path_data>;

    void do_begin_contour(rvgf x0, rvgf y0);

    void do_end_open_contour(rvgf x0, rvgf y0);

    void do_end_closed_contour(rvgf x0, rvgf y0);

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1);

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2);

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2);

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3);

// i_regular_path interface
friend i_regular_path<path_data>;

    void do_begin_regular_contour(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi);

    void do_end_regular_open_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf);

    void do_end_regular_closed_contour(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf);

    void do_degenerate_segment(rvgf xi, rvgf yi, rvgf dx, rvgf dy,
        rvgf xf, rvgf yf);

    void do_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w);

    void do_inner_cusp(rvgf dxi, rvgf dyi, rvgf x, rvgf y, rvgf dxf, rvgf dyf,
        rvgf w);

    void do_begin_segment_piece(rvgf xi, rvgf yi, rvgf dxi, rvgf dyi);

    void do_end_segment_piece(rvgf dxf, rvgf dyf, rvgf xf, rvgf yf);

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1);

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2);

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2);

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3);

// i_offsetting_parameters interface
friend i_offsetting_parameters<path_data>;

    void do_offset_cusp_parameter(rvgf t);

    void do_evolute_cusp_parameter(rvgf t);

// i_join_parameters interface
friend i_join_parameters<path_data>;

    void do_join_tangent_parameter(rvgf t);

    void do_join_vertex_parameter(rvgf t);

// i_cubic_parameters interface
friend i_cubic_parameters<path_data>;

    void do_inflection_parameter(rvgf t);

    void do_double_point_parameter(rvgf t);

// i_decorated_path interface
friend i_decorated_path<path_data>;

    void do_initial_cap(rvgf dx, rvgf dy, rvgf x, rvgf y);

    void do_terminal_cap(rvgf x, rvgf y, rvgf dx, rvgf dy);

    void do_initial_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y);

    void do_terminal_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy);

    void do_backward_initial_cap(rvgf dx, rvgf dy, rvgf x, rvgf y);

    void do_backward_terminal_cap(rvgf x, rvgf y, rvgf dx, rvgf dy);

    void do_backward_initial_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y);

    void do_backward_terminal_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy);

    void do_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w);

    void do_inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w);

// i_dashing_parameters interface
friend i_dashing_parameters<path_data>;

    void do_begin_dash_parameter(rvgf t);

    void do_end_dash_parameter(rvgf t);

    void do_backward_begin_dash_parameter(rvgf t);

    void do_backward_end_dash_parameter(rvgf t);

// i_monotonic_parameters interface
friend i_monotonic_parameters<path_data>;

    void do_root_dx_parameter(rvgf t);

    void do_root_dy_parameter(rvgf t);

    void do_root_dw_parameter(rvgf t);

// miscellaneous

    void push_instruction(path_instruction instruction, int rewind = -2);

    void push_embeded_instruction(path_instruction instruction, floatint datum);

    template <typename ...REST>
    void push_data(rvgf first, REST ...rest);

    void push_data(void);

};

#include "rvg-path-data.hpp"

} // namespace rvg

#endif
