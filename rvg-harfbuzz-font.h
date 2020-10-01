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
#ifndef RVG_HARFBUZZ_FONT_H
#define RVG_HARFBUZZ_FONT_H

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include "rvg-ptr.h"

namespace rvg {

class harfbuzz_font:
    public boost::intrusive_ref_counter<harfbuzz_font> {

    hb_font_t *m_font;

    static char *load_file(const char *name, size_t *len) {
        FILE *fp = fopen(name, "rb");
        if (!fp) return nullptr;
        fseek(fp, 0, SEEK_END);
        *len = ftell(fp);
        char *buf = (char *) malloc(*len+1);
        if (!buf) return nullptr;
        fseek(fp, 0, SEEK_SET);
        size_t got = fread(buf, 1, *len, fp);
        buf[*len] = '\0';
        fclose(fp);
        if (got != *len) {
            *len = 0;
            free(buf);
            return nullptr;
        } else {
            return buf;
        }
    }

public:

    using ptr = boost::intrusive_ptr<harfbuzz_font>;
    using const_ptr = boost::intrusive_ptr<const harfbuzz_font>;

    harfbuzz_font(const char *filename, int face_index):
        m_font(hb_font_get_empty()) {
        size_t len = 0;
        char *data = load_file(filename, &len);
        if (data) {
            hb_blob_t *blob = hb_blob_create(data, static_cast<int>(len),
                HB_MEMORY_MODE_WRITABLE, data, (hb_destroy_func_t) free);
            if (blob != hb_blob_get_empty()) {
                hb_face_t *face = hb_face_create(blob, face_index);
                hb_blob_destroy(blob);
                if (face != hb_face_get_empty()) {
                    m_font = hb_font_create(face);
                    hb_face_destroy(face);
                }
            }
        }
    }

    ~harfbuzz_font() {
        if (valid()) {
            hb_font_destroy(m_font);
        }
    }

    int valid(void) const {
        return m_font != hb_font_get_empty();
    }

    hb_font_t *get(void) {
        return m_font;
    }

    void get_glyph_name(int codepoint, char *name, int len) {
        hb_font_glyph_to_string(m_font, codepoint, name, len);
    }

    void set_scale(int s) {
        hb_font_set_scale(m_font, s, s);
    }

    void set_scale(int sx, int sy) {
        hb_font_set_scale(m_font, sx, sy);
    }

    void set_ft_funcs(void) {
        hb_ft_font_set_funcs(m_font);
    }

    void set_ot_funcs(void) {
        hb_ot_font_set_funcs(m_font);
    }

    int get_units_per_EM(void) {
        hb_face_t *face = hb_font_get_face(m_font);
        return hb_face_get_upem(face);
    }

    void set_units_per_EM(int units_per_EM) {
        hb_face_t *face = hb_font_get_face(m_font);
        hb_face_set_upem(face, units_per_EM);
    }
};

}

#endif
