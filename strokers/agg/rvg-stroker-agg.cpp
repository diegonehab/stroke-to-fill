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
#include <cmath>
#include <cstdint>
#include <iostream>

#include <agg_conv_stroke.h>
#include <agg_conv_curve.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>

#include "rvg-stroker-agg.h"

#include "rvg-xform.h"
#include "rvg-xform-svd.h"
#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"

namespace rvg {
    namespace stroker {

class input_path_f_to_agg_path_storage final:
    public i_input_path<input_path_f_to_agg_path_storage> {

    agg::path_storage &m_aggpath;

public:
	explicit input_path_f_to_agg_path_storage(agg::path_storage &aggpath):
        m_aggpath(aggpath) { ; }

private:

friend i_input_path<input_path_f_to_agg_path_storage>;

    void do_begin_contour(float x0, float y0) {
		m_aggpath.move_to(x0, y0);
	}

    void do_end_open_contour(float x0, float y0) {
		(void) x0; (void) y0;
		// do nothing
	}

    void do_end_closed_contour(float x0, float y0) {
		(void) x0; (void) y0;
		m_aggpath.end_poly(agg::path_flags_close);
	}

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
		m_aggpath.line_to(x1, y1);
	}

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		(void) x0; (void) y0;
		m_aggpath.curve3(x1, y1, x2, y2);
	}

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
        (void) x0; (void) y0; (void) x1; (void) y1;
        (void) w1; (void) x2; (void) y2;
        // should never be called
	}

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
		m_aggpath.curve4(x1, y1, x2, y2, x3, y3);
	}
};

inline auto make_input_path_f_to_agg_path_storage(agg::path_storage &output) {
	return input_path_f_to_agg_path_storage{output};
}

template <typename T> void
set_style(float width, stroke_style::const_ptr style, T& stroke) {
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
			stroke.line_cap(agg::butt_cap);
            break;
        case e_stroke_cap::round:
			stroke.line_cap(agg::round_cap);
            break;
        case e_stroke_cap::square:
			stroke.line_cap(agg::square_cap);
            break;
        default:
            break;
    }
    switch (style->get_join()) {
        case e_stroke_join::bevel:
			stroke.line_join(agg::bevel_join);
            break;
        case e_stroke_join::round:
			stroke.line_join(agg::round_join);
            break;
        case e_stroke_join::miter_clip:
			stroke.line_join(agg::miter_join);
            break;
        case e_stroke_join::miter_or_bevel:
			stroke.line_join(agg::miter_join_revert);
            break;
        default:
            break;
    }

    switch (style->get_inner_join()) {
        case e_stroke_join::bevel:
			stroke.inner_join(agg::inner_bevel);
            break;
        case e_stroke_join::miter_clip:
        case e_stroke_join::miter_or_bevel:
			stroke.inner_join(agg::inner_miter);
            break;
        case e_stroke_join::round:
        default:
			stroke.inner_join(agg::inner_round);
            break;
    }

    stroke.miter_limit(style->get_miter_limit());

	stroke.width(width);
}

template <typename T>
void set_dash(float width, stroke_style::const_ptr style, T &aggdash) {
    const auto &dashes = style->get_dashes();
    if (!dashes.empty()) {
        for (int i = 0; i+1 < (int) dashes.size(); i+=2) {
            aggdash.add_dash(dashes[i]*width, dashes[i+1]*width);
        }
        if (dashes.size() % 2 == 1) {
            aggdash.add_dash(dashes.back()*width, dashes.front()*width);
            for (int i = 1; i+1 < (int) dashes.size(); i+=2) {
                aggdash.add_dash(dashes[i]*width, dashes[i+1]*width);
            }
        }
    }
    aggdash.dash_start(style->get_dash_offset()*width);
}

template <typename T>
void read_back(T &stroke, path_data &output_path) {
    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(
            output_path));
    double x[4], y[4];
    int nvertices = 0;
    int ncurves = 0;
	int done = 0;
    int first = 1;
    while (!done) {
        auto cmd = stroke.vertex(&x[nvertices % 4], &y[nvertices % 4]);
        nvertices++;
        switch (cmd) {
            case agg::path_cmd_stop:
				done = 1;
                converter.close_path();
                break;
            case agg::path_cmd_move_to:
                if (!first) {
                    converter.close_path();
                }
                first = 0;
                converter.move_to_abs(x[(nvertices-1) % 4],
					y[(nvertices-1) % 4]);
                break;
            case agg::path_cmd_line_to:
                converter.line_to_abs(
                    x[(nvertices-1) % 4], y[(nvertices-1) % 4]);
                break;
            case agg::path_cmd_curve3:
                ncurves++;
                if (ncurves == 2) {
					converter.quad_to_abs(
                        x[(nvertices-2) % 4], y[(nvertices-2) % 4],
                        x[(nvertices-1) % 4], y[(nvertices-1) % 4]);
                    ncurves = 0;
                }
                break;
            case agg::path_cmd_curve4:
                ncurves++;
                if (ncurves == 3) {
                    converter.cubic_to_abs(
                        x[(nvertices-3) % 4], y[(nvertices-3) % 4],
                        x[(nvertices-2) % 4], y[(nvertices-2) % 4],
                        x[(nvertices-1) % 4], y[(nvertices-1) % 4]);
                    ncurves = 0;
                }
                break;
            default:
                break;
        }
	}
}

float xf_scale(const xform &xf) {
    linearity A(xf[0][0], xf[0][1], xf[1][0], xf[1][1]);
    rotation U;
    scaling S;
    asvd(A, U, S);
    return std::max(std::fabs(S.get_sx()), std::fabs(S.get_sy()));
}

shape agg(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {
	agg::path_storage agg_input_path;
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_input_path_f_to_agg_path_storage(agg_input_path))));
	agg::conv_curve<decltype(agg_input_path)> curve(agg_input_path);
	curve.approximation_method(agg::curve_div); // agg::curve_inc
    curve.approximation_scale(xf_scale(screen_xf));
    curve.angle_tolerance(0.2);
//    curve.angle_tolerance(100);
    auto output_path = make_intrusive<path_data>();
	if (style->get_dashes().empty()) {
		agg::conv_stroke<decltype(curve)> stroke(curve);
		set_style(width, style, stroke);
		read_back(stroke, *output_path);
	} else {
		agg::conv_dash<decltype(curve)> dash(curve);
		set_dash(width, style, dash);
		agg::conv_stroke<decltype(dash)> stroke(dash);
		set_style(width, style, stroke);
		read_back(stroke, *output_path);
	}

    return shape{output_path};
}

} }
