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
#ifndef RVG_REGULAR_PATH_F_ORIENT_H
#define RVG_REGULAR_PATH_F_ORIENT_H

#include "rvg-i-point-regular-path.h"
#include "rvg-i-monotonic-parameters-f-forwarder.h"
#include "rvg-i-offsetting-parameters-f-forwarder.h"
#include "rvg-i-cubic-parameters-f-forwarder.h"
#include "rvg-path-data.h"

namespace rvg {

template <typename SINK>
class regular_path_f_orient final:
    public i_sink<regular_path_f_orient<SINK>>,
    public i_monotonic_parameters_f_forwarder<regular_path_f_orient<SINK>>,
    public i_cubic_parameters_f_forwarder<regular_path_f_orient<SINK>>,
    public i_offsetting_parameters_f_forwarder<regular_path_f_orient<SINK>>,
    public i_regular_path_f_forwarder<regular_path_f_orient<SINK>> {

    path_data m_saved;

    SINK m_sink;

public:

    explicit regular_path_f_orient(SINK &&sink):
       m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_i_regular_path<SINK>::value,
            "sink is not an i_regular_path");
    }

private:

friend i_sink<regular_path_f_orient<SINK>>;

    path_data &do_sink(void) {
        return m_saved; // so all forwarders send to m_saved
    }

    const path_data &do_sink(void) const {
        return m_saved; // so all forwarders send to m_saved
    }

friend i_regular_path<regular_path_f_orient<SINK>>;

    void do_begin_regular_contour(float x, float y, float dx, float dy) {
        m_saved.begin_regular_contour(x, y, dx, dy);
    }

    void do_end_regular_open_contour(float dx, float dy, float x, float y) {
        m_saved.end_regular_open_contour(dx, dy, x, y);
        m_saved.propagate_orientations();
        m_saved.iterate(m_sink);
        m_saved.clear();
    }

    void do_end_regular_closed_contour(float dx, float dy, float x, float y) {
        m_saved.end_regular_closed_contour(dx, dy, x, y);
        m_saved.propagate_orientations();
        m_saved.iterate(m_sink);
        m_saved.clear();
    }
};

template <typename SINK>
static auto
make_regular_path_f_orient(SINK &&sink) {
    return regular_path_f_orient<SINK>{std::forward<SINK>(sink)};
}

} // namespace rvg

#endif
