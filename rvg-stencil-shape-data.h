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
#ifndef RVG_STENCIL_SHAPE_DATA_H
#define RVG_STENCIL_SHAPE_DATA_H

#include "rvg-paint.h"
#include "rvg-shape.h"

namespace rvg {

class stencil_shape_data {

    e_winding_rule m_winding_rule;
    shape::const_ptr m_shape_ptr;

public:
    stencil_shape_data(e_winding_rule winding_rule,
        shape::const_ptr shape_ptr):
        m_winding_rule{winding_rule},
        m_shape_ptr{shape_ptr} {
        ;
    }

    e_winding_rule get_winding_rule(void) const {
        return m_winding_rule;
    }

    const shape &get_shape(void) const {
        return *m_shape_ptr;
    }

    shape::const_ptr get_shape_ptr(void) const {
        return m_shape_ptr;
    }

};

} // namespace rvg

#endif
