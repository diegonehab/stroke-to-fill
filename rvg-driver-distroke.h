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
#ifndef RVG_DRIVER_DISTROKE_H
#define RVG_DRIVER_DISTROKE_H

#include <cstdio>
#include <vector>
#include <memory>

#include "rvg-window.h"
#include "rvg-viewport.h"
#include "rvg-scene.h"

namespace rvg {
    namespace driver {
        namespace distroke {

template <typename DERIVED>
class i_accelerated {

protected:

    DERIVED &derived(void);

    const DERIVED &derived(void) const;

public:

    void color(RGBA8 c);

    void transform(const xform &xf);

    void width(rvgf w);

    void miter_limit(rvgf m);

    void linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1);

    void quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1, rvgf x2, rvgf y2);

    void cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3);

    void rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2);

    void round_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void bevel_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void miter_clip_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void round_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);

    void square_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);

    void triangle_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);
};

class accelerated final:
    public i_accelerated<accelerated> {

public:

    accelerated(void);

    template <typename F>
    void iterate(F &sink) const;

    template <typename F>
    void iterate(F &&sink) const;

private:

	enum class e_type {
        color,
        transform,
        width,
        miter_limit,
        linear_segment_piece,
        quadratic_segment_piece,
        rational_quadratic_segment_piece,
        cubic_segment_piece,
        round_join,
        bevel_join,
        miter_clip_join,
        miter_or_bevel_join,
        round_cap,
        square_cap,
        triangle_cap,
	};

    std::shared_ptr<std::vector<e_type>> m_instructions_ptr;
    std::shared_ptr<std::vector<rvgf>> m_data_ptr;

    std::vector<e_type> &m_instructions;
    std::vector<rvgf> &m_data;

    void push_data(void);

    template <typename ...REST>
    void push_data(rvgf first, REST ...rest);

    friend i_accelerated<accelerated>;

    void do_color(RGBA8 c);

    void do_transform(const xform &xf);

    void do_width(rvgf w);

    void do_miter_limit(rvgf m);

    void do_linear_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1);

    void do_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0,
        rvgf x1, rvgf y1, rvgf x2, rvgf y2);

    void do_cubic_segment_piece(rvgf ti, rvgf tf, rvgf x0, rvgf y0, rvgf x1,
        rvgf y1, rvgf x2, rvgf y2, rvgf x3, rvgf y3);

    void do_rational_quadratic_segment_piece(rvgf ti, rvgf tf, rvgf x0,
        rvgf y0, rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2);

    void do_round_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void do_bevel_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void do_miter_clip_join(rvgf nx0, rvgf ny0, rvgf x, rvgf y, rvgf nx1, rvgf ny1);

    void do_round_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);

    void do_square_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);

    void do_triangle_cap(rvgf x, rvgf y, rvgf nx, rvgf ny);

};

accelerated accelerate(const scene &c, const window &w, const viewport &v);

void render(const accelerated &a, const window &w, const viewport &v,
    FILE *out, const std::vector<std::string> &args =
        std::vector<std::string>());

} } } // namespace rvg::driver::distroke

#endif
