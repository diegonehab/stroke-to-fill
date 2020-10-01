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
#ifndef RVG_I_POINT_DECORATED_PATH_F_FORWARDER_H
#define RVG_I_POINT_DECORATED_PATH_F_FORWARDER_H

#include "rvg-i-point-decorated-path.h"

namespace rvg {

template <typename DERIVED>
class i_point_decorated_path_f_forwarder: public i_point_decorated_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

// i_point_decorated_path interface
friend i_point_decorated_path<DERIVED>;

    void do_initial_cap(const R2 &p, const R2 &d) {
        return derived().sink().initial_cap(p, d);
    }

    void do_terminal_cap(const R2 &d, const R2 &p) {
        return derived().sink().terminal_cap(d, p);
    }

    void do_backward_initial_cap(const R2 &p, const R2 &d) {
        return derived().sink().backward_initial_cap(p, d);
    }

    void do_backward_terminal_cap(const R2 &d, const R2 &p) {
        return derived().sink().backward_terminal_cap(d, p);
    }

    void do_initial_butt_cap(const R2 &p, const R2 &d) {
        return derived().sink().initial_butt_cap(p, d);
    }

    void do_terminal_butt_cap(const R2 &d, const R2 &p) {
        return derived().sink().terminal_butt_cap(d, p);
    }

    void do_backward_initial_butt_cap(const R2 &p, const R2 &d) {
        return derived().sink().backward_initial_butt_cap(p, d);
    }

    void do_backward_terminal_butt_cap(const R2 &d, const R2 &p) {
        return derived().sink().backward_terminal_butt_cap(d, p);
    }

    void do_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return derived().sink().join(d0, p, d1, w);
    }

    void do_inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return derived().sink().inner_join(d0, p, d1, w);
    }

};

} // namespace rvg

#endif
