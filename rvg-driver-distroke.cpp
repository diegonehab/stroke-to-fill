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
#include <utility>
#include <omp.h>

#include "rvg-lua.h"

#include "rvg-rgba.h"
#include "rvg-i-input-path.h"
#include "rvg-xform.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-image.h"
#include "rvg-pngio.h"
#include "rvg-tuple.h"
#include "rvg-bezier.h"
#include "rvg-bezier-arc-length.h"

#include "rvg-lua-facade.h"

#include "rvg-driver-distroke.h"

#include <boost/range/adaptor/sliced.hpp>

namespace rvg {
    namespace driver {
        namespace distroke {

template <typename DERIVED>
DERIVED &
i_accelerated<DERIVED>::
derived(void) {
    return *static_cast<DERIVED *>(this);
}

template <typename DERIVED>
const DERIVED &
i_accelerated<DERIVED>::
derived(void) const {
    return *static_cast<const DERIVED *>(this);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
color(RGBA8 c) {
    return derived().do_color(c);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
transform(const xform &xf) {
    return derived().do_transform(xf);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
width(rvgf w) {
    return derived().do_width(w);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
miter_limit(rvgf m) {
    return derived().do_miter_limit(m);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
    return derived().do_linear_segment_piece(ti, tf, x0, y0, x1, y1);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
    rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
    return derived().do_quadratic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
    return derived().do_cubic_segment_piece(ti, tf, x0, y0, x1, y1,
        x2, y2, x3, y3);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf w1, rvgf x2, rvgf y2) {
    return derived().do_rational_quadratic_segment_piece(ti, tf, x0, y0,
        x1, y1, w1, x2, y2);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
round_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    return derived().do_round_join(nx0, ny0, x, y, nx1, ny1);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
bevel_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    return derived().do_bevel_join(nx0, ny0, x, y, nx1, ny1);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
miter_clip_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    return derived().do_miter_clip_join(nx0, ny0, x, y, nx1, ny1);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
round_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    return derived().do_round_cap(x, y, nx, ny);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
square_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    return derived().do_square_cap(x, y, nx, ny);
}

template <typename DERIVED>
void
i_accelerated<DERIVED>::
triangle_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    return derived().do_triangle_cap(x, y, nx, ny);
}

accelerated::
accelerated(void):
    m_instructions_ptr(std::make_shared<std::vector<e_type>>()),
    m_data_ptr(std::make_shared<std::vector<rvgf>>()),
    m_instructions(*m_instructions_ptr),
    m_data(*m_data_ptr) {
    ;
}

void
accelerated::
push_data(void) {
    ;
}

template <typename ...REST>
void
accelerated::
push_data(rvgf first, REST ...rest) {
    m_data.push_back(first);
    push_data(rest...);
}

void
accelerated::
do_color(RGBA8 c) {
    RGBA<rvgf> cf{c};
    push_data(cf[0], cf[1], cf[2], cf[3]);
    m_instructions.push_back(e_type::color);
}

void
accelerated::
do_transform(const xform &xf) {
    push_data(xf[0][0], xf[0][1], xf[0][2], xf[1][0], xf[1][1], xf[1][2]);
    m_instructions.push_back(e_type::transform);
}

void
accelerated::
do_width(rvgf w) {
    push_data(w);
    m_instructions.push_back(e_type::width);
}

void
accelerated::
do_miter_limit(rvgf m) {
    push_data(m);
    m_instructions.push_back(e_type::miter_limit);
}

void
accelerated::
do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
    push_data(ti, tf, x0, y0, x1, y1);
    m_instructions.push_back(e_type::linear_segment_piece);
}

void
accelerated::
do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2) {
    push_data(ti, tf, x0, y0, x1, y1, x2, y2);
    m_instructions.push_back(e_type::quadratic_segment_piece);
}

void
accelerated::
do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
    push_data(ti, tf, x0, y0, x1, y1, x2, y2, x3, y3);
    m_instructions.push_back(e_type::cubic_segment_piece);
}

void
accelerated::
do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1, rvgf y1,
    rvgf w1, rvgf x2, rvgf y2) {
    push_data(ti, tf, x0, y0, x1, y1, w1, x2, y2);
    m_instructions.push_back(e_type::rational_quadratic_segment_piece);
}

void
accelerated::
do_round_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    push_data(nx0, ny0, x, y, nx1, ny1);
    m_instructions.push_back(e_type::round_join);
}

void
accelerated::
do_bevel_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    push_data(nx0, ny0, x, y, nx1, ny1);
    m_instructions.push_back(e_type::bevel_join);
}

void
accelerated::
do_miter_clip_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
    rvgf nx1, rvgf ny1) {
    push_data(nx0, ny0, x, y, nx1, ny1);
    m_instructions.push_back(e_type::miter_clip_join);
}

void
accelerated::
do_round_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    push_data(x, y, nx, ny);
    m_instructions.push_back(e_type::round_cap);
}

void
accelerated::
do_square_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    push_data(x, y, nx, ny);
    m_instructions.push_back(e_type::square_cap);
}

void
accelerated::
do_triangle_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
    push_data(x, y, nx, ny);
    m_instructions.push_back(e_type::triangle_cap);
}

template <typename F>
void
accelerated::
iterate(F &&sink) const {
    this->iterate(sink);
}

template <typename F>
void
accelerated::
iterate(F &sink) const {
    unsigned cursor = 0;
    for (auto i: m_instructions) {
        switch (i) {
            case e_type::width:
                sink.width(m_data[cursor]);
                cursor++;
                break;
            case e_type::miter_limit:
                sink.miter_limit(m_data[cursor]);
                cursor++;
                break;
            case e_type::color:
                sink.color(make_rgba(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3]));
                cursor += 4;
                break;
            case e_type::linear_segment_piece:
                sink.linear_segment_piece(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3], m_data[cursor+4],
                    m_data[cursor+5]);
                cursor += 6;
                break;
            case e_type::quadratic_segment_piece:
                sink.quadratic_segment_piece(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3], m_data[cursor+4],
                    m_data[cursor+5], m_data[cursor+6], m_data[cursor+7]);
                cursor += 8;
                break;
            case e_type::rational_quadratic_segment_piece:
                sink.rational_quadratic_segment_piece(m_data[cursor],
                    m_data[cursor+1], m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5], m_data[cursor+6],
                    m_data[cursor+7], m_data[cursor+8]);
                cursor += 9;
                break;
            case e_type::cubic_segment_piece:
                sink.cubic_segment_piece(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5],
                    m_data[cursor+6], m_data[cursor+7],
                    m_data[cursor+8], m_data[cursor+9]);
                cursor += 10;
                break;
            case e_type::transform:
                sink.transform(make_affinity(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5]));
                cursor += 6;
                break;
            case e_type::round_join:
                sink.round_join(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5]);
                cursor += 6;
                break;
            case e_type::bevel_join:
                sink.bevel_join(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5]);
                cursor += 6;
                break;
            case e_type::miter_clip_join:
                sink.miter_clip_join(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3],
                    m_data[cursor+4], m_data[cursor+5]);
                cursor += 6;
                break;
            case e_type::round_cap:
                sink.round_cap(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3]);
                cursor += 4;
                break;
            case e_type::square_cap:
                sink.square_cap(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3]);
                cursor += 4;
                break;
            case e_type::triangle_cap:
                sink.triangle_cap(m_data[cursor], m_data[cursor+1],
                    m_data[cursor+2], m_data[cursor+3]);
                cursor += 4;
                break;
            default:
                assert(0);
                break;
        }
    }
}

template <typename SINK>
class input_path_f_to_accelerated final:
    public i_input_path<input_path_f_to_accelerated<SINK>> {

public:

    explicit input_path_f_to_accelerated(rvgf width,
        const stroke_style &style, SINK &&sink):
        m_sink(std::forward<SINK>(sink)),
        m_width{width},
        m_style{style},
        m_dashing{!style.get_dashes().empty()},
        m_initial_pen_down{true}, m_current_pen_down{true},
        m_initial_length_needed{0}, m_current_length_needed{0},
        m_initial_dash{0}, m_current_dash{0},
        m_resets_on_move{style.get_resets_on_move()},
        m_pen_down_at_begin_contour{true} {

        if (m_dashing) {
            const auto &dashes = m_style.get_dashes();
            rvgf length{0};
            for (rvgf d: dashes) {
                length += d*m_width;
            }
            rvgf offset = std::fmod(m_style.get_dash_offset()*m_width,
                length);
            if (offset < rvgf{0}) offset = length+offset;
            m_initial_pen_down = true;
            m_initial_dash = 0;
            while (offset >= dashes[m_initial_dash]*m_width) {
                offset -= dashes[m_initial_dash]*m_width;
                m_initial_dash++;
                if (m_initial_dash >= (int) dashes.size()) {
                    m_initial_dash = 0;
                }
                m_initial_pen_down = !m_initial_pen_down;
            }
            m_initial_length_needed = dashes[m_initial_dash]*m_width - offset;

            m_current_pen_down = m_initial_pen_down;
            m_current_dash = m_initial_dash;
            m_current_length_needed = m_initial_length_needed;
            m_pen_down_at_begin_contour = m_current_pen_down;
        }
    }

private:

    SINK m_sink;
    rvgf m_width;
    const stroke_style &m_style;

    bool m_dashing;
    bool m_initial_pen_down, m_current_pen_down;
    rvgf m_initial_length_needed, m_current_length_needed;
    int m_initial_dash, m_current_dash;
    bool m_resets_on_move, m_pen_down_at_begin_contour;

    R2 m_first_position, m_first_normal, m_initial_normal,
       m_final_normal, m_previous_final_normal;
    bool m_brand_new_contour;

    bool is_almost_zero(const R2 &p) {
        return util::is_almost_zero(p[0]) && util::is_almost_zero(p[1]);
    }

    R2 initial_normal(const R2 &p0, const R2 &p1, const R2 &p2) {
        auto dp = perp(p1-p0);
        if (!is_almost_zero(dp)) {
            return dp;
        } else {
            return perp(p2-p0);
        }
    }

    R2 initial_normal(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        return initial_normal(R2{x0, y0}, R2{x1, y1}, R2{x2, y2});
    }

    R2 initial_normal(const R2 &p0, const R2 &p1, const R2 &p2, const R2 &p3) {
        auto dp = perp(p1-p0);
        if (!is_almost_zero(dp)) {
            return dp;
        } else {
            dp = perp(p2-p0);
            if (!is_almost_zero(dp)) {
                return dp;
            } else {
                return perp(p3-p0);
            }
        }
    }

    R2 initial_normal(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        return initial_normal(R2{x0, y0}, R2{x1, y1}, R2{x2, y2}, R2{x3, y3});
    }

    R2 initial_normal(const R2 &p0, const R3 &p1, const R2 &p2) {
        int s = util::sgn(p1[2]);
        auto dp = perp(R2{s*(p1[0]-p1[2]*p0[0]), s*(p1[1]-p1[2]*p0[1])});
        if (!is_almost_zero(dp)) {
            return dp;
        } else {
            return perp(p2-p0);
        }
    }

    R2 initial_normal(rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf w1,
        rvgf x2, rvgf y2) {
        return initial_normal(R2{x0, y0}, R3{x1, y1, w1}, R2{x2, y2});
    }

    SINK &sink(void) {
        return m_sink;
    }

    const SINK &sink(void) const {
        return m_sink;
    }

friend i_input_path<input_path_f_to_accelerated<SINK>>;

    void output_cap(e_stroke_cap cap, const R2 &p, const R2 &n) {
        switch (cap) {
            case e_stroke_cap::round:
                m_sink.round_cap(p[0], p[1], n[0], n[1]);
                break;
            case e_stroke_cap::square:
                m_sink.square_cap(p[0], p[1], n[0], n[1]);
                break;
            case e_stroke_cap::triangle:
                m_sink.triangle_cap(p[0], p[1], n[0], n[1]);
                break;
            default:
                break;
        }
    }

    void output_joins(R2 n0, rvgf x, rvgf y, R2 n1) {
        if (m_current_pen_down && !m_brand_new_contour) {
            if (dot(n0, perp(n1)) < 0) {
                // we are traversing an inner join
                // flip the normals as if we were traversing an outer join
                R2 t = -n0;
                n0 = -n1;
                n1 = t;
            }
            switch (m_style.get_join()) {
                case e_stroke_join::round:
                    sink().round_join(n0[0], n0[1], x, y, n1[0], n1[1]);
                    break;
                case e_stroke_join::bevel:
                    sink().bevel_join(n0[0], n0[1], x, y, n1[0], n1[1]);
                    break;
                case e_stroke_join::miter_clip:
                    sink().miter_clip_join(n0[0], n0[1], x, y, n1[0], n1[1]);
                    break;
                case e_stroke_join::miter_or_bevel: {
                    R2 c{x,y};
                    auto sq = [](rvgf a) -> rvgf { return a*a; };
                    auto d0 = perp(n0);
                    auto d1 = perp(n1);
                    auto q0 = c+0.5*m_width*n0;
                    auto q1 = c+0.5*m_width*n1;
                    auto v = intersection(q0, d0, q1, d1);
                    auto limit2 = sq(0.5*m_width*m_style.get_miter_limit());
                    if (sq(v.get_x()-c.get_x()*v.get_w()) +
                        sq(v.get_y()-c.get_y()*v.get_w()) <=
                        limit2*sq(v.get_w())) {
                        sink().miter_clip_join(n0[0], n0[1], x, y, n1[0], n1[1]);
                    } else {
                        sink().bevel_join(n0[0], n0[1], x, y, n1[0], n1[1]);
                    }
                    break;
                }
                default:
                    break;
            }
            if (m_style.get_inner_join() == e_stroke_join::round) {
                sink().round_join(-n0[0], -n0[1], x, y, -n1[0], -n1[1]);
            } else if (m_style.get_inner_join() == e_stroke_join::bevel) {
                sink().bevel_join(-n0[0], -n0[1], x, y, -n1[0], -n1[1]);
            }
        }
        m_brand_new_contour = false;
    }

    void do_begin_contour(rvgf x, rvgf y) {
        m_first_position = R2{x, y};
        m_brand_new_contour = true;
        if (m_dashing) {
            if (m_resets_on_move) {
                m_current_length_needed = m_initial_length_needed;
                m_current_dash = m_initial_dash;
                m_current_pen_down = m_initial_pen_down;
            }
            if (m_current_pen_down) {
                m_pen_down_at_begin_contour = true;
            } else {
                m_pen_down_at_begin_contour = false;
            }
        } else {
            m_pen_down_at_begin_contour = true;
        }
    }

    void do_end_open_contour(rvgf x, rvgf y) {
        // If the pen was down, draw the cap
        if (m_current_pen_down) {
            output_cap(m_style.get_terminal_cap(), R2{x,y},
                -m_final_normal);
        }
        // Draw initial cap that we were unable to when we begun the contour
        // because we didn't know yet that it was open
        if (m_pen_down_at_begin_contour &&
            m_style.get_initial_cap() != e_stroke_cap::butt) {
            output_cap(m_style.get_initial_cap(), m_first_position,
                m_first_normal);
        }
    }

    void do_end_closed_contour(rvgf x, rvgf y) {
        // If the pen was down when we begun and ended, add a join and butt cap
        if (m_current_pen_down && m_pen_down_at_begin_contour) {
            output_joins(m_final_normal, x, y, m_first_normal);
        // Otherwise, if started up but we ended down, add terminal_cap
        } else if (m_current_pen_down) {
            output_cap(m_style.get_terminal_cap(),
                R2{x,y}, -m_final_normal);
        // Otherwise, if we started down and ended up, add missing initial_cap
        } else if (m_pen_down_at_begin_contour &&
            m_style.get_initial_cap() != e_stroke_cap::butt) {
            output_cap(m_style.get_initial_cap(),
                m_first_position, m_first_normal);
        }
    }

    void set_normals(const R2 &n0, const R2 &n1) {
        m_previous_final_normal = m_final_normal;
        m_initial_normal = n0/len(n0);
        m_final_normal = n1/len(n1);
        if (m_brand_new_contour) {
            m_first_normal = m_initial_normal;
        }
    }

    template <typename ARCLEN, typename EVAL, typename PIECE>
    void process_dashes(const ARCLEN &a, const PIECE &piece, const EVAL &eval) {
        rvgf ti = 0;
        rvgf length = a.get_length();
        const auto &dashes = m_style.get_dashes();
        // Until segment is fully used
        while (length > 0) {
            // How much of current_length_needed can we satisfy with
            // the segment piece?
            rvgf now = std::min(length, m_current_length_needed);
            length -= now;
            // Snap to segment boundary if almost all used
            if (util::is_almost_zero(length)) {
                length = 0;
            }
            // Consume part of the dash (or space)
            m_current_length_needed -= now;
            // Snap to dash boundary if almost all consumed
            if (util::is_almost_zero(m_current_length_needed)) {
                m_current_length_needed = 0;
            }
            // If dash (or space) is complete, advance
            if (m_current_length_needed <= 0) {
                m_current_dash++;
                if (m_current_dash >= (int) dashes.size()) {
                    m_current_dash = 0;
                }
                m_current_length_needed = dashes[m_current_dash]*m_width;
                rvgf l = a.get_length();
                rvgf t = a.get_absolute_parameter(
                    a.get_relative_parameter_for_length_fraction((l-length)/l));
                if (m_current_pen_down) {
                    R2 p, d;
                    std::tie(p, d) = eval(t);
                    d = d/len(d);
                    piece(ti, t);
                    output_cap(m_style.get_dash_terminal_cap(), p, -perp(d));
                } else {
                    ti = t;
                    R2 p, d;
                    std::tie(p, d) = eval(t);
                    d = d/len(d);
                    output_cap(m_style.get_dash_initial_cap(), p, perp(d));
                }
                m_current_pen_down = !m_current_pen_down;
            }
        }
        if (m_current_pen_down) {
            piece(ti, 1);
        }
    }

    void do_linear_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1) {
        R2 dp = perp(R2{x1-x0, y1-y0});
        set_normals(dp, dp);
        output_joins(m_previous_final_normal, x0, y0, m_initial_normal);
        if (m_dashing) {
            auto a = make_linear_segment_arc_length<rvgf>(0, 1, R2{x0, y0},
                R2{x1, y1});
            process_dashes(a,
                [&](rvgf ti, rvgf tf) {
                    m_sink.linear_segment_piece(ti, tf, x0, y0, x1, y1);
            },  [&](rvgf t) {
                    return std::make_pair((1-t)*R2{x0,y0}+t*R2{x1,y1}, -perp(dp));
            });
        } else {
            return sink().linear_segment_piece(0, 1, x0, y0, x1, y1);
        }
    }

    void do_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2) {
        set_normals(
            initial_normal(x0, y0, x1, y1, x2, y2),
            -initial_normal(x2, y2, x1, y1, x0, y0)
        );
        output_joins(m_previous_final_normal, x0, y0, m_initial_normal);
        if (m_dashing) {
            auto s = std::make_tuple(R2{x0,y0}, R2{x1,y1}, R2{x2,y2});
            auto ds = bezier_derivative(s);
            auto d2s = bezier_derivative(ds);
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(0, 1,
                make_quadratic_segment_ds2_from_tuples<rvgf>(s, ds));
            process_dashes(a,
                [&](rvgf ti, rvgf tf) {
                    m_sink.quadratic_segment_piece(ti, tf, x0, y0, x1, y1,
                        x2, y2);
            },  [&](rvgf t) {
                    auto p = bezier_evaluate_horner<rvgf>(s, t);
                    auto d = tangent_direction(p,
                        bezier_evaluate_horner<rvgf>(ds, t),
                        bezier_evaluate_horner<rvgf>(d2s, t));
                    return std::make_pair(p, d);
            });
        } else {
            return sink().quadratic_segment_piece(0, 1, x0, y0, x1, y1, x2, y2);
        }
    }

    void do_rational_quadratic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf w1, rvgf x2, rvgf y2) {
        set_normals(
            initial_normal(x0, y0, x1, y1, w1, x2, y2),
            -initial_normal(x2, y2, x1, y1, w1, x0, y0)
        );
        output_joins(m_previous_final_normal, x0, y0, m_initial_normal);
        if (m_dashing) {
            auto s = std::make_tuple(R3{x0,y0,1}, R3{x1,y1,w1}, R3{x2,y2,1});
            auto ds = bezier_derivative(s);
            auto d2s = bezier_derivative(ds);
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(0, 1,
                make_rational_quadratic_segment_ds2_from_tuples<rvgf>(s, ds));
            process_dashes(a,
               [&](rvgf ti, rvgf tf) {
                    m_sink.rational_quadratic_segment_piece(ti, tf, x0, y0,
                        x1, y1, w1, x2, y2);
            }, [&](rvgf t) {
                auto p = bezier_evaluate_horner<rvgf>(s, t);
                auto d = tangent_direction(p,
                    bezier_evaluate_horner<rvgf>(ds, t),
                    bezier_evaluate_horner<rvgf>(d2s, t));
                return std::make_pair(project<R2>(p), d);
            });
        } else {
            return sink().rational_quadratic_segment_piece(0, 1, x0, y0, x1, y1,
                w1, x2, y2);
        }
    }

    void do_cubic_segment(rvgf x0, rvgf y0, rvgf x1, rvgf y1,
        rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        set_normals(
            initial_normal(x0, y0, x1, y1, x2, y2, x3, y3),
            -initial_normal(x3, y3, x2, y2, x1, y1, x0, y0)
        );
        output_joins(m_previous_final_normal, x0, y0, m_initial_normal);
        if (m_dashing) {
            auto s = std::make_tuple(R2{x0,y0},R2{x1,y1},R2{x2,y2},R2{x3,y3});
            auto ds = bezier_derivative(s);
            auto d2s = bezier_derivative(ds);
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(0, 1,
                make_cubic_segment_ds2_from_tuples<rvgf>(s, ds));
            process_dashes(a, [&](rvgf ti, rvgf tf) {
                m_sink.cubic_segment_piece(ti, tf, x0, y0, x1, y1, x2, y2,
                    x3, y3);
            },  [&](rvgf t) {
                    auto p = bezier_evaluate_horner<rvgf>(s, t);
                    auto d = tangent_direction(p,
                        bezier_evaluate_horner<rvgf>(ds, t),
                        bezier_evaluate_horner<rvgf>(d2s, t));
                    return std::make_pair(p, d);
            });
        } else {
            return sink().cubic_segment_piece(0, 1, x0, y0, x1, y1, x2, y2,
                x3, y3);
        }
    }
};

template <typename SINK>
auto make_input_path_f_to_accelerated(
    rvgf width, const stroke_style &style, SINK &&sink) {
    return input_path_f_to_accelerated<SINK>{width, style,
        std::forward<SINK>(sink)};
}

template <typename SINK>
class scene_f_accelerate final:
	public i_scene_data<scene_f_accelerate<SINK>> {

    std::vector<xform> m_xf_stack;
	SINK m_sink;

    SINK &sink(void) {
        return m_sink;
    }

    const SINK &sink(void) const {
        return m_sink;
    }

public:
    scene_f_accelerate(const xform &xf, SINK &&sink):
        m_xf_stack{{xf}},
        m_sink(std::forward<SINK>(sink)) { ; }

private:

    const xform &top_xf(void) const {
        return m_xf_stack.back();
    }

	void push_xf(const xform &xf) {
		m_xf_stack.push_back(top_xf() * xf);
	}

	void pop_xf(void) {
		if (m_xf_stack.size() > 0) {
			m_xf_stack.pop_back();
		}
	}

    friend i_scene_data<scene_f_accelerate<SINK>>;

    void do_painted_shape(e_winding_rule, const shape &s, const paint &p) {
        if (s.is_stroke() && p.is_solid_color()) {
            const auto &sh = s.get_stroke_data().get_shape();
            sink().color(pre_multiply(p.get_solid_color())*p.get_opacity());
            auto w = s.get_stroke_data().get_width();
            sink().width(w);
            const auto &st = s.get_stroke_data().get_style();
            if (st.get_join() == e_stroke_join::miter_clip ||
                st.get_join() == e_stroke_join::miter_or_bevel) {
                sink().miter_limit(st.get_miter_limit());
            }
            sink().transform(s.get_xf().transformed(top_xf()).inverse());
            sh.as_path_data_ptr()->iterate(
                make_input_path_f_close_contours(
                    make_input_path_f_xform(sh.get_xf(),
                        make_input_path_f_to_accelerated(w, st, sink()))));
        }
    }

    void do_tensor_product_patch(const patch<16,4> &) {
    }

    void do_coons_patch(const patch<12,4> &) {
    }

    void do_gouraud_triangle(const patch<3,3> &) {
    }

    void do_stencil_shape(e_winding_rule, const shape &) {
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
    }

    void do_begin_fade(uint16_t depth, unorm8 opacity) {
        (void) depth;
        (void) opacity;
    }

    void do_end_fade(uint16_t depth, unorm8 opacity) {
        (void) depth;
        (void) opacity;
    }

    void do_begin_blur(uint16_t depth, rvgf radius) {
        (void) depth;
        (void) radius;
    }

    void do_end_blur(uint16_t depth, rvgf radius) {
        (void) depth;
        (void) radius;
    }

    void do_begin_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        push_xf(xf);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        (void) depth;
        (void) xf;
        pop_xf();
    }
};

template <typename SINK>
scene_f_accelerate<SINK> make_scene_f_accelerate(const xform &xf, SINK &&sink) {
    return scene_f_accelerate<SINK>{xf, std::forward<SINK>(sink)};
}

accelerated accelerate(const scene &c, const window &w, const viewport &v) {
    accelerated a;
    c.get_scene_data().iterate(
        make_scene_f_accelerate(
            make_windowviewport(w, v)*c.get_xf(), a));
    return a;
}

class accelerated_f_sample_color final:
    public i_accelerated<accelerated_f_sample_color> {

    rvgf m_sx, m_sy; // screen coordinates
    rvgf m_tx, m_ty; // current transformed coordinates
    rvgf m_m;        // current miter limit
    double m_hw, m_hw2;  // current half stroke width and its square
    RGBA8 m_fg;       // current foreground color
    RGBA8 &m_c;       // running color
    bool m_blended;   // already blended

    constexpr static rvgf m_eps = 0.0000152587890625;

public:
    accelerated_f_sample_color(rvgf sx, rvgf sy, RGBA8 &c):
       m_sx(sx), m_sy(sy), m_c(c) {
    }

private:
    friend i_accelerated<accelerated_f_sample_color>;

    void do_color(RGBA8 c) {
        m_fg = c;
        m_blended = false;
    }

    void do_transform(const xform &xf) {
        std::tie(m_tx, m_ty, std::ignore) = xf.apply(m_sx, m_sy);
    }

    void do_width(rvgf w) {
        m_hw = .5*w;
        m_hw2 = m_hw*m_hw;
    }

    void do_miter_limit(rvgf m) {
        m_m = m;
    }

    void blend(void) {
        m_c = over(m_fg, m_c);
        m_blended = true;
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf p0, rvgf q0,
        rvgf p1, rvgf q1) {
        if (m_blended) return;
        double x0 = p0, x1 = p1;
        double y0 = q0, y1 = q1;
        x0 -= m_tx;
        x1 -= m_tx;
        y0 -= m_ty;
        y1 -= m_ty;
        double u = (-x0)*(x1-x0) + (-y0)*(y1-y0);
        double v = (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
        int s = util::sgn(v);
        u *= s;
        v *= s;
        if (u >= v*(ti+m_eps) && u <= v*(tf-m_eps)) {
            double t = u/v;
            double x = x0+t*(x1-x0);
            double y = y0+t*(y1-y0);
            if (x*x + y*y <= m_hw2) {
                blend();
            }
        }
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        if (m_blended) return;
        using namespace boost::adaptors;
        auto x = tuple_map(
            std::make_tuple(x0, x1, x2),
            [this](double a) { return a - m_tx; }
        );
        auto y = tuple_map(
            std::make_tuple(y0, y1, y2),
            [this](double a) { return a - m_ty; }
        );
        auto dx = bezier_derivative(x);
        auto dy = bezier_derivative(y);
        auto xdx = bezier_product<double>(x, dx);
        auto ydy = bezier_product<double>(y, dy);
        auto p = tuple_zip_map(xdx, ydy, [](double a, double b) {
            return a + b;
        });
        auto ts = bezier_roots<double>(p, ti, tf);
        for (auto t: ts | sliced(1, ts.size()-1)) {
            double xt = bezier_evaluate_horner<double>(x, t);
            double yt = bezier_evaluate_horner<double>(y, t);
            if (t > m_eps && t < 1.f-m_eps && xt*xt + yt*yt <= m_hw2) {
                blend();
                break;
            }
        }
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        if (m_blended) return;
        using namespace boost::adaptors;
        auto x = tuple_map(
            std::make_tuple(x0, x1, x2, x3),
            [this](double a) { return a - m_tx; }
        );
        auto y = tuple_map(
            std::make_tuple(y0, y1, y2, y3),
            [this](double a) { return a - m_ty; }
        );
        auto dx = bezier_derivative(x);
        auto dy = bezier_derivative(y);
        auto xdx = bezier_product<double>(x, dx);
        auto ydy = bezier_product<double>(y, dy);
        auto p = tuple_zip_map(xdx, ydy, [](double a, double b) {
            return a + b;
        });
        auto ts = bezier_roots<double>(p, ti,  tf);
        for (auto t: ts | sliced(1, ts.size()-1)) {
            double xt = bezier_evaluate_horner<double>(x, t);
            double yt = bezier_evaluate_horner<double>(y, t);
            if (t > m_eps && t < 1.f-m_eps && xt*xt + yt*yt <= m_hw2) {
                blend();
                break;
            }
        }
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf,
        rvgf x0, rvgf y0, rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        (void) ti; (void) tf;
        if (m_blended) return;
        using namespace boost::adaptors;
        auto w = std::make_tuple(1., static_cast<double>(w1), 1.);
        auto x = tuple_zip_map(
            std::make_tuple(x0, x1, x2),
            w,
            [this](double a, double b) { return a - b*m_tx; }
        );
        auto y = tuple_zip_map(
            std::make_tuple(y0, y1, y2),
            w,
            [this](double a, double b) { return a - b*m_ty; }
        );
        auto dx = bezier_derivative(x);
        auto dy = bezier_derivative(y);
        auto dw = bezier_derivative(w);
        auto wdx_xdw = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<double>(w, dx),
                bezier_product<double>(x, dw),
                std::minus<double>()
            )
        );
        auto wdy_ydw = bezier_lower_degree(
            tuple_zip_map(
                bezier_product<double>(w, dy),
                bezier_product<double>(y, dw),
                std::minus<double>()
            )
        );
        auto p = tuple_zip_map(
            bezier_product<double>(x, wdx_xdw),
            bezier_product<double>(y, wdy_ydw),
            std::plus<double>()
        );
        auto ts = bezier_roots<double>(p, ti, tf);
        for (auto t: ts | sliced(1, ts.size()-1)) {
            double xt = bezier_evaluate_horner<double>(x, t);
            double yt = bezier_evaluate_horner<double>(y, t);
            double wt = bezier_evaluate_horner<double>(w, t);
            if (t > m_eps && t < 1.f-m_eps && xt*xt + yt*yt <= m_hw2*wt*wt) {
                blend();
                break;
            }
        }
    }

    void do_round_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
        rvgf nx1, rvgf ny1) {
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (u*u + v*v <= m_hw2 &&
            -nx0*v + ny0*u > 0. &&
            -nx1*v + ny1*u <= 0.) {
            blend();
        }
    }

    void do_bevel_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
        rvgf nx1, rvgf ny1) {
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (-nx0*v + ny0*u > 0. && -nx1*v + ny1*u <= 0.) {
            u -= nx0*m_hw;
            v -= ny0*m_hw;
            double dx = (nx1-nx0)*m_hw;
            double dy = (ny1-ny0)*m_hw;
            if (-u*dy + v*dx < 0.)
                blend();
        }
    }

    void do_miter_clip_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y,
        rvgf nx1, rvgf ny1) {
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (-nx0*v + ny0*u > 0. && -nx1*v + ny1*u <= 0.) {
            double p = u - nx0*m_hw;
            double q = v - ny0*m_hw;
            double r = u - nx1*m_hw;
            double s = v - ny1*m_hw;
            if (p*nx0 + q*ny0 < 0. && r*nx1 + s*ny1 < 0.) {
                R2 b = clockwise_bisector(R2{nx0,ny0}, R2{nx1,ny1});
                u -= b.get_x()*m_hw*m_m;
                v -= b.get_y()*m_hw*m_m;
                if (u*b.get_x() + v*b.get_y() < 0.)
                    blend();
            }
        }
    }

    void do_round_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (u*u + v*v <= m_hw2 && -nx*v + ny*u < 0.) {
            blend();
        }
    }

    void do_square_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
        (void) x; (void) y; (void) nx; (void) ny;
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (-nx*v + ny*u < 0 && -nx*v + ny*u > -m_hw &&
            nx*u + ny*v < m_hw && nx*u + ny*v > -m_hw) {
            blend();
        }
    }

    void do_triangle_cap(rvgf x, rvgf y, rvgf nx, rvgf ny) {
        (void) x; (void) y; (void) nx; (void) ny;
        if (m_blended) return;
        double u = m_tx - x;
        double v = m_ty - y;
        if (-nx*v + ny*u < 0
            && (nx+ny)*u + (ny-nx)*v > -m_hw
            && (nx-ny)*u + (nx+ny)*v < m_hw) {
            blend();
        }
    }
};

RGBA8 sample(const accelerated &a, rvgf x, rvgf y, RGBA8 bg) {
    a.iterate(accelerated_f_sample_color{x, y, bg});
    return bg;
}

void render(const accelerated &a, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &) {
    (void) w;
    int xl, yb, xr, yt;
    static constexpr rvgf ox[] = {
		0.441953f,0.216327f,0.350296f,-0.018187f,
		0.495827f,-0.4154813f,0.11729f,-0.02673f,
		-0.380037f,-0.22633f,0.064872f,0.227373f,
		-0.330074f,-0.171254f,0.342183f,-0.158753f,
    };
    static constexpr rvgf oy[] = {
		0.232335f,-0.22118f,-0.4544032f,-0.138588f,
		-0.218711f,-0.4654322f,0.090573f,0.308073f,
		-0.004619f,-0.243974f,-0.4197364f,0.339616f,
		0.281552f,-0.49681762f,-0.0005880f,0.081051f,
    };
	static constexpr int n = static_cast<int>(sizeof(ox)/sizeof(ox[0]));
	static_assert(n == static_cast<int>(sizeof(oy)/sizeof(oy[0])),
		"invalid supersampling pattern");
    std::tie(xl, yb) = v.bl();
    std::tie(xr, yt) = v.tr();
    int vxmin = std::min(xl,xr);
    int vxmax = std::max(xl,xr);
    int vymin = std::min(yb,yt);
    int vymax = std::max(yb,yt);
    int width = vxmax-vxmin;
    int height = vymax-vymin;
    image<uint8_t, 4> buf;
    buf.resize(width, height);
#pragma omp parallel for
    for (int i = 0; i < height; ++i) {
        rvgf y = vymin+i+0.5f;
        for (int j = 0; j < width; ++j) {
            rvgf x = vxmin+j+0.5f;
			RGBA<uint16_t> sc;
			for (int s = 0; s < n; ++s) {
				sc += remove_gamma(
                    post_divide(
                        sample(a, x+ox[s], y+oy[s],
                            make_rgba8(255, 255, 255, 255))));
			}
			auto c = add_gamma(RGBA8{sc/n});
			buf.set_pixel(j, i, c[0], c[1], c[2], c[3]);
        }
    }
    store_png<uint8_t>(out, buf);
}

} } } // namespace rvg::driver::distroke

using namespace rvg::driver::distroke;

// Lua version of the rvg::driver::distroke::accelerate function
static int luaaccelerate(lua_State *L) {
    rvg_lua_push<accelerated>(L, accelerate(
            rvg_lua_check<rvg::scene>(L, 1),
            rvg_lua_check<rvg::window>(L, 2),
            rvg_lua_check<rvg::viewport>(L, 3)));
    return 1;
}

// Lua version of the render function
static int luarender(lua_State *L) {
    auto a = rvg_lua_check<accelerated>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    FILE *f = rvg_lua_check_file(L, 4);
    render(a, w, v, f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg moddistroke[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.distroke"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_distroke(lua_State *L) {
    rvg_lua_init(L); // mettab
    rvg_lua_createtype<accelerated>(L, "distroke accelerated", -1);
    // mettab
    rvg_lua_facade_new_driver(L, moddistroke); // mettab driver
    return 1;
}
