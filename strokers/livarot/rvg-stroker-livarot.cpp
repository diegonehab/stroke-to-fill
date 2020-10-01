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
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-svg-path-parse.h"

#include "rvg-stroker-livarot.h"

#include "livarot/Path.h"
#include "livarot/Shape.h"

namespace rvg {
    namespace stroker {

class input_path_f_to_livarot_path final:
    public i_input_path<input_path_f_to_livarot_path> {

public:

    Path *m_path;

    input_path_f_to_livarot_path(Path *path): m_path(path) {
    }

private:

friend i_input_path<input_path_f_to_livarot_path>;

    void do_begin_contour(float x0, float y0) {
        m_path->MoveTo(Geom::Point{x0, y0});
    }

    void do_end_open_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
    }

    void do_end_closed_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        m_path->Close();
    }

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
        m_path->LineTo(Geom::Point{x1, y1});
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		// elevate quadratic to cubic
		float u1 = (x0+2.f*x1)/3.f, v1 = (y0+2.f*y1)/3.f;
		float u2 = (x2+2.f*x1)/3.f, v2 = (y2+2.f*y1)/3.f;
        do_cubic_segment(x0, y0, u1, v1, u2, v2, x2, y2);
    }

    void do_rational_quadratic_segment(float, float, float, float, float,
        float, float) {
        // should never be called
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
        m_path->CubicTo(Geom::Point{x3, y3},
            Geom::Point{3.f*(x1-x0), 3.f*(y1-y0)},
            Geom::Point{3.f*(x3-x2), 3.f*(y3-y2)});
    }
};

inline auto make_input_path_f_to_livarot_path(Path *p) {
    return input_path_f_to_livarot_path{p};
}

void init_livarot_path(const shape &input_shape, const xform &screen_xf,
	Path *p) {
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_input_path_f_to_livarot_path(p))));
}

JoinType livarot_join(e_stroke_join join) {
    switch (join) {
        case e_stroke_join::miter_or_bevel:
        case e_stroke_join::miter_clip:
            return join_pointy;
        case e_stroke_join::round:
            return join_round;
        default:
            return join_straight;
    }
}

ButtType livarot_cap(e_stroke_cap cap) {
    switch (cap) {
        case e_stroke_cap::round:
            return butt_round;
        case e_stroke_cap::triangle:
            return butt_pointy;
        case e_stroke_cap::square:
            return butt_square;
        default:
            return butt_straight;
    }
}

std::vector<float> livarot_dashes(float width, stroke_style::const_ptr style,
	float *length, float *offset) {
    std::vector<float> dashes;
	float dlength = 0.0;
	// Find total length
	for (float d: style->get_dashes()) {
		dlength += d*width;
	}
    *length = dlength;
	for (float d: style->get_dashes()) {
		dashes.push_back(d*width);
	}
    float doffset = std::fmod(style->get_dash_offset()*width, dlength);
    if (doffset < 0.f) doffset = dlength+doffset;
	for (int i = 1; i < (int) dashes.size(); i++) {
		dashes[i] += dashes[i-1];
	}
	*offset = doffset;
	return dashes;
}

shape livarot_stroke(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style) {

    Path livarot_input_path; // orig
	init_livarot_path(input_shape, screen_xf, &livarot_input_path);

	auto output_path = make_intrusive<path_data>();

	float dash_offset = 0.f;
    float dash_length = 0.f;
    auto dashes = livarot_dashes(width, style, &dash_length, &dash_offset);
    livarot_input_path.Convert(0.1);

    if (!dashes.empty()) {
        livarot_input_path.DashPolyline(
			0.f,
			0.f,
            dash_length,
            (int) dashes.size(),
            &dashes[0],
            true,
            dash_offset
        );
    }

    Shape strokedShape; //theShape

    livarot_input_path.Stroke(&strokedShape,
        false,
        0.5*width,
        livarot_join(style->get_join()),
        livarot_cap(style->get_initial_cap()),
        style->get_miter_limit()*width
    );

    Path livarot_output_path;
    strokedShape.ConvertToForme(&livarot_output_path);

    auto s = livarot_output_path.svg_dump_path();
    svg_path_iterate(s.c_str(), *output_path);


	return shape{output_path};
}

shape livarot_outline(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style) {

    Path livarot_input_path; // orig
	init_livarot_path(input_shape, screen_xf, &livarot_input_path);

	auto output_path = make_intrusive<path_data>();

	float dash_offset = 0.f;
    float dash_length = 0.f;
    auto dashes = livarot_dashes(width, style, &dash_length, &dash_offset);

    if (!dashes.empty()) {
        livarot_input_path.Convert(0.1);
        livarot_input_path.DashPolyline(
			0.f,
			0.f,
            dash_length,
            (int) dashes.size(),
            &dashes[0],
            true,
            dash_offset
        );
        livarot_input_path.Simplify(0.1);
    }

    Path livarot_output_path;
    livarot_input_path.Outline(&livarot_output_path,
        0.5*width,
        livarot_join(style->get_join()),
        livarot_cap(style->get_initial_cap()),
        style->get_miter_limit()*width
    );

    auto s = livarot_output_path.svg_dump_path();
    svg_path_iterate(s.c_str(), *output_path);

	return shape{output_path};
}


} }

