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
#ifndef RVG_SCENE_F_STROKE_H
#define RVG_SCENE_F_STROKE_H

#include "rvg-lua.h"
#include "rvg-i-scene-data-f-forwarder.h"

#include "rvg-stroker-rvg.h"

#ifdef STROKER_LIVAROT
#include "livarot/rvg-stroker-livarot.h"
#endif

#ifdef STROKER_DIRECT2D
#include "direct2d/rvg-stroker-direct2d.h"
#endif

#ifdef STROKER_QUARTZ
#include "quartz/rvg-stroker-quartz.h"
#endif

#ifdef STROKER_AGG
#include "agg/rvg-stroker-agg.h"
#endif

#ifdef STROKER_SKIA
#include "skia/rvg-stroker-skia.h"
#endif

#ifdef STROKER_QT5
#include "qt5/rvg-stroker-qt5.h"
#endif

#ifdef STROKER_MUPDF
#include "mupdf/rvg-stroker-mupdf.h"
#endif

#ifdef STROKER_CAIRO
#include "cairo/rvg-stroker-cairo.h"
#endif

#ifdef STROKER_GS
#include "gs/rvg-stroker-gs.h"
#endif

#ifdef STROKER_OPENVG_RI
#include "openvg-ri/rvg-stroker-openvg-ri.h"
#endif

namespace rvg {

template <typename SINK>
class scene_f_stroke:
    public i_sink<scene_f_stroke<SINK>>,
    public i_scene_data_f_forwarder<scene_f_stroke<SINK>> {

    e_stroke_method m_method;
    bool m_mock;
    SINK m_sink;

	std::vector<xform> m_xf_stack;

public:
	scene_f_stroke(e_stroke_method method, bool mock, SINK &&sink):
        m_method{method},
        m_mock{mock},
        m_sink{std::forward<SINK>(sink)} {
        ;
    }

private:

    void push_xf(const xform &xf) {
        if (m_xf_stack.empty()) {
            m_xf_stack.push_back(xf);
        } else {
            m_xf_stack.push_back(m_xf_stack.back().transformed(xf));
        }
    }

    void pop_xf(void) {
        if (!m_xf_stack.empty()) {
            m_xf_stack.pop_back();
        }
    }

    xform top_xf(void) {
        if (!m_xf_stack.empty()) {
            return m_xf_stack.back();
        } else return xform{};
    }

friend i_sink<scene_f_stroke<SINK>>;

    SINK &do_sink(void) {
        return m_sink;
    }

    const SINK &do_sink(void) const {
        return m_sink;
    }

    shape stroke_to_fill(const shape &s, e_stroke_method m, const xform &screen_xf) {
        if (s.get_type() == shape::e_type::stroke) {
            // maybe we are stroking the result of a stroke?
            auto to_stroke = stroke_to_fill(s.get_stroke_data().get_shape(), m,
                s.get_xf().transformed(screen_xf));
            auto style = s.get_stroke_data().get_style_ptr();
            float width = s.get_stroke_data().get_width();
            switch (m) {
#ifdef STROKER_LIVAROT
                case e_stroke_method::livarot_stroke:
                    return ::rvg::stroker::livarot_stroke(to_stroke, screen_xf, width, style);
                case e_stroke_method::livarot_outline:
                    return ::rvg::stroker::livarot_outline(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_DIRECT2D
                case e_stroke_method::direct2d:
                    return ::rvg::stroker::direct2d(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_QUARTZ
                case e_stroke_method::quartz:
                    return ::rvg::stroker::quartz(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_AGG
                case e_stroke_method::agg:
                    return ::rvg::stroker::agg(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_SKIA
                case e_stroke_method::skia:
                    return ::rvg::stroker::skia(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_QT5
                case e_stroke_method::qt5:
                    return ::rvg::stroker::qt5(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_MUPDF
                case e_stroke_method::mupdf:
                    return ::rvg::stroker::mupdf(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_CAIRO
                case e_stroke_method::cairo_traps:
                    return ::rvg::stroker::cairo_traps(to_stroke, screen_xf, width, style);
                case e_stroke_method::cairo_polygon:
                    return ::rvg::stroker::cairo_polygon(to_stroke, screen_xf, width, style);
                case e_stroke_method::cairo_tristrip:
                    return ::rvg::stroker::cairo_tristrip(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_GS
                case e_stroke_method::gs:
                    return ::rvg::stroker::gs(to_stroke, screen_xf, width, style);
                case e_stroke_method::gs_compat:
                    return ::rvg::stroker::gs_compat(to_stroke, screen_xf, width, style);
                case e_stroke_method::gs_fast:
                    return ::rvg::stroker::gs_fast(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_OPENVG_RI
                case e_stroke_method::openvg_ri:
                    return ::rvg::stroker::openvg_ri(to_stroke, screen_xf, width, style);
#endif
#ifdef STROKER_RVG
                case e_stroke_method::rvg:
#endif
                default:
                    return ::rvg::stroker::rvg(to_stroke, screen_xf, width, style);

            }
        } else {
            return s;
        }
    }

friend i_scene_data<scene_f_stroke<SINK>>;

    void do_painted_shape(e_winding_rule rule, const shape &s, const paint &p) {
        if (s.get_type() == shape::e_type::stroke) {
            if (m_mock) {
                m_sink.painted_shape(e_winding_rule::non_zero,
                    stroke_to_fill(shape{}, m_method, top_xf()), p);
            } else {
                m_sink.painted_shape(e_winding_rule::non_zero,
                    stroke_to_fill(s, m_method, top_xf()), p);
            }
        } else {
            m_sink.painted_shape(rule, s, p);
        }
	}

    void do_begin_transform(uint16_t depth, const xform &xf) {
        push_xf(xf);
        m_sink.begin_transform(depth, xf);
    }

    void do_end_transform(uint16_t depth, const xform &xf) {
        pop_xf();
        m_sink.end_transform(depth, xf);
    }

};

template <typename SINK>
static inline auto
make_scene_f_stroke(e_stroke_method method, SINK &&sink) {
    return scene_f_stroke<SINK>{method, false, std::forward<SINK>(sink)};
}

template <typename SINK>
static inline auto
make_scene_f_stroke(e_stroke_method method, bool mock, SINK &&sink) {
    return scene_f_stroke<SINK>{method, mock, std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
