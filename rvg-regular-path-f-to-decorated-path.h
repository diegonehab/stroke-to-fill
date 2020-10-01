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
#ifndef RVG_REGULAR_PATH_F_TO_DECORATED_PATH_H
#define RVG_REGULAR_PATH_F_TO_DECORATED_PATH_H

#include "rvg-i-regular-path.h"
#include "rvg-i-decorated-path.h"
#include "rvg-i-sink.h"
#include "rvg-i-parameters-f-hold.h"
#include "rvg-i-point-regular-path.h"
#include "rvg-util.h"
#include "rvg-bezier-arc-length.h"
#include "rvg-stroke-style.h"

namespace rvg {

// This module is responsible for converting a regular path
// to something that can be stroked.
//
// First, consider a path without dashing.
//
// When start processing an input contour, we have no idea
// if the contour will be open or closed until we reach the
// end. So we start the contour by replacing a
// begin_regular_contour with a initial_butt_cap.
//
// Segments are passed along, and a join is added them.
// (These may be be eliminated or replaced by inner_join by
// later filters.)
//
// If we eventually find an end_regular_closed_contour, we
// simply add a join that makes the orientation match the
// one in the beginning of the contour, and finish with
// terminal_butt_cap.
//
// However, if we find an end_regular_open_contour, we have
// to add the initial_cap we didn't add when we processed
// begin_regular_contour. So we finish the output contour
// with an terminal_cap, and then create a new contour containing
// just a initial_cap and an terminal_butt_cap.
//
// There are no begin_regular_contour,
// end_regular_open_contour or end_regular_closed_contour
// left.
//
// Now consider the case of dashing.
//
// The dashing process maintains a Boolean value
// current_pen_down that is true when we are inside a
// dash, and false when we are outside.
//
// If the pen is down when we find a begin_regular_contour,
// we replace it with a initial_butt_cap.  If, however, a
// contour begins when the pen is up, we simply keep going.
// Who knows when the pen will be down again?
//
// Segment pieces that go by while the pen is up are skipped
// entirely.
//
// Dashing itself is implemented by begin_dash_parameter and
// end_dash_parameter. For each dash that ends or starts
// inside a segment piece, there will be a corresponding
// end_dash_parameter or begin_dash_parameter, respectively.
//
// If a segment pice appears while the pen is down, we add a
// join to the previous segment. If the pen was up, we
// don't add the join. Regardless, we then add all the
// begin_dash_parameter and end_dash_parameters for the
// segment piece needed. If any part of the segment was
// visible, we then add the segment piece itself.
//
// When we find an end_regular_open_contour while the pen is
// down, we add an terminal_cap. If the pen was up, we simply
// ignore it.  But we check if the pen was down when we
// procesed the previous begin_regular_contour. If so, we
// add a new contour with a initial_cap and terminal_butt_cap.
//
// If we instead find an end_regular_closed_contour, we check if the
// pen was down when the contour started and ended, and add the
// join in that case. Otherwise, if the pen is currently
// down, we add an terminal_cap. Otherwise, if the pen was down
// when the contour started, we add a new contour with initial_cap and
// terminal_butt_cap.

template <typename SINK>
class regular_path_f_to_decorated_path final:
    public i_sink<regular_path_f_to_decorated_path<SINK>>,
    public i_parameters_f_hold<regular_path_f_to_decorated_path<SINK>>,
    public i_point_regular_path<regular_path_f_to_decorated_path<SINK>> {

    SINK m_sink;
    rvgf m_width;
    stroke_style::const_ptr m_style;
    R2 m_first_p, m_first_d;
    R2 m_pf, m_df;

    bool m_dashing;
    bool m_initial_pen_down, m_current_pen_down;
    rvgf m_initial_length_needed, m_current_length_needed;
    int m_initial_dash, m_current_dash;
    bool m_resets_on_move, m_pen_down_at_begin_contour;

public:

    regular_path_f_to_decorated_path(rvgf width,
        stroke_style::const_ptr style, SINK &&sink):
        m_sink{std::forward<SINK>(sink)},
        m_width{width},
        m_style{style},
        m_dashing{!style->get_dashes().empty()},
        m_initial_pen_down{true}, m_current_pen_down{true},
        m_initial_length_needed{0}, m_current_length_needed{0},
        m_initial_dash{0}, m_current_dash{0},
        m_resets_on_move{style->get_resets_on_move()},
        m_pen_down_at_begin_contour{true} {

        static_assert(
            meta::is_an_i_decorated_path<SINK>::value &&
            meta::is_an_i_regular_path<SINK>::value,
            "sink is not both i_regular_path and i_decorated_path");

        if (m_dashing) {
            const auto &dashes = m_style->get_dashes();
            rvgf length{0};
            for (rvgf d: dashes) {
                length += d*m_width;
            }
            rvgf offset = std::fmod(m_style->get_dash_offset()*m_width,
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

friend i_sink<regular_path_f_to_decorated_path<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

friend i_point_regular_path<regular_path_f_to_decorated_path<SINK>>;

    void do_begin_regular_contour(const R2 &p, const R2 &d) {
        if (m_dashing) {
            if (m_resets_on_move) {
                m_current_length_needed = m_initial_length_needed;
                m_current_dash = m_initial_dash;
                m_current_pen_down = m_initial_pen_down;
            }
            m_first_p = m_pf = p;
            m_first_d = m_df = d;
            if (m_current_pen_down) {
                // We can't draw the start cap yet, because
                // the contour may be closed. In the
                // end_contour method, we know if we should
                // add the cap of simply connect with a join.
                m_pen_down_at_begin_contour = true;
                m_sink.initial_butt_cap(p, d);
            } else {
                // Here we don't even know if we need this
                // initial control point, because it may be
                // several segments in the future before we
                // put our pen down, if at all
                m_pen_down_at_begin_contour = false;
            }
        } else {
            m_first_p = m_pf = p;
            m_first_d = m_df = d;
            m_sink.initial_butt_cap(p, d);
            m_pen_down_at_begin_contour = true;
        }
    }

    void do_end_regular_open_contour(const R2 &d, const R2 &p) {
        // If the pen was down, draw the cap
        if (m_current_pen_down) {
            m_sink.terminal_cap(d, p);
        }
        // Draw initial cap that we were unable to when we begun the contour
        // because we didn't know yet that it was open
        if (m_pen_down_at_begin_contour &&
            m_style->get_initial_cap() != e_stroke_cap::butt) {
            m_sink.initial_cap(m_first_p, m_first_d);
            m_sink.terminal_butt_cap(m_first_d, m_first_p);
        }
    }

    void do_end_regular_closed_contour(const R2 &d, const R2 &p) {
        assert(m_first_p == p);
        // If the pen was down when we begun and ended, add a join and butt cap
        if (m_current_pen_down && m_pen_down_at_begin_contour) {
            m_sink.join(d, p, m_first_d, 0);
            m_sink.terminal_butt_cap(m_first_d, p);
        // Otherwise, if started up but we ended down, add terminal_cap
        } else if (m_current_pen_down) {
            m_sink.terminal_cap(d, p);
        // Otherwise, if we started down and ended up, add missing initial_cap
        } else if (m_pen_down_at_begin_contour &&
            m_style->get_initial_cap() != e_stroke_cap::butt) {
            m_sink.initial_cap(m_first_p, m_first_d);
            m_sink.terminal_butt_cap(m_first_d, m_first_p);
        }
    }

    void do_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        if (m_current_pen_down) {
            m_sink.cusp(di, p, df, w);
            m_df = df;
            m_pf = p;
        }
    }

    void do_inner_cusp(const R2 &di, const R2 &p, const R2 &df, rvgf w) {
        if (m_current_pen_down) {
            m_sink.inner_cusp(di, p, df, w);
            m_df = df;
            m_pf = p;
        }
    }

    void do_degenerate_segment(const R2 &pi, const R2 &d, const R2 &pf) {
        if (m_current_pen_down) {
            m_sink.join(m_df, pi, d, 0);
            m_df = d;
            m_pf = pf;
            return m_sink.degenerate_segment(pi, d, pf);
        }
    }

    template <typename ARCLEN>
    bool visible(const ARCLEN &a) {
        return m_current_pen_down || m_current_length_needed < a.get_length();
    }

    template <typename ARCLEN>
    void process_dashes(const ARCLEN &a) {
        rvgf length = a.get_length();
        const auto &dashes = m_style->get_dashes();
        // Until segment is fully used
        rvgf last_t{0};
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
                // Make sure output parameters are in increasing order
                t = std::max(t, std::nextafter(last_t, 2.f));
                last_t = t;
                if (m_current_pen_down) {
                    m_sink.end_dash_parameter(t);
                } else {
                    m_sink.begin_dash_parameter(t);
                }
                m_current_pen_down = !m_current_pen_down;
            }
        }
    }

    template <typename ARCLEN, typename PIECE>
    void process_dashed_segment_piece(const ARCLEN &a, const PIECE &piece) {
        // If segment has a visible piece, dash() will
        // output the parameters where dashes start and end
        // If segment has no visible part, dash_segment will
        // simply consume the segment length advancing the
        // dashing pattern
        bool v = visible(a);
        process_dashes(a);
        if (v) {
            piece();
        }
    }

    void do_begin_segment_piece(const R2 &pi, const R2 &di) {
        if (m_current_pen_down) {
            // We leave the decision of inner/outer/omit to the
            // simplify joins filter
            m_sink.join(m_df, pi, di, 0);
        }
    }

    void do_end_segment_piece(const R2 &df, const R2 &pf) {
        m_pf = pf;
        m_df = df;
    }

    void do_linear_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1) {
        if (m_dashing) {
            auto a = make_linear_segment_arc_length<rvgf>(ti, tf, p0, p1);
            process_dashed_segment_piece(a, [&](void) {
                this->forward_parameters();
                this->clear_parameters();
                m_sink.linear_segment_piece(ti, tf, p0, p1);
            });
        } else {
            this->forward_parameters();
            this->clear_parameters();
            m_sink.linear_segment_piece(ti, tf, p0, p1);
        }
    }

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, const R2 &p0,
        const R2 &p1, const R2 &p2) {
        if (m_dashing) {
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(ti, tf,
                make_quadratic_segment_ds2<rvgf>(p0, p1, p2));
            process_dashed_segment_piece(a, [&](void) {
                this->forward_parameters();
                this->clear_parameters();
                m_sink.quadratic_segment_piece(ti, tf, p0, p1, p2);
            });
        } else {
            this->forward_parameters();
            this->clear_parameters();
            m_sink.quadratic_segment_piece(ti, tf, p0, p1, p2);
        }
    }

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, const R3 &p0,
        const R3 &p1, const R3 &p2) {
        if (m_dashing) {
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(ti, tf,
                make_rational_quadratic_segment_ds2<rvgf>(p0, p1, p2));
            process_dashed_segment_piece(a, [&](void) {
                this->forward_parameters();
                this->clear_parameters();
                m_sink.rational_quadratic_segment_piece(ti, tf, p0, p1, p2);
            });
        } else {
            this->forward_parameters();
            this->clear_parameters();
            m_sink.rational_quadratic_segment_piece(ti, tf, p0, p1, p2);
        }
    }

    void do_cubic_segment_piece(rvgf ti, rvgf tf, const R2 &p0, const R2 &p1,
        const R2 &p2, const R2 &p3) {
        if (m_dashing) {
            auto a = make_arc_length<rvgf, RVG_ARC_LENGTH_INTERVALS>(ti, tf,
                make_cubic_segment_ds2<rvgf>(p0, p1, p2, p3));
            process_dashed_segment_piece(a, [&](void) {
                this->forward_parameters();
                this->clear_parameters();
                m_sink.cubic_segment_piece(ti, tf, p0, p1, p2, p3);
            });
        } else {
            this->forward_parameters();
            this->clear_parameters();
            m_sink.cubic_segment_piece(ti, tf, p0, p1, p2, p3);
        }
    }

};

template <typename SINK>
static auto make_regular_path_f_to_decorated_path(rvgf width,
    stroke_style::const_ptr style, SINK &&sink) {
    return regular_path_f_to_decorated_path<SINK>(width, style,
        std::forward<SINK>(sink));
}

} // namespace rvg

#endif
