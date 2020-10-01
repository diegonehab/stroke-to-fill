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
#ifndef RVG_I_STROKABLE_H
#define RVG_I_STROKABLE_H

#include "rvg-stroke-style.h"

namespace rvg {

template <typename DERIVED>
class i_strokable {
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    DERIVED stroked(float width) const {
        return derived().do_stroked(width);
    }

    DERIVED stroked(float width, stroke_style::const_ptr style) const {
        return derived().do_stroked(width, style);
    }

    DERIVED dashed(const stroke_dashes &dashes) const {
        return derived().do_dashed(dashes);
    }

    DERIVED reset_on_move(bool value = true) const {
        return derived().do_reset_on_move(value);
    }

    DERIVED dash_offset(float value) const {
        return derived().do_dash_offset(value);
    }

    DERIVED capped(e_stroke_cap cap) const {
        return derived().do_capped(cap);
    }

    DERIVED initial_capped(e_stroke_cap cap) const {
        return derived().do_initial_capped(cap);
    }

    DERIVED dash_initial_capped(e_stroke_cap cap) const {
        return derived().do_dash_initial_capped(cap);
    }

    DERIVED terminal_capped(e_stroke_cap cap) const {
        return derived().do_terminal_capped(cap);
    }

    DERIVED dash_terminal_capped(e_stroke_cap cap) const {
        return derived().do_dash_terminal_capped(cap);
    }

    DERIVED joined(e_stroke_join join) const {
        return derived().do_joined(join);
    }

    DERIVED inner_joined(e_stroke_join inner_join) const {
        return derived().do_inner_joined(inner_join);
    }

    DERIVED miter_limited(float miter_limit) const {
        return derived().do_miter_limited(miter_limit);
    }

};

} // namespace rvg


#endif
