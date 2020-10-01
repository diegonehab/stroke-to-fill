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
#include "rvg-gs.h"
#include "rvg-stroker-gs.h"

#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-input-path-f-xform.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-rational-quadratic-to-cubics.h"

namespace rvg {
    namespace stroker {

class path_f_to_gx_path final: public i_input_path<path_f_to_gx_path> {

    void *m_path;

public:

	explicit path_f_to_gx_path(void *path): m_path(path) { ; }

private:

friend i_input_path<path_f_to_gx_path>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
		(void) x0; (void) y0;
        rvg_gx_path_add_line(m_path, x1, y1);
	}

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
		(void) x0; (void) y0;
		(void) dx0; (void) dy0;
		(void) dx1; (void) dy1;
        rvg_gx_path_add_line(m_path, x1, y1);
	}

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        // elevate quadratic to cubic
        float u1 = (x0+2.f*x1)/3.f, v1 = (y0+2.f*y1)/3.f;
        float u2 = (x2+2.f*x1)/3.f, v2 = (y2+2.f*y1)/3.f;
        rvg_gx_path_add_curve(m_path, u1, v1, u2, v2, x2, y2);
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
		rvg_gx_path_add_curve(m_path, x1, y1, x2, y2, x3, y3);
	}

    void do_begin_contour(float x0, float y0) {
		rvg_gx_path_add_point(m_path, x0, y0);
	}

    void do_end_open_contour(float x0, float y0) {
		(void) x0; (void) y0;
		// do nothing
	}

    void do_end_closed_contour(float x0, float y0) {
		(void) x0; (void) y0;
		rvg_gx_path_close_subpath(m_path);
	}
};

static inline auto make_path_f_to_gx_path(void *output) {
	return path_f_to_gx_path{output};
}

static shape gs_any(const shape &input_shape, const xform &screen_xf,
    float width, stroke_style::const_ptr style, int mode) {
    void *pmem = rvg_gs_malloc_init();
    void *pdev = nullptr;
    rvg_gs_copydevice(&pdev, rvg_gs_getdefaultdevice(), pmem);
	void *pgs = rvg_gs_gstate_alloc(pmem);
    rvg_gs_setcpsimode(pmem, 0);
	void *input_gx_path = rvg_gx_path_alloc(pmem);
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_rational_quadratic_to_cubics(
                    make_path_f_to_gx_path(input_gx_path))));

    rvg_gs_setlinewidth(pgs, width);

    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            rvg_gs_setlinecap(pgs, rvg_gs_cap_butt);
            break;
        case e_stroke_cap::round:
            rvg_gs_setlinecap(pgs, rvg_gs_cap_round);
            break;
        case e_stroke_cap::square:
            rvg_gs_setlinecap(pgs, rvg_gs_cap_square);
            break;
        case e_stroke_cap::triangle:
            rvg_gs_setlinecap(pgs, rvg_gs_cap_triangle);
            break;
        default:
            break;
    }
    switch (style->get_join()) {
        case e_stroke_join::bevel:
			rvg_gs_setlinejoin(pgs, rvg_gs_join_bevel);
            break;
        case e_stroke_join::round:
			rvg_gs_setlinejoin(pgs, rvg_gs_join_round);
            break;
        case e_stroke_join::miter_clip: // gs does not have clipped miters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
			rvg_gs_setlinejoin(pgs, rvg_gs_join_miter);
            break;
        default:
            break;
    }

    auto dashes = style->get_dashes();
    for (auto &d: dashes) {
        d *= width;
    }
    if (!dashes.empty()) {
        rvg_gs_setdash(pgs, &dashes[0], dashes.size(),
            style->get_dash_offset()*width);
    }

    void *output_gx_path = rvg_gx_path_alloc(pmem);

    rvg_gx_stroke_path_only(input_gx_path, output_gx_path, pdev, pgs,
		0.2f, mode);
//		5.f, mode);

    auto output_path = make_intrusive<path_data>();

    auto converter = make_svg_path_f_to_input_path(
        make_input_path_f_close_contours(*output_path));

    void *cenum = rvg_gs_path_enum_alloc(pmem);
    rvg_gx_path_enum_init(cenum, output_gx_path);

    std::vector<unsigned char> vs_mem(rvg_gs_fixed_point_sizeof*3);
    unsigned char *vs = &vs_mem[0];
    for ( ;; ) {
        int pe_op = rvg_gx_path_enum_next(cenum, vs);
        if (pe_op == rvg_gs_pe_moveto) {
            float x0, y0;
            rvg_gs_fixed_point_get(vs, 0, &x0, &y0);
            converter.move_to_abs(x0, y0);
        } else if (pe_op == rvg_gs_pe_lineto) {
            float x1, y1;
            rvg_gs_fixed_point_get(vs, 0, &x1, &y1);
            converter.line_to_abs(x1, y1);
        } else if (pe_op == rvg_gs_pe_closepath) {
            converter.close_path();
        } else if (pe_op == rvg_gs_pe_curveto) {
            float x1, y1;
            float x2, y2;
            float x3, y3;
            rvg_gs_fixed_point_get(vs, 0, &x1, &y1);
            rvg_gs_fixed_point_get(vs, 1, &x2, &y2);
            rvg_gs_fixed_point_get(vs, 2, &x3, &y3);
            converter.cubic_to_abs(x1, y1, x2, y2, x3, y3);
        } else if (pe_op == 0) {
            break;
        } else {
            fprintf(stderr, "unknown operation found (%d)!\n", pe_op);
        }
    }

    rvg_gx_path_free(input_gx_path);
    rvg_gx_path_free(output_gx_path);
    rvg_gs_gstate_free(pgs);
    rvg_gs_malloc_release(pmem);

    return shape{output_path};
}

shape gs_compat(const shape &input_shape,
    const xform &screen_xf, float width, stroke_style::const_ptr style) {
    return gs_any(input_shape, screen_xf, width, style, RVG_STROKER_GS_COMPAT);
}

shape gs_fast(const shape &input_shape,
    const xform &screen_xf, float width, stroke_style::const_ptr style) {
    return gs_any(input_shape, screen_xf, width, style, RVG_STROKER_GS_FAST);
}

shape gs(const shape &input_shape,
    const xform &screen_xf, float width, stroke_style::const_ptr style) {
    return gs_any(input_shape, screen_xf, width, style, RVG_STROKER_GS);
}

} }
