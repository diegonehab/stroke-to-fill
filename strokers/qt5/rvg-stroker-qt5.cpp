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

#include <QPainterPathStroker>
#include <QPainterPath>

#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"

#include "rvg-stroker-qt5.h"

namespace rvg {
    namespace stroker {

class input_path_f_to_qpainter_path final:
    public i_input_path<input_path_f_to_qpainter_path> {

    QPainterPath *m_path;

public:
    input_path_f_to_qpainter_path(QPainterPath *path): m_path(path) { }

private:

friend i_input_path<input_path_f_to_qpainter_path>;

    void do_begin_contour(float x0, float y0) {
        m_path->moveTo(x0, y0);
    }

    void do_end_open_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
    }

    void do_end_closed_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        m_path->closeSubpath();
    }

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
        m_path->lineTo(x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		(void) x0; (void) y0;
		m_path->quadTo(x1, y1, x2, y2);
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
		m_path->cubicTo(x1, y1, x2, y2, x3, y3);
    }

};

static inline auto make_input_path_f_to_qpainter_path(QPainterPath *path) {
    return input_path_f_to_qpainter_path{path};
}

shape qt5(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {
    QPainterPath qt_input_path;
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_input_path_f_to_qpainter_path(&qt_input_path))));

    QPainterPathStroker qt_stroker;

    qt_stroker.setWidth(width);
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            qt_stroker.setCapStyle(Qt::FlatCap);
            break;
        case e_stroke_cap::round:
            qt_stroker.setCapStyle(Qt::RoundCap);
            break;
        case e_stroke_cap::square:
            qt_stroker.setCapStyle(Qt::SquareCap);
            break;
        default:
            break;
    }
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            qt_stroker.setJoinStyle(Qt::BevelJoin);
            break;
        case e_stroke_join::round:
            qt_stroker.setJoinStyle(Qt::RoundJoin);
            break;
        case e_stroke_join::miter_clip:
            qt_stroker.setJoinStyle(Qt::MiterJoin);
            break;
        case e_stroke_join::miter_or_bevel:
            qt_stroker.setJoinStyle(Qt::SvgMiterJoin);
            break;
        default:
            break;
    }
    qt_stroker.setMiterLimit(style->get_miter_limit());
    qt_stroker.setDashOffset(style->get_dash_offset());
    if (!style->get_dashes().empty()) {
        QVector<qreal> dashPattern;
        for (auto d: style->get_dashes())
            dashPattern.push_back(d);
        qt_stroker.setDashPattern(dashPattern);
    }

    QPainterPath qt_output_path = qt_stroker.createStroke(qt_input_path);

    auto output_path = make_intrusive<path_data>();
    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));

    int curve_to_data = 0;
    float x[3];
    float y[3];
    for (int i = 0; i < qt_output_path.elementCount(); i++) {
        auto element = qt_output_path.elementAt(i);
        switch (element.type) {
            case QPainterPath::MoveToElement:
                converter.move_to_abs(element.x, element.y);
                break;
            case QPainterPath::LineToElement:
                converter.line_to_abs(element.x, element.y);
                break;
            case QPainterPath::CurveToElement:
                x[curve_to_data] = element.x;
                y[curve_to_data] = element.y;
                curve_to_data++;
                break;
            case QPainterPath::CurveToDataElement:
                x[curve_to_data] = element.x;
                y[curve_to_data] = element.y;
                curve_to_data++;
                if (curve_to_data == 3) {
                    converter.cubic_to_abs(x[0], y[0], x[1], y[1], x[2], y[2]);
                    curve_to_data = 0;
                }
                break;
        }
    }
    converter.close_path();

    return shape{output_path};
}

} }
