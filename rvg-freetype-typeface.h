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
#ifndef RVG_FREETYPE_TYPEFACE_H
#define RVG_FREETYPE_TYPEFACE_H

#include <tuple>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H
#include FT_BBOX_H

#include "rvg-i-typeface.h"
#include "rvg-ptr.h"
#include "rvg-image.h"
#include "rvg-floatint.h"
#include "rvg-svg-path-f-to-input-path.h"

#define RVG_FT_OUTLINE_FLAGS ( \
    FT_LOAD_IGNORE_TRANSFORM | \
    FT_LOAD_LINEAR_DESIGN | \
    FT_LOAD_NO_SCALE | \
    FT_LOAD_NO_BITMAP | \
    FT_LOAD_NO_HINTING | \
    FT_LOAD_NO_AUTOHINT )

#define RVG_FT_ALPHA_FLAGS ( \
    FT_LOAD_IGNORE_TRANSFORM | \
    FT_LOAD_LINEAR_DESIGN | \
    FT_LOAD_NO_BITMAP | \
    FT_LOAD_NO_HINTING | \
    FT_LOAD_NO_AUTOHINT )

namespace rvg {

class freetype_typeface final:
    public i_typeface<freetype_typeface>,
    public boost::intrusive_ref_counter<freetype_typeface> {

public:

    using ptr = boost::intrusive_ptr<freetype_typeface>;
    using const_ptr = boost::intrusive_ptr<const freetype_typeface>;

private:

    FT_Face m_face;

public:

    freetype_typeface(FT_Face face): m_face(face) {
        ;
    }

    ~freetype_typeface() {
        FT_Done_Face(m_face);
    }


private:

    template <typename T>
    static int moveto(const FT_Vector* p0, void *user) {
        auto &svg_sink = std::get<0>(*reinterpret_cast<T *>(user));
        auto &count = std::get<1>(*reinterpret_cast<T *>(user));
        if (count) {
            svg_sink.close_path();
        }
        svg_sink.move_to_abs((rvgf) p0->x, (rvgf) p0->y);
        count++;
        return 0;
    }

    template <typename T>
    static int lineto(const FT_Vector* p0, void *user) {
        auto &svg_sink = std::get<0>(*reinterpret_cast<T *>(user));
        auto &count = std::get<1>(*reinterpret_cast<T *>(user));
        svg_sink.line_to_abs((rvgf) p0->x, (rvgf) p0->y);
        count++;
        return 0;
    }

    template <typename T>
    static int quadto(const FT_Vector* p0, const FT_Vector *p1, void *user) {
        auto &svg_sink = std::get<0>(*reinterpret_cast<T *>(user));
        auto &count = std::get<1>(*reinterpret_cast<T *>(user));
        svg_sink.quad_to_abs((rvgf) p0->x, (rvgf)p0->y, (rvgf)p1->x, (rvgf)p1->y);
        count++;
        return 0;
    }

    template <typename T>
    static int cubicto(const FT_Vector* p0, const FT_Vector *p1,
        const FT_Vector *p2, void *user) {
        auto &svg_sink = std::get<0>(*reinterpret_cast<T *>(user));
        auto &count = std::get<1>(*reinterpret_cast<T *>(user));
        svg_sink.cubic_to_abs((rvgf)p0->x, (rvgf)p0->y, (rvgf)p1->x, (rvgf)p1->y, (rvgf)p2->x, (rvgf)p2->y);
        count++;
        return 0;
    }


// i_typeface interface
friend i_typeface<freetype_typeface>;

    std::pair<int, int> do_get_kerning(int left_idx, int right_idx) {
        if (FT_HAS_KERNING(m_face)) {
            FT_Vector delta;
            FT_Get_Kerning(m_face, left_idx, right_idx,
                FT_KERNING_UNSCALED, &delta);
            return std::make_pair(delta.x, delta.y);
        } else {
            return std::make_pair(0,0);
        }
    }

    int do_get_char_index(int ch) {
        return FT_Get_Char_Index(m_face, ch);
    }

    int do_get_attributes(face_attributes *fa) {
		fa->num_faces = m_face->num_faces;
		fa->face_index = m_face->face_index;
		fa->num_glyphs = m_face->num_glyphs;
		fa->family_name = m_face->family_name;
		fa->style_name = m_face->style_name;
		fa->units_per_EM = m_face->units_per_EM;
		fa->ascender = m_face->ascender;
		fa->descender = m_face->descender;
		fa->height = m_face->height;
		fa->max_advance_width = m_face->max_advance_width;
		fa->max_advance_height = m_face->max_advance_height;
		fa->underline_position = m_face->underline_position;
		fa->underline_thickness = m_face->underline_thickness;
		fa->bounding_box = bbox<int>(m_face->bbox.xMin, m_face->bbox.yMin,
            m_face->bbox.xMax, m_face->bbox.yMax);
        return 1;
    }

    int do_get_glyph_metrics(int idx, glyph_metrics *gm) {
        if (!FT_Load_Glyph(m_face, idx, RVG_FT_OUTLINE_FLAGS)) {
            gm->width = m_face->glyph->metrics.width;
            gm->height = m_face->glyph->metrics.height;
            gm->horiz_bearing_x = m_face->glyph->metrics.horiBearingX;
            gm->horiz_bearing_y = m_face->glyph->metrics.horiBearingY;
            gm->horiz_advance = m_face->glyph->metrics.horiAdvance;
            gm->vert_bearing_x = m_face->glyph->metrics.vertBearingX;
            gm->vert_bearing_y = m_face->glyph->metrics.vertBearingY;
            gm->vert_advance = m_face->glyph->metrics.vertAdvance;
            return 1;
        } else {
            return 0;
        }
    }

    int do_get_glyph_alpha(int idx, int pixels_per_EM, glyph_alpha *ga) {
        FT_Set_Pixel_Sizes(m_face, 0, pixels_per_EM);
        if (!FT_Load_Glyph(m_face, idx, RVG_FT_ALPHA_FLAGS) &&
            !FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL)) {
            FT_Bitmap *bitmap = &m_face->glyph->bitmap;
            int width = bitmap->width;
            int height = bitmap->rows;
            int pitch = bitmap->pitch;
            auto alpha = make_intrusive<image<uint8_t,1>>();
            alpha->load_from(width, width, height, -pitch, 1,
                bitmap->buffer+(height-1)*pitch);
            ga->alpha = alpha;
            ga->left = m_face->glyph->bitmap_left;
            ga->top = m_face->glyph->bitmap_top;
            ga->bottom = ga->top - height;
            ga->pixels_per_EM = pixels_per_EM;
            return 1;
        } else {
            return 0;
        }
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    int do_iterate_glyph_outline(int idx, PF &&sink) {
        if (!FT_Load_Glyph(m_face, idx, RVG_FT_OUTLINE_FLAGS)) {
            auto svg_sink = make_svg_path_f_to_input_path(
                std::forward<PF>(sink));
            int count = 0;
            auto user = std::forward_as_tuple(svg_sink, count);
            using T = decltype(user);
            static FT_Outline_Funcs funcs{
                &moveto<T>,
                &lineto<T>,
                &quadto<T>,
                &cubicto<T>,
                0, 0
            };
            FT_Outline_Decompose(&m_face->glyph->outline, &funcs, &user);
            if (count) {
                svg_sink.close_path();
            }
            return 1;
        } else {
            return 0;
        }
    }
};

}

#endif
