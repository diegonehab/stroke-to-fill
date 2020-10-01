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
extern "C" {

#include <mupdf/fitz/context.h>
#include <mupdf/fitz/pixmap.h>
#include <mupdf/fitz/glyph.h>
#include <mupdf/fitz/path.h>
#include <mupdf/private/fitz/draw-imp.h>

}

#include <map>

#include "rvg-stroker-mupdf.h"

#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"

namespace rvg {
    namespace stroker {

class path_f_to_fz_path: public i_input_path<path_f_to_fz_path> {
    fz_context *m_ctx;
    fz_path *m_path;
public:
	explicit path_f_to_fz_path(fz_context *ctx, fz_path *path):
        m_ctx(ctx), m_path(path) { ; }

private:

friend i_input_path<path_f_to_fz_path>;

    void do_begin_contour(float x0, float y0) {
        fz_moveto(m_ctx, m_path, x0, y0);
	}

    void do_end_open_contour(float x0, float y0) {
		(void) x0; (void) y0;
		// do nothing
	}

    void do_end_closed_contour(float x0, float y0) {
		(void) x0; (void) y0;
        fz_closepath(m_ctx, m_path);
	}

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
        fz_lineto(m_ctx, m_path, x1, y1);
	}

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
		(void) x0; (void) y0;
        fz_quadto(m_ctx, m_path, x1, y1, x2, y2);
	}

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
		(void) x0; (void) y0;
		(void) x1; (void) y1; (void) w1;
		(void) x2; (void) y2;
        // should never be called
	}

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
		(void) x0; (void) y0;
        fz_curveto(m_ctx, m_path, x1, y1, x2, y2, x3, y3);
	}

};

fz_linecap rvg_cap_to_fz_cap(e_stroke_cap cap) {
    switch (cap) {
        case e_stroke_cap::butt: return FZ_LINECAP_BUTT;
        case e_stroke_cap::round: return FZ_LINECAP_ROUND;
        case e_stroke_cap::square: return FZ_LINECAP_SQUARE;
        case e_stroke_cap::triangle: return FZ_LINECAP_TRIANGLE;
        default: return FZ_LINECAP_BUTT;
    }
}

fz_linejoin rvg_join_to_fz_join(e_stroke_join join) {
    switch (join) {
        case e_stroke_join::bevel: return FZ_LINEJOIN_BEVEL;
        case e_stroke_join::round: return FZ_LINEJOIN_ROUND;
        case e_stroke_join::miter_clip: return FZ_LINEJOIN_MITER_XPS;
        case e_stroke_join::miter_or_bevel: return FZ_LINEJOIN_MITER;
        default: return FZ_LINEJOIN_BEVEL;
    }
}

static inline auto make_path_f_to_fz_path(fz_context *ctx, fz_path *output) {
	return path_f_to_fz_path{ctx, output};
}

static fz_matrix rvg_xf_to_fz_matrix(const xform &xf) {
    fz_matrix m;
    m.a = xf[0][0]; m.b = xf[0][1]; m.e = xf[0][2];
    m.c = xf[1][0]; m.d = xf[1][1]; m.f = xf[1][2];
    return m;
}


shape mupdf(const shape &input_shape, const xform &screen_xf, float width,
    stroke_style::const_ptr style) {

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    fz_path *input_fz_path = fz_new_path(ctx);

    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_path_f_to_fz_path(ctx, input_fz_path))));

    fz_stroke_state *input_fz_style = fz_new_stroke_state_with_dash_len(ctx,
        style->get_dashes().size());

    input_fz_style->start_cap = rvg_cap_to_fz_cap(style->get_initial_cap());
    input_fz_style->end_cap = rvg_cap_to_fz_cap(style->get_terminal_cap());
    input_fz_style->dash_cap = rvg_cap_to_fz_cap(style->get_dash_initial_cap());

    input_fz_style->linejoin = rvg_join_to_fz_join(style->get_join());

    input_fz_style->miterlimit = style->get_miter_limit();
    input_fz_style->dash_phase = style->get_dash_offset()*width;
    const auto &dashes = style->get_dashes();
    input_fz_style->dash_len = dashes.size();
    if (!dashes.empty()) {
        for (unsigned i = 0; i < dashes.size(); i++) {
            input_fz_style->dash_list[i] = dashes[i]*width;
        }
    }

    fz_matrix ctm = rvg_xf_to_fz_matrix(screen_xf);
    fz_rasterizer *ras = fz_new_gel(ctx);
    fz_set_rasterizer_graphics_aa_level(ctx, &ras->aa, 8);
    fz_gel *gel = (fz_gel *) ras;
    fz_irect scissor;
    // infinite irect
    scissor.x0 = 1; scissor.x1 = scissor.x0-1;
    scissor.y0 = 1; scissor.y1 = scissor.y0-1;
    float expansion = fz_matrix_expansion(ctm);
    float flatness = 0.3f / expansion;
    if (flatness < 0.001f) flatness = 0.001f;

    fz_flatten_stroke_path(ctx, ras, input_fz_path, input_fz_style,
        ctm, flatness, width, &scissor, nullptr /* bbox */);

#if 0
fprintf(stderr, "after %d\n", gel->len);
    for (int i = 0; i < gel->len; i++) {
        fz_edge *e = &gel->edges[i];
        int x0, y0;
        int x1, y1;
        x0 = e->x;
        y0 = e->y;
        x1 = e->xmove;
        y1 = e->adj_up;
        fprintf(stderr, "%d %d -> %d %d\n", x0, y0, x1, y1);
    }
#endif

    float tofloat_x = 1.f/ctx->aa->hscale;
    float tofloat_y = 1.f/ctx->aa->vscale;

    using point = std::pair<int,int>;
    std::multimap<point, point> map;

    for (int i = 0; i < gel->len; i++) {
        fz_edge *e = &gel->edges[i];
        int x0, y0;
        int x1, y1;
        x0 = e->x;
        y0 = e->y;
        x1 = e->xmove;
        y1 = e->adj_up;
        map.insert(std::make_pair(point(x0, y0), point(x1, y1)));
    }

    auto output_path = make_intrusive<path_data>();
    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));

    while (!map.empty()) {
        auto edge = map.begin();
        auto p0 = edge->first;
        auto p1 = edge->first;
        converter.move_to_abs(tofloat_x*p0.first, tofloat_y*p0.second);
        while (edge != map.end()) {
            p1 = edge->second;
            converter.line_to_abs(tofloat_x*p1.first, tofloat_y*p1.second);
            map.erase(edge);
            edge = map.find(p1);
        }
        if (p1 == p0)
            converter.close_path();
    }

    fz_drop_rasterizer(ctx, ras);
    fz_drop_stroke_state(ctx, input_fz_style);
    fz_drop_path(ctx, input_fz_path);
    fz_drop_context(ctx);

    return shape{output_path};
}

} }
