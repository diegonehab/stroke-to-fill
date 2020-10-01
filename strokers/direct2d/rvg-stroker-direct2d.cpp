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
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d2d1.h>
#include <d2d1helper.h>

#include "rvg-stroker-direct2d.h"

#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"
#include "rvg-input-path-f-to-svg-path.h"
#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-close-contours.h"

using namespace D2D1;

namespace rvg {
    namespace stroker {

class input_path_f_to_geometry_sink final:
    public i_input_path<input_path_f_to_geometry_sink> {

	ID2D1GeometrySink *m_sink;

public:
    input_path_f_to_geometry_sink(ID2D1GeometrySink *sink): m_sink(sink) { }

private:
    friend i_input_path<input_path_f_to_geometry_sink>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
		m_sink->AddLine(Point2F(x1, y1));
    }

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
		(void) x0; (void) y0;
		(void) dx0; (void) dy0;
		(void) dx1; (void) dy1;
		m_sink->AddLine(Point2F(x1, y1));
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        (void) x0; (void) y0;
		m_sink->AddQuadraticBezier(QuadraticBezierSegment(Point2F(x1, y1),
            Point2F(x2, y2)));
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
		m_sink->AddBezier(BezierSegment(Point2F(x1, y1),
            Point2F(x2, y2), Point2F(x3, y3)));
    }

    void do_begin_contour(float x0, float y0) {
		m_sink->BeginFigure(Point2F(x0, y0), D2D1_FIGURE_BEGIN_HOLLOW);
    }

    void do_end_open_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        m_sink->EndFigure(D2D1_FIGURE_END_OPEN);
    }

    void do_end_closed_contour(float x0, float y0) {
        (void) x0;
        (void) y0;
        m_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    }
};

static inline input_path_f_to_geometry_sink make_input_path_f_to_geometry_sink(
	ID2D1GeometrySink *sink) {
    return input_path_f_to_geometry_sink{sink};
}


template <typename SINK>
class geometry_sink_f_to_svg_path: public ID2D1GeometrySink {

    SINK m_sink;
	UINT m_cRef;

public:
	explicit geometry_sink_f_to_svg_path(SINK &&sink):
        m_sink(std::forward<SINK>(sink)),
        m_cRef(1) {	}

	STDMETHOD_(ULONG, AddRef)(THIS) {
		return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&m_cRef));
	}

	STDMETHOD_(ULONG, Release)(THIS) {
		ULONG cRef = static_cast<ULONG>(
			InterlockedDecrement(reinterpret_cast<LONG volatile *>(&m_cRef)));

		if (0 == cRef) {
			delete this;
		}

		return cRef;
	}

	STDMETHOD(QueryInterface)(THIS_ REFIID iid, void** ppvObject) {
		HRESULT hr = S_OK;

		if (__uuidof(IUnknown) == iid) {
			*ppvObject = static_cast<IUnknown*>(this);
			AddRef();
		} else if (__uuidof(ID2D1SimplifiedGeometrySink) == iid) {
			*ppvObject = static_cast<ID2D1SimplifiedGeometrySink*>(this);
			AddRef();
		} else {
			*ppvObject = NULL;
			hr = E_NOINTERFACE;
		}

		return hr;
	}

	STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT *beziers,
        UINT count) {
        for (unsigned i = 0; i < count; i++)
            this->AddBezier(beziers + i);
	}

	STDMETHOD_(void, AddArc)(const D2D1_ARC_SEGMENT *arc) {
        m_sink.svg_arc_to_abs(arc->size.width, arc->size.height,
            arc->rotationAngle, arc->arcSize == D2D1_ARC_SIZE_LARGE,
            arc->sweepDirection == D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            arc->point.x, arc->point.y);
	}

	STDMETHOD_(void, AddBezier)(const D2D1_BEZIER_SEGMENT *bezier) {
        m_sink.cubic_to_abs(
            bezier->point1.x,
            bezier->point1.y,
            bezier->point2.x,
            bezier->point2.y,
            bezier->point3.x,
            bezier->point3.y);
	}

	STDMETHOD_(void, AddLine)(D2D1_POINT_2F point) {
        m_sink.line_to_abs(point.x, point.y);
	}

	STDMETHOD_(void, AddQuadraticBezier)(const D2D1_QUADRATIC_BEZIER_SEGMENT *
        quadratic_bezier) {
        m_sink.quad_to_abs(
            quadratic_bezier->point1.x,
            quadratic_bezier->point1.y,
            quadratic_bezier->point2.x,
            quadratic_bezier->point2.y);
	}

	STDMETHOD_(void, AddQuadraticBeziers)(
        const D2D1_QUADRATIC_BEZIER_SEGMENT *quadratic_beziers, UINT count) {
        for (unsigned i = 0; i < count; i++)
            this->AddQuadraticBezier(quadratic_beziers + i);
	}

	STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT count)	{
		for (unsigned i = 0; i < count; i++)
            this->AddLine(points[i]);
	}

	STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint,
        D2D1_FIGURE_BEGIN figureBegin) {
		(void) figureBegin;
		m_sink.move_to_abs(startPoint.x, startPoint.y);
	}

	STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd) {
        if (figureEnd == D2D1_FIGURE_END_CLOSED)
            m_sink.close_path();
	}

	STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode) {
        (void) fillMode;
	}

	STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags) {
        (void) vertexFlags;
	}

	STDMETHOD(Close)() {
		return S_OK;
	}

};

template <typename SINK>
geometry_sink_f_to_svg_path<SINK>
make_geometry_sink_f_to_svg_path(SINK &&sink) {
	return geometry_sink_f_to_svg_path<SINK>{std::forward<SINK>(sink)};
}

shape direct2d(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {
    (void) screen_xf;
    xform xf;
	ID2D1Factory *pFactory;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	ID2D1PathGeometry *pInputPath;
	pFactory->CreatePathGeometry(&pInputPath);
	ID2D1GeometrySink *pSink;

	pInputPath->Open(&pSink);
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_input_path_f_to_geometry_sink(pSink))));
	pSink->Close();

    D2D1_CAP_STYLE startCap = D2D1_CAP_STYLE_FLAT;
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            startCap = D2D1_CAP_STYLE_FLAT;
            break;
        case e_stroke_cap::round:
            startCap = D2D1_CAP_STYLE_ROUND;
            break;
        case e_stroke_cap::square:
            startCap = D2D1_CAP_STYLE_SQUARE;
            break;
        case e_stroke_cap::triangle:
            startCap = D2D1_CAP_STYLE_TRIANGLE;
            break;
        default:
            break;
    }

    D2D1_CAP_STYLE endCap = D2D1_CAP_STYLE_FLAT;
    switch (style->get_terminal_cap()) {
        case e_stroke_cap::butt:
            endCap = D2D1_CAP_STYLE_FLAT;
            break;
        case e_stroke_cap::round:
            endCap = D2D1_CAP_STYLE_ROUND;
            break;
        case e_stroke_cap::square:
            endCap = D2D1_CAP_STYLE_SQUARE;
            break;
        case e_stroke_cap::triangle:
            endCap = D2D1_CAP_STYLE_TRIANGLE;
            break;
        default:
            break;
    }

    D2D1_CAP_STYLE dashCap = D2D1_CAP_STYLE_FLAT;
    switch (style->get_dash_initial_cap()) {
        case e_stroke_cap::butt:
            dashCap = D2D1_CAP_STYLE_FLAT;
            break;
        case e_stroke_cap::round:
            dashCap = D2D1_CAP_STYLE_ROUND;
            break;
        case e_stroke_cap::square:
            dashCap = D2D1_CAP_STYLE_SQUARE;
            break;
        case e_stroke_cap::triangle:
            dashCap = D2D1_CAP_STYLE_TRIANGLE;
            break;
        default:
            break;
    }


    D2D1_LINE_JOIN lineJoin = D2D1_LINE_JOIN_MITER;
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            lineJoin =  D2D1_LINE_JOIN_BEVEL;
            break;
        case e_stroke_join::round:
            lineJoin =  D2D1_LINE_JOIN_ROUND;
            break;
        case e_stroke_join::miter_or_bevel:
            lineJoin =  D2D1_LINE_JOIN_MITER_OR_BEVEL;
            break;
        case e_stroke_join::miter_clip:
            lineJoin =  D2D1_LINE_JOIN_MITER;
            break;
        default:
            break;
    }

    FLOAT miterLimit = style->get_miter_limit();
	D2D1_DASH_STYLE dashStyle = style->get_dashes().empty() ?
		D2D1_DASH_STYLE_SOLID : D2D1_DASH_STYLE_CUSTOM;
    FLOAT dashOffset = style->get_dash_offset();

    std::vector<FLOAT> dashes;

    for (auto d: style->get_dashes()) {
        dashes.push_back(d);
    }

	ID2D1StrokeStyle *pStyle;
	pFactory->CreateStrokeStyle(
        StrokeStyleProperties(
            startCap,
		    endCap,
		    dashCap,
		    lineJoin,
		    miterLimit,
            dashStyle,
            dashOffset),
		dashes.empty()? nullptr: &dashes[0],
        static_cast<UINT>(dashes.size()),
        &pStyle);

	ID2D1PathGeometry *pStrokedPath;
	pFactory->CreatePathGeometry(&pStrokedPath);
	pStrokedPath->Open(&pSink);
	pInputPath->Widen(width, pStyle, IdentityMatrix(), pSink);
	pSink->Close();

	auto output_path = make_intrusive<path_data>();
    auto svg_path = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));
	auto converter = make_geometry_sink_f_to_svg_path(svg_path);

	pStrokedPath->Stream(&converter);

    svg_path.close_path();

	pInputPath->Release();
	pSink->Release();
	pStrokedPath->Release();
	pFactory->Release();

	return shape{output_path};
}

} }

