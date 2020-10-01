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
#ifndef RVG_LINEAR_GRADIENT_DATA_H
#define RVG_LINEAR_GRADIENT_DATA_H

#include "rvg-ptr.h"
#include "rvg-color-ramp.h"

namespace rvg {

class linear_gradient_data:
    public boost::intrusive_ref_counter<linear_gradient_data> {
private:
    color_ramp::const_ptr m_color_ramp_ptr;
    float m_x1, m_y1, m_x2, m_y2;
public:
    using ptr = boost::intrusive_ptr<linear_gradient_data>;
    using const_ptr = boost::intrusive_ptr<const linear_gradient_data>;

    linear_gradient_data(color_ramp::const_ptr color_ramp_ptr,
        float x1, float y1, float x2, float y2):
        m_color_ramp_ptr(color_ramp_ptr),
        m_x1(x1), m_y1(y1),
        m_x2(x2), m_y2(y2) { ; }

    color_ramp::const_ptr get_color_ramp_ptr(void) const {
        return m_color_ramp_ptr;
    }

    const color_ramp &get_color_ramp(void) const {
        return *m_color_ramp_ptr;
    }

    float get_x1(void) const {
        return m_x1;
    }

    float get_y1(void) const {
        return m_y1;
    }

    float get_x2(void) const {
        return m_x2;
    }

    float get_y2(void) const {
        return m_y2;
    }
};


} // namespace rvg

#endif
