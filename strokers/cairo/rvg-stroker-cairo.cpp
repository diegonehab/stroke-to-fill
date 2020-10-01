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
#include <cairo.h>
#include <cairoint.h>
#include <cairo-traps-private.h>
#include <cairo-tristrip-private.h>

#include <map>

#include "rvg-input-path-f-rational-quadratic-to-cubics.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-svg-path-f-to-input-path.h"

#include "rvg-stroker-cairo.h"

namespace rvg {
    namespace stroker {

using closed_path = input_path_f_close_contours<path_data &, false>;
using svg_closed_path = svg_path_f_to_input_path<closed_path>;

class input_path_f_to_cairo_path_fixed final:
    public i_input_path<input_path_f_to_cairo_path_fixed> {

    cairo_path_fixed_t *m_path;

public:
    input_path_f_to_cairo_path_fixed(cairo_path_fixed_t *path): m_path(path) { }

private:

friend i_input_path<input_path_f_to_cairo_path_fixed>;

    void do_begin_contour(float x0, float y0) {
        _cairo_path_fixed_move_to(m_path,
            _cairo_fixed_from_double(x0),
            _cairo_fixed_from_double(y0));
    }

    void do_end_open_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
    }

    void do_end_closed_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        _cairo_path_fixed_close_path(m_path);
    }

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
		_cairo_path_fixed_line_to(m_path,
            _cairo_fixed_from_double(x1),
            _cairo_fixed_from_double(y1));
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		// elevate quadratic to cubic
		float u1 = (x0+2.f*x1)/3.f, v1 = (y0+2.f*y1)/3.f;
		float u2 = (x2+2.f*x1)/3.f, v2 = (y2+2.f*y1)/3.f;
		_cairo_path_fixed_curve_to(m_path,
            _cairo_fixed_from_double(u1),
            _cairo_fixed_from_double(v1),
            _cairo_fixed_from_double(u2),
            _cairo_fixed_from_double(v2),
            _cairo_fixed_from_double(x2),
            _cairo_fixed_from_double(y2));
    }

    void do_rational_quadratic_segment(float, float, float, float, float,
        float, float) {
        // should never be called
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
		_cairo_path_fixed_curve_to(m_path,
            _cairo_fixed_from_double(x1),
            _cairo_fixed_from_double(y1),
            _cairo_fixed_from_double(x2),
            _cairo_fixed_from_double(y2),
            _cairo_fixed_from_double(x3),
            _cairo_fixed_from_double(y3));
    }
};

inline input_path_f_to_cairo_path_fixed make_input_path_f_to_cairo_path_fixed(
    cairo_path_fixed_t *path) {
    return input_path_f_to_cairo_path_fixed{path};
}

void init_cairo_path_fixed(const shape &input_shape, const xform &screen_xf,
	cairo_path_fixed_t *cairo_path) {
    _cairo_path_fixed_init(cairo_path);
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_input_path_f_to_cairo_path_fixed(cairo_path))));
}

void init_cairo_stroke_style(float width, stroke_style::const_ptr style,
    cairo_stroke_style_t *cairo_style) {
    _cairo_stroke_style_init(cairo_style);
	cairo_style->line_width = width;
	cairo_style->miter_limit = style->get_miter_limit();
	cairo_style->dash_offset = style->get_dash_offset()*width;
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            cairo_style->line_cap = CAIRO_LINE_CAP_BUTT;
            break;
        case e_stroke_cap::round:
            cairo_style->line_cap = CAIRO_LINE_CAP_ROUND;
            break;
        case e_stroke_cap::square:
            cairo_style->line_cap = CAIRO_LINE_CAP_SQUARE;
            break;
        default:
            break;
    }
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            cairo_style->line_join = CAIRO_LINE_JOIN_BEVEL;
            break;
        case e_stroke_join::round:
            cairo_style->line_join = CAIRO_LINE_JOIN_ROUND;
            break;
        case e_stroke_join::miter_clip: // Cairo does not have clipped miters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            cairo_style->line_join = CAIRO_LINE_JOIN_MITER;
            break;
        default:
            break;
    }
	if (!style->get_dashes().empty()) {
		const auto &dashes = style->get_dashes();
		cairo_style->dash = (double *) malloc(dashes.size()*sizeof(double));
		for (unsigned i = 0; i < dashes.size(); i++) {
			cairo_style->dash[i] = width*dashes[i];
		}
        cairo_style->num_dashes = dashes.size();
	}
}

cairo_status_t move_to(void *closure, const cairo_point_t *p) {
	reinterpret_cast<svg_closed_path *>(closure)->move_to_abs(
    	_cairo_fixed_to_double(p->x), _cairo_fixed_to_double(p->y));
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t line_to(void *closure, const cairo_point_t *p) {
	reinterpret_cast<svg_closed_path *>(closure)->line_to_abs(
    	_cairo_fixed_to_double(p->x), _cairo_fixed_to_double(p->y));
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t curve_to(void *closure, const cairo_point_t *p1,
    const cairo_point_t *p2, const cairo_point_t *p3) {
	reinterpret_cast<svg_closed_path *>(closure)->cubic_to_abs(
        _cairo_fixed_to_double(p1->x),
        _cairo_fixed_to_double(p1->y),
        _cairo_fixed_to_double(p2->x),
        _cairo_fixed_to_double(p2->y),
        _cairo_fixed_to_double(p3->x),
        _cairo_fixed_to_double(p3->y));
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t close_path(void *closure) {
	reinterpret_cast<svg_closed_path *>(closure)->close_path();
    return CAIRO_STATUS_SUCCESS;
}

shape cairo_traps(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style) {
	cairo_stroke_style_t cairo_style;
	init_cairo_stroke_style(width, style, &cairo_style);
	cairo_path_fixed_t cairo_input_path;
	init_cairo_path_fixed(input_shape, screen_xf, &cairo_input_path);
    cairo_matrix_t identity;
    cairo_matrix_init_identity(&identity);
    cairo_traps_t traps;
    _cairo_traps_init(&traps);
    // D?? change tolerance based on screen_xf
    _cairo_path_fixed_stroke_to_traps(&cairo_input_path, &cairo_style,
		//&identity, &identity, 2.f, &traps);
		&identity, &identity, CAIRO_GSTATE_TOLERANCE_DEFAULT, &traps);
    cairo_path_fixed_t cairo_output_path;
    _cairo_path_fixed_init(&cairo_output_path);
    _cairo_traps_path(&traps, &cairo_output_path);
	auto output_path = make_intrusive<path_data>();
	auto converter = make_svg_path_f_to_input_path(
		make_input_path_f_close_contours(*output_path));
    _cairo_path_fixed_interpret(&cairo_output_path,
        move_to,
        line_to,
        curve_to,
        close_path,
        &converter);
    converter.close_path();
    _cairo_traps_fini (&traps);
    _cairo_path_fixed_fini(&cairo_output_path);
    _cairo_stroke_style_fini(&cairo_style);
	return shape{output_path};
}

shape cairo_tristrip(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style) {
	cairo_stroke_style_t cairo_style;
	init_cairo_stroke_style(width, style, &cairo_style);
	cairo_path_fixed_t cairo_input_path;
	init_cairo_path_fixed(input_shape, screen_xf, &cairo_input_path);
    cairo_matrix_t identity;
    cairo_matrix_init_identity(&identity);
    cairo_tristrip_t strip;
    _cairo_tristrip_init(&strip);
    cairo_box_t dummy;
    _cairo_tristrip_limit(&strip, &dummy, 0);
    // D?? change tolerance based on screen_xf
    _cairo_path_fixed_stroke_to_tristrip(&cairo_input_path, &cairo_style,
         //&identity, &identity, 2.f, &strip);
         &identity, &identity, CAIRO_GSTATE_TOLERANCE_DEFAULT, &strip);
    cairo_point_t pts[3];
    if (strip.num_points >= 2) {
        pts[0] = strip.points[0];
        pts[1] = strip.points[1];
    }
    auto output_path = make_intrusive<path_data>();
    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));
    for (int n = 2; n < strip.num_points; n++) {
        pts[(n-0)%3] = strip.points[n];
        double x2 = _cairo_fixed_to_double(pts[(n-2)%3].x);
        double y2 = _cairo_fixed_to_double(pts[(n-2)%3].y);
        double x1 = _cairo_fixed_to_double(pts[(n-1)%3].x);
        double y1 = _cairo_fixed_to_double(pts[(n-1)%3].y);
        double x0 = _cairo_fixed_to_double(pts[(n-0)%3].x);
        double y0 = _cairo_fixed_to_double(pts[(n-0)%3].y);
        double d = x0*y1 + x1*y2 + x2*y0 - x1*y0 - x2*y1 - x0*y2;
        if (!util::is_almost_zero(d)) {
            if (d > 0) {
                converter.move_to_abs(x2, y2);
                converter.line_to_abs(x1, y1);
                converter.line_to_abs(x0, y0);
                converter.close_path();
            } else {
                converter.move_to_abs(x2, y2);
                converter.line_to_abs(x0, y0);
                converter.line_to_abs(x1, y1);
                converter.close_path();
            }
        }
    }
    _cairo_tristrip_fini(&strip);
    _cairo_stroke_style_fini(&cairo_style);
    return shape{output_path};
}

shape cairo_polygon(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style) {
	cairo_stroke_style_t cairo_style;
	init_cairo_stroke_style(width, style, &cairo_style);
	cairo_path_fixed_t cairo_input_path;
	init_cairo_path_fixed(input_shape, screen_xf, &cairo_input_path);
    cairo_matrix_t identity;
    cairo_matrix_init_identity(&identity);
    cairo_polygon_t cairo_poly;
    _cairo_polygon_init(&cairo_poly, nullptr, 0);
    // D?? change tolerance based on screen_xf
    _cairo_path_fixed_stroke_to_polygon(&cairo_input_path, &cairo_style,
        &identity, &identity, CAIRO_GSTATE_TOLERANCE_DEFAULT, &cairo_poly);
        //&identity, &identity, 2.f, &cairo_poly);
    auto output_path = make_intrusive<path_data>();
    auto converter = make_svg_path_f_to_input_path(
            make_input_path_f_close_contours(*output_path));

    using point = std::pair<int,int>;
    std::multimap<point, point> map;

    for (int n = 0; n < cairo_poly.num_edges; n++) {
        cairo_edge_t *edge = &cairo_poly.edges[n];
        int x0, y0;
        int x1, y1;
        if (edge->dir < 0) {
            x0 = edge->line.p2.x;
            y0 = edge->line.p2.y;
            x1 = edge->line.p1.x;
            y1 = edge->line.p1.y;
        } else {
            x0 = edge->line.p1.x;
            y0 = edge->line.p1.y;
            x1 = edge->line.p2.x;
            y1 = edge->line.p2.y;
        }
        map.insert(std::make_pair(point(x0, y0), point(x1, y1)));
    }

    while (!map.empty()) {
        auto edge = map.begin();
        auto p0 = edge->first;
        converter.move_to_abs(
            _cairo_fixed_to_double(p0.first),
            _cairo_fixed_to_double(p0.second));
        while (edge != map.end()) {
            auto p1 = edge->second;
            converter.line_to_abs(
                _cairo_fixed_to_double(p1.first),
                _cairo_fixed_to_double(p1.second));
            map.erase(edge);
            edge = map.find(p1);
        }
        converter.close_path();
    }

    _cairo_path_fixed_fini(&cairo_input_path);
    _cairo_stroke_style_fini(&cairo_style);
    _cairo_polygon_fini(&cairo_poly);
	return shape{output_path};
}

} }

