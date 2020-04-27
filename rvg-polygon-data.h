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
#ifndef RVG_POLYGON_DATA_H
#define RVG_POLYGON_DATA_H

#include <vector>
#include <type_traits>
#include <iterator>

#include "rvg-ptr.h"
#include "rvg-path-data.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-xform.h"
#include "rvg-meta.h"


namespace rvg {


namespace meta {
    template <typename IT>
    using is_path_float_iterator = std::integral_constant<bool,
        std::is_base_of<std::input_iterator_tag,
        typename std::iterator_traits<IT>::iterator_category>::value &&
        std::is_same<typename std::iterator_traits<IT>::value_type, rvgf>::value>;
}

class polygon_data:
    public boost::intrusive_ref_counter<polygon_data> {

    // ??D change this to an array of R2 vertices?
    // Also, turn this into a  template that can be either
    // an open polyline or closed polygon?
    std::vector<rvgf> m_coordinates;

public:

    using ptr = boost::intrusive_ptr<polygon_data>;
    using const_ptr = boost::intrusive_ptr<const polygon_data>;

    polygon_data(const std::initializer_list<rvgf> &coordinates):
        m_coordinates(coordinates) { ; }

    template <typename SIT>
    polygon_data(SIT &begin, const SIT &end, typename std::enable_if<
		meta::is_path_float_iterator<SIT>::value>::type * = nullptr):
        m_coordinates(begin, end) { ; }

    const std::vector<rvgf> get_coordinates(void) const
        { return m_coordinates; }

    path_data::ptr as_path_data_ptr(const xform &post_xf) const {
        (void) post_xf;
        auto p = make_intrusive<path_data>();
        auto f = make_input_path_f_close_contours(*p);
        auto n = m_coordinates.size()/2;
        if (n >= 1) {
            f.begin_contour(m_coordinates[0], m_coordinates[1]);
            for (unsigned i = 0; i < n-1; ++i) {
                f.linear_segment(m_coordinates[2*i], m_coordinates[2*i+1],
                    m_coordinates[2*i+2], m_coordinates[2*i+3]);
            }
            f.end_closed_contour(m_coordinates[2*(n-1)],
                m_coordinates[2*(n-1)+1]);
        }
        return p;
    }
};

} // namespace rvg

#endif
