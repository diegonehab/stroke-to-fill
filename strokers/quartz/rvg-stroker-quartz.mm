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

#include <ApplicationServices/ApplicationServices.h>

#include "rvg-i-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"

#include "rvg-stroker-quartz.h"

namespace rvg {
    namespace stroker {

class input_path_f_to_cgpath final:
    public i_input_path<input_path_f_to_cgpath> {

    CGMutablePathRef m_path;

public:
    input_path_f_to_cgpath(CGMutablePathRef path): m_path(path) { }

private:

friend i_input_path<input_path_f_to_cgpath>;

    void do_begin_contour(float x0, float y0) {
        CGPathMoveToPoint(m_path, nullptr, x0, y0);
    }

    void do_end_open_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
    }

    void do_end_closed_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        CGPathCloseSubpath(m_path);
    }

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
		CGPathAddLineToPoint(m_path, nullptr, x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        (void) x0; (void) y0;
		CGPathAddQuadCurveToPoint(m_path, nullptr, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
		(void) x0; (void) y0;
		(void) x1; (void) y1; (void) w1;
		(void) x2; (void) y2;
        // Should never be called
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
		CGPathAddCurveToPoint(m_path, nullptr, x1, y1, x2, y2, x3, y3);
    }

};

static inline auto make_input_path_f_to_cgpath(CGMutablePathRef path) {
    return input_path_f_to_cgpath{path};
}

static path_data dummy{};
using cgpath_f_to_path = decltype(
    make_svg_path_f_to_input_path(
        dummy));

void callback(void *info, const CGPathElement *element) {
    cgpath_f_to_path *converter = reinterpret_cast<cgpath_f_to_path *>(info);
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            converter->move_to_abs(
                element->points[0].x, element->points[0].y);
            break;
        case kCGPathElementAddLineToPoint:
            converter->line_to_abs(
                element->points[0].x, element->points[0].y);
            break;
        case kCGPathElementAddQuadCurveToPoint:
            converter->quad_to_abs(
                element->points[0].x, element->points[0].y,
                element->points[1].x, element->points[1].y);
            break;
        case kCGPathElementAddCurveToPoint:
            converter->cubic_to_abs(
                element->points[0].x, element->points[0].y,
                element->points[1].x, element->points[1].y,
                element->points[2].x, element->points[2].y);
            break;
        case kCGPathElementCloseSubpath:
            converter->close_path();
            break;
    }
}

shape quartz(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {
    CGMutablePathRef input_cgpath = CGPathCreateMutable();

    input_shape.as_path_data_ptr(screen_xf)->iterate(
        make_input_path_f_xform(input_shape.get_xf(),
            make_input_path_f_rational_quadratic_to_cubics(
                make_input_path_f_to_cgpath(input_cgpath))));

    CGPathRef tostroke_cgpath;
    if (style->get_dashes().empty()) {
        tostroke_cgpath = CGPathCreateCopy(input_cgpath);
    } else {
        std::vector<CGFloat> lengths;
        for (auto d: style->get_dashes()) lengths.push_back(d*width);
        tostroke_cgpath = CGPathCreateCopyByDashingPath(input_cgpath,
            nullptr, style->get_dash_offset()*width,
            &lengths[0], lengths.size());
    }

    auto cap = kCGLineCapButt;
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            cap = kCGLineCapButt;
            break;
        case e_stroke_cap::round:
            cap = kCGLineCapRound;
            break;
        case e_stroke_cap::square:
            cap = kCGLineCapSquare;
            break;
        default:
            break;
    }
    auto join = kCGLineJoinMiter;
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            join = kCGLineJoinBevel;
            break;
        case e_stroke_join::round:
            join = kCGLineJoinRound;
            break;
        case e_stroke_join::miter_clip: // Quartz does not have clipped miters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            join = kCGLineJoinMiter;
            break;
        default:
            break;
    }

    auto stroked_cgpath = CGPathCreateCopyByStrokingPath(tostroke_cgpath,
        nullptr, width, cap, join, style->get_miter_limit());

	auto output_path = make_intrusive<path_data>();
	auto converter = make_svg_path_f_to_input_path(*output_path);

    CGPathApply(stroked_cgpath, &converter, callback);

    CGPathRelease(input_cgpath);
    CGPathRelease(tostroke_cgpath);
    CGPathRelease(stroked_cgpath);

	return shape{output_path};
}

} }

