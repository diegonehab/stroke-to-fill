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
#ifndef RVG_I_DECORATED_PATH_H
#define RVG_I_DECORATED_PATH_H

#include "rvg-floatint.h"
#include "rvg-meta.h"
#include "rvg-point.h"

namespace rvg {

template <typename DERIVED>
class i_decorated_path {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    void initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_initial_cap(x, y, dx, dy);
    }

    void backward_initial_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_backward_initial_cap(x, y, dx, dy);
    }

    void initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_initial_butt_cap(x, y, dx, dy);
    }

    void backward_initial_butt_cap(rvgf x, rvgf y, rvgf dx, rvgf dy) {
        return derived().do_backward_initial_butt_cap(x, y, dx, dy);
    }

    void terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_terminal_cap(dx, dy, x, y);
    }

    void backward_terminal_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_backward_terminal_cap(dx, dy, x, y);
    }

    void terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_terminal_butt_cap(dx, dy, x, y);
    }

    void backward_terminal_butt_cap(rvgf dx, rvgf dy, rvgf x, rvgf y) {
        return derived().do_backward_terminal_butt_cap(dx, dy, x, y);
    }

    void join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1, rvgf w) {
        return derived().do_join(dx0, dy0, x, y, dx1, dy1, w);
    }

    void inner_join(rvgf dx0, rvgf dy0, rvgf x, rvgf y, rvgf dx1, rvgf dy1,
        rvgf w) {
        return derived().do_inner_join(dx0, dy0, x, y, dx1, dy1, w);
    }

    void initial_cap(const R2 &p, const R2 &d) {
        return initial_cap(p[0], p[1], d[0], d[1]);
    }

    void terminal_cap(const R2 &d, const R2 &p) {
        return terminal_cap(d[0], d[1], p[0], p[1]);
    }

    void backward_initial_cap(const R2 &p, const R2 &d) {
        return backward_initial_cap(p[0], p[1], d[0], d[1]);
    }

    void backward_terminal_cap(const R2 &d, const R2 &p) {
        return backward_terminal_cap(d[0], d[1], p[0], p[1]);
    }

    void initial_butt_cap(const R2 &p, const R2 &d) {
        return initial_butt_cap(p[0], p[1], d[0], d[1]);
    }

    void terminal_butt_cap(const R2 &d, const R2 &p) {
        return terminal_butt_cap(d[0], d[1], p[0], p[1]);
    }

    void backward_initial_butt_cap(const R2 &p, const R2 &d) {
        return backward_initial_butt_cap(p[0], p[1], d[0], d[1]);
    }

    void backward_terminal_butt_cap(const R2 &d, const R2 &p) {
        return backward_terminal_butt_cap(d[0], d[1], p[0], p[1]);
    }

    void join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return join(d0[0], d0[1], p[0], p[1], d1[0], d1[1], w);
    }

    void inner_join(const R2 &d0, const R2 &p, const R2 &d1, rvgf w) {
        return inner_join(d0[0], d0[1], p[0], p[1], d1[0], d1[1], w);
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_decorated_path = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_decorated_path,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

namespace detail {
	class not_an_i_decorated_path { };
}

template <typename U, typename T>
using inherit_if_i_decorated_path =
	typename std::conditional<
		is_an_i_decorated_path<U>::value,
		T,
		detail::not_an_i_decorated_path
	>::type;


} } // namespace rvg::meta

#endif
