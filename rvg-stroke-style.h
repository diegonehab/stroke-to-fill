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
#ifndef RVG_STROKE_STYLE_H
#define RVG_STROKE_STYLE_H

#include "rvg-ptr.h"

#include "rvg-stroke-cap.h"
#include "rvg-stroke-join.h"
#include "rvg-stroke-dashes.h"

// ??D The dashes should be proportional to the stroke width,
//     so we can reuse a given dash style
//     Add ref qualifiers to all methods so we don't need to
//     can modify in-place in case the *this is an r-value ref
namespace rvg {

class stroke_style final:
    public boost::intrusive_ref_counter<stroke_style> {
public:

    using ptr = boost::intrusive_ptr<stroke_style>;
    using const_ptr = boost::intrusive_ptr<const stroke_style>;

    static constexpr e_stroke_cap default_cap = e_stroke_cap::butt;
    static constexpr e_stroke_join default_join = e_stroke_join::miter_or_bevel;
    static constexpr e_stroke_join default_inner_join = e_stroke_join::round;
    static constexpr float default_miter_limit = 4.f;
    static constexpr bool default_resets_on_move = false;
    static constexpr float default_dash_offset = 0.f;

    static const stroke_dashes::const_ptr default_dashes_ptr;

private:

    e_stroke_cap m_initial_cap, m_terminal_cap, m_dash_initial_cap, m_dash_terminal_cap;
    e_stroke_join m_join, m_inner_join;
    bool m_resets_on_move;
    float m_miter_limit, m_dash_offset;
    stroke_dashes::const_ptr m_dashes_ptr;

public:

    stroke_style(void):
        m_initial_cap(default_cap),
        m_terminal_cap(default_cap),
        m_dash_initial_cap(default_cap),
        m_dash_terminal_cap(default_cap),
        m_join(default_join),
        m_inner_join(default_inner_join),
        m_resets_on_move(default_resets_on_move),
        m_miter_limit(default_miter_limit),
        m_dash_offset(default_dash_offset),
        m_dashes_ptr(default_dashes_ptr) {
            // There is no way to know that the static default_dashes_ptr
            // member has been initialized in rvg-stroke-style.cpp
            // before some other module tries to construct a
            // stroke_style object while initializing one of its
            // own static members. So we need to guard for
            // the possibility that default_dashes_ptr is
            // still zero-initialized here
            if (!m_dashes_ptr) {
                m_dashes_ptr = make_intrusive<stroke_dashes>();
            }
        }

    stroke_dashes::const_ptr get_dashes_ptr(void) const {
        return m_dashes_ptr;
    }

    const stroke_dashes &get_dashes(void) const {
        return *m_dashes_ptr;
    }

    e_stroke_join get_join(void) const {
        return m_join;
    }

    e_stroke_join get_inner_join(void) const {
        return m_inner_join;
    }

    e_stroke_cap get_initial_cap(void) const {
        return m_initial_cap;
    }

    e_stroke_cap get_terminal_cap(void) const {
        return m_terminal_cap;
    }

    e_stroke_cap get_dash_terminal_cap(void) const {
        return m_dash_terminal_cap;
    }

    e_stroke_cap get_dash_initial_cap(void) const {
        return m_dash_initial_cap;
    }

    float get_miter_limit(void) const {
        return m_miter_limit;
    }

    float get_dash_offset(void) const {
        return m_dash_offset;
    }

    bool get_resets_on_move(void) const {
        return m_resets_on_move;
    }

    stroke_style dashed(const stroke_dashes &dashes) const {
        stroke_style copy = *this;
        copy.m_dashes_ptr = make_intrusive<stroke_dashes>(dashes);
        return copy;
    }

    stroke_style reset_on_move(bool value) const {
        stroke_style copy = *this;
        copy.m_resets_on_move = value;
        return copy;
    }

    stroke_style dash_offset(float value) const {
        stroke_style copy = *this;
        copy.m_dash_offset = value;
        return copy;
    }

    stroke_style capped(e_stroke_cap cap) const {
        stroke_style copy = *this;
        copy.m_initial_cap = cap;
        copy.m_dash_initial_cap = cap;
        copy.m_terminal_cap = cap;
        copy.m_dash_terminal_cap = cap;
        return copy;
    }

    stroke_style initial_capped(e_stroke_cap cap) const {
        stroke_style copy = *this;
        copy.m_initial_cap = cap;
        return copy;
    }

    stroke_style terminal_capped(e_stroke_cap cap) const {
        stroke_style copy = *this;
        copy.m_terminal_cap = cap;
        return copy;
    }

    stroke_style dash_initial_capped(e_stroke_cap cap) const {
        stroke_style copy = *this;
        copy.m_dash_initial_cap = cap;
        return copy;
    }

    stroke_style dash_terminal_capped(e_stroke_cap cap) const {
        stroke_style copy = *this;
        copy.m_dash_terminal_cap = cap;
        return copy;
    }

    stroke_style joined(e_stroke_join join) const {
        stroke_style copy = *this;
        copy.m_join = join;
        return copy;
    }

    stroke_style inner_joined(e_stroke_join inner_join) const {
        stroke_style copy = *this;
        copy.m_inner_join = inner_join;
        return copy;
    }

    stroke_style miter_limited(float miter_limit) const {
        stroke_style copy = *this;
        copy.m_miter_limit = miter_limit;
        return copy;
    }
};

} // namespace rvg

#endif
