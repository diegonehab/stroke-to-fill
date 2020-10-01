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
#ifndef RVG_HARFBUZZ_BUFFER_H
#define RVG_HARFBUZZ_BUFFER_H

#include <hb.h>
#include <hb-icu.h>

#include "rvg-ptr.h"
#include "rvg-harfbuzz-features.h"

namespace rvg {

class harfbuzz_buffer: public boost::intrusive_ref_counter<harfbuzz_buffer> {

    hb_buffer_t *m_buffer;

public:

    using ptr = boost::intrusive_ptr<harfbuzz_buffer>;
    using const_ptr = boost::intrusive_ptr<const harfbuzz_buffer>;

    harfbuzz_buffer() {
        m_buffer = hb_buffer_create();
        if (m_buffer != hb_buffer_get_empty()) {
            hb_buffer_set_unicode_funcs(m_buffer, hb_icu_get_unicode_funcs());
            hb_buffer_set_direction(m_buffer, HB_DIRECTION_LTR); // default
        }
    }

    ~harfbuzz_buffer() {
        if (valid()) {
            hb_buffer_destroy(m_buffer);
        }
    }

    int valid(void) const {
        return m_buffer != hb_buffer_get_empty();
    }

    void set_direction(const char *dir_str, int len) {
        hb_direction_t dir = hb_direction_from_string(dir_str, len);
        if (dir != HB_DIRECTION_INVALID)
            hb_buffer_set_direction(m_buffer, dir);
    }

    void set_direction(hb_direction_t dir) {
        hb_buffer_set_direction(m_buffer, dir);
    }

    void set_script(hb_script_t script) {
        hb_buffer_set_script(m_buffer, script);
    }

    void set_script(const char *script_str, int len) {
        hb_script_t script = hb_script_from_string(script_str, len);
        if (script != HB_SCRIPT_INVALID)
            hb_buffer_set_script(m_buffer, script);
    }

    void set_language(const char *lang_str, int len) {
        hb_language_t lang = hb_language_from_string(lang_str, len);
        if (lang != HB_LANGUAGE_INVALID)
            hb_buffer_set_language(m_buffer, lang);
    }

    void set_language(hb_language_t lang) {
        hb_buffer_set_language(m_buffer, lang);
    }

    void set_cluster_level(hb_buffer_cluster_level_t level) {
        hb_buffer_set_cluster_level(m_buffer, level);
    }

    void guess_segment_properties(void) {
        hb_buffer_guess_segment_properties(m_buffer);
    }

    void add(const char *data, int data_len, int text_os, int text_len) {
        hb_buffer_add_utf8(m_buffer, data, data_len, text_os, text_len);
    }

    void add(const char *data) {
        add(data, -1, 0, -1);
    }

    void reset(void) {
        hb_buffer_reset(m_buffer);
    }

    void shape(harfbuzz_font::ptr font, harfbuzz_features::const_ptr feat) {
        if (feat && !feat->empty()) {
            int len = static_cast<int>(feat->size());
            hb_shape(font->get(), m_buffer, &(*feat)[0], len);
        } else {
            hb_shape(font->get(), m_buffer, nullptr, 0);
        }
    }

    void shape(harfbuzz_font::ptr font) {
        hb_shape(font->get(), m_buffer, nullptr, 0);
    }

    void normalize_glyphs(void) {
        hb_buffer_normalize_glyphs(m_buffer);
    }

    hb_glyph_info_t* get_glyph_infos(unsigned *length) {
        return hb_buffer_get_glyph_infos(m_buffer, length);
    }

    hb_glyph_position_t* get_glyph_positions(unsigned *length) {
        return hb_buffer_get_glyph_positions(m_buffer, length);
    }

    hb_buffer_t *get(void) const {
        return m_buffer;
    }
};

}

#endif
