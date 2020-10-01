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
#ifndef RVG_I_DECORATED_PATH_F_FORWARDER_H
#define RVG_I_DECORATED_PATH_F_FORWARDER_H

#include "rvg-i-decorated-path.h"

namespace rvg {

template <typename DERIVED>
class i_decorated_path_f_forwarder: public i_decorated_path<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

// i_decorated_path interface
friend i_decorated_path<DERIVED>;

    void do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().sink().initial_cap(x, y, dx, dy);
    }

    void do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().sink().backward_initial_cap(x, y, dx, dy);
    }

    void do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().sink().initial_butt_cap(x, y, dx, dy);
    }

    void do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().sink().backward_initial_butt_cap(x, y, dx, dy);
    }

    void do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().sink().terminal_cap(dx, dy, x, y);
    }

    void do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().sink().backward_terminal_cap(dx, dy, x, y);
    }

    void do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().sink().terminal_butt_cap(dx, dy, x, y);
    }

    void do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().sink().backward_terminal_butt_cap(dx, dy, x, y);
    }

    void do_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        return derived().sink().join(dx0, dy0, x, y, dx1, dy1, w);
    }

    void do_inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        return derived().sink().inner_join(dx0, dy0, x, y, dx1, dy1, w);
    }

};

} // namespace rvg

#endif
