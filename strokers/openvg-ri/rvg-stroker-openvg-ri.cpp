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

#include "openvg.h"
#include "riPath.h"
#include "riRasterizer.h"

#include "rvg-input-path-f-xform.h"
#include "rvg-i-svg-path.h"
#include "rvg-input-path-f-to-svg-path.h"

#include "rvg-stroker-openvg-ri.h"

namespace rvg {
    namespace stroker {

class svg_path_f_segments_data final:
    public i_svg_path<svg_path_f_segments_data> {

    std::vector<uint8_t> &m_segments;
    std::vector<float> &m_data;

    template <typename ...DATA>
    void print_cmd(int cmd, DATA ...data) {
        m_segments.push_back(cmd);
        print_data(data...);
    }

    void print_data(void) { }

    template <typename ...REST>
    void print_data(float f, REST... rest) {
        m_data.push_back(f);
        print_data(rest...);
    }

public:
    svg_path_f_segments_data(std::vector<uint8_t> &segments,
        std::vector<float> &data):
        m_segments(segments),
        m_data(data) { ; }

private:
    friend i_svg_path<svg_path_f_segments_data>;

    void do_move_to_abs(float x0, float y0) {
        print_cmd(VG_MOVE_TO_ABS, x0, y0);
    }

    void do_move_to_rel(float x0, float y0) {
        print_cmd(VG_MOVE_TO_REL, x0, y0);
    }

    void do_close_path(void) {
        print_cmd(VG_CLOSE_PATH);
    }

    void do_line_to_abs(float x1, float y1) {
        print_cmd(VG_LINE_TO_ABS, x1, y1);
    }

    void do_line_to_rel(float x1, float y1) {
        print_cmd(VG_LINE_TO_REL, x1, y1);
    }

    void do_hline_to_abs(float x1) {
        print_cmd(VG_HLINE_TO_ABS, x1);
    }

    void do_hline_to_rel(float x1) {
        print_cmd(VG_HLINE_TO_REL, x1);
    }

    void do_vline_to_abs(float y1) {
        print_cmd(VG_VLINE_TO_ABS, y1);
    }

    void do_vline_to_rel(float y1) {
        print_cmd(VG_VLINE_TO_REL, y1);
    }

    void do_quad_to_abs(float x1, float y1, float x2, float y2) {
        print_cmd(VG_QUAD_TO_ABS, x1, y1, x2, y2);
    }

    void do_quad_to_rel(float x1, float y1, float x2, float y2) {
        print_cmd(VG_QUAD_TO_REL, x1, y1, x2, y2);
    }

    void do_squad_to_abs(float x2, float y2) {
        print_cmd(VG_SQUAD_TO_ABS, x2, y2);
    }

    void do_squad_to_rel(float x2, float y2) {
        print_cmd(VG_SQUAD_TO_REL, x2, y2);
    }

    void do_rquad_to_abs(float x1, float y1, float w1, float x2, float y2) {
        (void) x1;
        (void) y1;
        (void) w1;
        (void) x2;
        (void) y2;
        //print_cmd('R', x1, y1, w1, x2, y2);
    }

    void do_rquad_to_rel(float x1, float y1, float w1, float x2, float y2) {
        (void) x1;
        (void) y1;
        (void) w1;
        (void) x2;
        (void) y2;
        //print_cmd('r', x1, y1, w1, x2, y2);
    }

    void do_svg_arc_to_abs(float rx, float ry, float a, float fa, float fs,
        float x2, float y2) {
        if (fa) {
            if (fs) {
                print_cmd(VG_LCCWARC_TO_ABS, rx, ry, a, x2, y2);
            } else {
                print_cmd(VG_LCWARC_TO_ABS, rx, ry, a, x2, y2);
            }
        } else {
            if (fs) {
                print_cmd(VG_SCCWARC_TO_ABS, rx, ry, a, x2, y2);
            } else {
                print_cmd(VG_SCWARC_TO_ABS, rx, ry, a, x2, y2);
            }
        }
    }

    void do_svg_arc_to_rel(float rx, float ry, float a, float fa, float fs,
        float x2, float y2) {
        if (fa) {
            if (fs) {
                print_cmd(VG_LCCWARC_TO_REL, rx, ry, a, x2, y2);
            } else {
                print_cmd(VG_LCWARC_TO_REL, rx, ry, a, x2, y2);
            }
        } else {
            if (fs) {
                print_cmd(VG_SCCWARC_TO_REL, rx, ry, a, x2, y2);
            } else {
                print_cmd(VG_SCWARC_TO_REL, rx, ry, a, x2, y2);
            }
        }
    }

    void do_cubic_to_abs(float x1, float y1, float x2, float y2,
    float x3, float y3) {
        print_cmd(VG_CUBIC_TO_ABS, x1, y1, x2, y2, x3, y3);
    }

    void do_cubic_to_rel(float x1, float y1, float x2, float y2,
    float x3, float y3) {
        print_cmd(VG_CUBIC_TO_REL, x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(float x2, float y2, float x3, float y3) {
        print_cmd(VG_SCUBIC_TO_ABS, x2, y2, x3, y3);
    }

    void do_scubic_to_rel(float x2, float y2, float x3, float y3) {
        print_cmd(VG_SCUBIC_TO_REL, x2, y2, x3, y3);
    }
};

svg_path_f_segments_data make_svg_path_f_segments_data(
    std::vector<uint8_t> &segments, std::vector<float> &data) {
	return svg_path_f_segments_data(segments, data);
}

class spy_rasterizer: public OpenVGRI::Rasterizer {

    path_data::ptr m_output_path;
    int m_nedges;
    OpenVGRI::Vector2 m_previous;

public:

    spy_rasterizer(path_data::ptr output_path):
        m_output_path(output_path), m_nedges(0) {}

    ~spy_rasterizer() {}

    void clear() {
        m_nedges = 0;
    }

    void addEdge(const OpenVGRI::Vector2& v0, const OpenVGRI::Vector2& v1) {
        if (m_nedges == 0) {
            m_output_path->begin_contour(v0.x, v0.y);
            m_previous = v0;
        }
        m_output_path->linear_segment(m_previous.x, m_previous.y, v1.x, v1.y);
        m_previous = v1;
        m_nedges++;
    }

    void fill() {
        if (m_nedges > 0) {
            m_output_path->end_closed_contour(m_previous.x, m_previous.y);
        }
    }
};


shape openvg_ri(const shape &input_shape,
    const xform &screen_xf, float width, stroke_style::const_ptr style) {
    std::vector<uint8_t> segments;
    std::vector<float> data;
    input_shape.as_path_data_ptr(input_shape.get_xf().
        transformed(screen_xf))->iterate(
            make_input_path_f_xform(input_shape.get_xf(),
                make_input_path_f_to_svg_path(
                    make_svg_path_f_segments_data(segments, data))));
    OpenVGRI::Array<float> vg_dashes;
    for (const auto &d: style->get_dashes())
        vg_dashes.push_back(d*width);
    VGCapStyle vg_cap = VG_CAP_BUTT;
    switch (style->get_initial_cap()) {
        case e_stroke_cap::butt:
            vg_cap = VG_CAP_BUTT;
            break;
        case e_stroke_cap::round:
            vg_cap = VG_CAP_ROUND;
            break;
        case e_stroke_cap::square:
            vg_cap = VG_CAP_SQUARE;
            break;
        default:
            break;
    }
    VGJoinStyle vg_join = VG_JOIN_MITER;
    switch (style->get_join()) {
        case e_stroke_join::bevel:
            vg_join = VG_JOIN_BEVEL;
            break;
        case e_stroke_join::round:
            vg_join = VG_JOIN_ROUND;
            break;
        case e_stroke_join::miter_clip: // openvg does not have clipped miters
        case e_stroke_join::miter_or_bevel: // only fallbacks to bevel
            vg_join = VG_JOIN_MITER;
            break;
        default:
            break;
    }
    OpenVGRI::Matrix3x3 id;
    OpenVGRI::Path vg_path(VG_PATH_FORMAT_STANDARD,
        VG_PATH_DATATYPE_F, 1, 0, 0, 0, 0);
    vg_path.addReference();
    vg_path.appendData(&segments[0], segments.size(),
        reinterpret_cast<uint8_t*>(&data[0]));
    auto output_path = make_intrusive<path_data>();
    spy_rasterizer raster(output_path);
    vg_path.stroke(id, raster, vg_dashes, style->get_dash_offset()*width,
        style->get_resets_on_move(), width, vg_cap, vg_join,
        style->get_miter_limit());
    vg_path.removeReference();

    return shape{output_path};
}

} }
