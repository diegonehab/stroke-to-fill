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
#include <cstdint>
#include <iostream>

#include <SkPath.h>
#include <SkPaint.h>
#include <SkDashPathEffect.h>

#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-bezier.h"
#include "rvg-canonize-rational-quadratic-bezier.h"

#include "rvg-stroker-skia.h"

namespace rvg {
    namespace stroker {

class input_path_f_to_skpath: public i_input_path<input_path_f_to_skpath> {
    SkPath &m_skpath;
public:
	explicit input_path_f_to_skpath(SkPath &skpath): m_skpath(skpath) { ; }

private:

friend i_input_path<input_path_f_to_skpath>;

    void do_begin_contour(float x0, float y0) {
		m_skpath.moveTo(x0, y0);
	}

    void do_end_open_contour(float x0, float y0) {
		(void) x0; (void) y0;
		// do nothing
	}

    void do_end_closed_contour(float x0, float y0) {
		(void) x0; (void) y0;
		m_skpath.close();
	}

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
		m_skpath.lineTo(x1, y1);
	}

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		(void) x0; (void) y0;
		m_skpath.quadTo(x1, y1, x2, y2);
	}

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
		(void) x0; (void) y0;
        if (util::is_almost_zero(w1)) {
            R3 p0{x0,y0,1}, p1{x1,y1,w1}, p2{x2,y2,1};
            R3 q1,q2,q3;
            // split in two
            std::tie(q1,q2,q3) = bezier_split<rvgf>(
                std::make_tuple(p0,p1,p2), rvgf{0.5}
            );
            // canonize
            R2 a0, a2, b0, b2;
            R3 a1, b1;
            std::tie(a0, a1, a2) = canonize_rational_quadratic_bezier(p0,q1,q2);
            std::tie(b0, b1, b2) = canonize_rational_quadratic_bezier(q2,q3,p2);
            // send two pieces
            m_skpath.conicTo(a1[0]/a1[2], a1[1]/a1[2], a2[0], a2[1], a1[2]);
            m_skpath.conicTo(b1[0]/b1[2], b1[1]/b1[2], b2[0], b2[1], b1[2]);
        } else {
            m_skpath.conicTo(x1 / w1, y1 / w1, x2, y2, w1);
        }

	}

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
		m_skpath.cubicTo(x1, y1, x2, y2, x3, y3);
	}

};

static inline auto make_input_path_f_to_skpath(SkPath &output) {
	return input_path_f_to_skpath{output};
}

shape skia(const shape &input_shape,
    const xform &screen_xf, float width, stroke_style::const_ptr style) {
    SkPaint sk_paint;
    SkPath sk_input_path, sk_output_path;
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_to_skpath(sk_input_path)));
    sk_paint.setStyle(SkPaint::kStroke_Style);
    sk_paint.setStrokeWidth(width);
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            sk_paint.setStrokeCap(SkPaint::kButt_Cap);
            break;
        case e_stroke_cap::round:
            sk_paint.setStrokeCap(SkPaint::kRound_Cap);
            break;
        case e_stroke_cap::square:
            sk_paint.setStrokeCap(SkPaint::kSquare_Cap);
            break;
        default:
            break;
    }
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            sk_paint.setStrokeJoin(SkPaint::kBevel_Join);
            break;
        case e_stroke_join::round:
            sk_paint.setStrokeJoin(SkPaint::kRound_Join);
            break;
        case e_stroke_join::miter_clip: // Skia does not have clipped miters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            sk_paint.setStrokeJoin(SkPaint::kMiter_Join);
            break;
        default:
            break;
    }
    auto dashes = style->get_dashes();
    for (auto &d: dashes) {
        d *= width;
    }
    if (!dashes.empty()) {
        sk_paint.setPathEffect(SkDashPathEffect::Make(&dashes[0],
            dashes.size(), style->get_dash_offset()*width));
    }
    sk_paint.getFillPath(sk_input_path, &sk_output_path);
    SkPath::Iter iter(sk_output_path, false);
    int done = 0;
    auto output_path = make_intrusive<path_data>();
    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));
    while (!done) {
        SkPoint pts[4];
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                converter.move_to_abs(pts[0].x(), pts[0].y());
                break;
            case SkPath::kLine_Verb:
                converter.line_to_abs(pts[1].x(), pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                converter.quad_to_abs(pts[1].x(), pts[1].y(), pts[2].x(),
                    pts[2].y());
                break;
            case SkPath::kConic_Verb: {
                auto w = iter.conicWeight();
                converter.rquad_to_abs(pts[1].x()*w, pts[1].y()*w,
                    w, pts[2].x(), pts[2].y());
                break;
            }
            case SkPath::kCubic_Verb:
                converter.cubic_to_abs(pts[1].x(), pts[1].y(), pts[2].x(),
                    pts[2].y(), pts[3].x(), pts[3].y());
                break;
            case SkPath::kClose_Verb:
                converter.close_path();
                break;
            case SkPath::kDone_Verb:
                done = 1;
                break;
        }
    }

    return shape{output_path};
}

} }
