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
#ifndef RVG_I_POINT_DECORATED_PATH_H
#define RVG_I_POINT_DECORATED_PATH_H

#include "rvg-meta.h"
#include "rvg-i-decorated-path.h"
#include "rvg-point.h"

namespace rvg {

template <typename DERIVED>
class i_point_decorated_path:
    public i_decorated_path<i_point_decorated_path<DERIVED>> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

    using base = i_decorated_path<i_point_decorated_path<DERIVED>>;

public:

    void initial_cap(const R2 &p, const R2 &d) {
        return derived().do_initial_cap(p, d);
    }

    void terminal_cap(const R2 &d, const R2 &p) {
        return derived().do_terminal_cap(d, p);
    }

    void backward_initial_cap(const R2 &p, const R2 &d) {
        return derived().do_backward_initial_cap(p, d);
    }

    void backward_terminal_cap(const R2 &d, const R2 &p) {
        return derived().do_backward_terminal_cap(d, p);
    }

    void initial_butt_cap(const R2 &p, const R2 &d) {
        return derived().do_initial_butt_cap(p, d);
    }

    void terminal_butt_cap(const R2 &d, const R2 &p) {
        return derived().do_terminal_butt_cap(d, p);
    }

    void backward_initial_butt_cap(const R2 &p, const R2 &d) {
        return derived().do_backward_initial_butt_cap(p, d);
    }

    void end_rbutt_cap(const R2 &d, const R2 &p) {
        return derived().do_backward_terminal_butt_cap(d, p);
    }

    void join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return derived().do_join(d0, p, d1, w);
    }

    void inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return derived().do_inner_join(d0, p, d1, w);
    }

    using base::initial_cap;
    using base::terminal_cap;
    using base::backward_initial_cap;
    using base::backward_terminal_cap;
    using base::initial_butt_cap;
    using base::terminal_butt_cap;
    using base::backward_initial_butt_cap;
    using base::backward_terminal_butt_cap;
    using base::join;
    using base::inner_join;

protected:

friend base;

    void do_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_initial_cap(make_R2(x, y), make_R2(dx, dy));
    }

    void do_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_terminal_cap(make_R2(dx, dy), make_R2(x, y));
    }

    void do_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_initial_butt_cap(make_R2(x, y), make_R2(dx, dy));
    }

    void do_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_terminal_butt_cap(make_R2(dx, dy), make_R2(x, y));
    }

    void do_backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_backward_initial_cap(make_R2(x, y), make_R2(dx, dy));
    }

    void do_backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_backward_terminal_cap(make_R2(dx, dy), make_R2(x, y));
    }

    void do_backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_backward_initial_butt_cap(make_R2(x, y), make_R2(dx, dy));
    }

    void do_backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_backward_terminal_butt_cap(make_R2(dx, dy), make_R2(x, y));
    }

    void do_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        return derived().do_join(make_R2(dx0, dy0), make_R2(x, y),
            make_R2(dx1, dy1), w);
    }

    void do_inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1,
        rvgf dy1, rvgf w) {
        return derived().do_inner_join(make_R2(dx0, dy0), make_R2(x, y),
            make_R2(dx1, dy1), w);
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_point_decorated_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_point_decorated_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
