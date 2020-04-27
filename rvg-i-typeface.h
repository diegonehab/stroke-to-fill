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
#ifndef RVG_I_TYPEFACE_H
#define RVG_I_TYPEFACE_H

#include "rvg-meta.h"
#include "rvg-bbox.h"
#include "rvg-image.h"
#include "rvg-i-path.h"

#include <cstdint>
#include <string>

namespace rvg {

typedef struct {
    int pixels_per_EM;
    int left, top, bottom;
    image<uint8_t,1>::ptr alpha;
} glyph_alpha;

typedef struct {
    int width, height;
    int horiz_bearing_x, horiz_bearing_y, horiz_advance;
    int vert_bearing_x, vert_bearing_y, vert_advance;
} glyph_metrics;

typedef struct {
    int num_faces;
    int face_index;
    int num_glyphs;
    std::string family_name;
    std::string style_name;
    int units_per_EM;
    int ascender;
    int descender;
    int height;
    int max_advance_width;
    int max_advance_height;
    int underline_position;
    int underline_thickness;
    bbox<int> bounding_box;
} face_attributes;

template <typename DERIVED>
class i_typeface {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    std::pair<int, int> get_kerning(int left, int right) {
        return derived().do_get_kerning(left, right);
    }

    int get_char_index(int ch) {
        return derived().do_get_char_index(ch);
    }

    int get_attributes(face_attributes *fa) {
        return derived().do_get_attributes(fa);
    }

    int get_glyph_metrics(int idx, glyph_metrics *gm) {
        return derived().do_get_glyph_metrics(idx, gm);
    }

    int get_glyph_alpha(int idx, int pixels_per_EM, glyph_alpha *ga) {
        return derived().do_get_glyph_alpha(idx, pixels_per_EM, ga);
    }

    template <typename PF, typename =
        std::enable_if<rvg::meta::is_an_i_path<PF>::value>>
    int iterate_glyph_outline(int idx, PF &&sink) {
        return derived().do_iterate_glyph_outline(idx,
            std::forward<PF>(sink));
    }
};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_typeface = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_typeface,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
