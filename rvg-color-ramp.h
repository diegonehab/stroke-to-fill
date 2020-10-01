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
#ifndef RVG_COLOR_RAMP_H
#define RVG_COLOR_RAMP_H

#include <vector>
#include <type_traits>
#include <iterator>

#include "rvg-ptr.h"
#include "rvg-meta.h"
#include "rvg-rgba.h"
#include "rvg-spread.h"

namespace rvg {

class color_stop {
    float m_offset;
    RGBA8 m_color;
public:
    color_stop(void): m_offset(0.f), m_color(RGBA8()) { ; }
    color_stop(float offset, RGBA8 color):
        m_offset(offset), m_color(color) { ; }
    float get_offset(void) const { return m_offset; }
    RGBA8 get_color(void) const { return m_color; }
};

namespace meta {

        template <typename IT>
        using is_color_stop_iterator = std::integral_constant<bool,
            std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IT>::iterator_category>::value &&
            std::is_same<typename std::iterator_traits<IT>::value_type, color_stop>::value>;

} // namespace meta

class color_ramp: public boost::intrusive_ref_counter<class color_ramp> {
    e_spread m_spread;
    std::vector<color_stop> m_color_stops;
public:
    using ptr = boost::intrusive_ptr<class color_ramp>;
    using const_ptr = boost::intrusive_ptr<const class color_ramp>;

	// construct from initializer list
    color_ramp(e_spread spread,
        const std::initializer_list<color_stop> &color_stops):
        m_spread(spread),
        m_color_stops(color_stops) { ; };

	// construct from stop iterator
    template <typename SIT>
    color_ramp(e_spread spread, SIT &begin, const SIT &end,
        typename std::enable_if<
            rvg::meta::is_color_stop_iterator<SIT>::value>::type * = nullptr):
		m_spread(spread),
        m_color_stops(begin, end) { ; }

    e_spread get_spread(void) const {
        return m_spread;
    }

    const std::vector<color_stop> &get_color_stops(void) const {
        return m_color_stops;
    }
};

} // namespace rvg

#endif
