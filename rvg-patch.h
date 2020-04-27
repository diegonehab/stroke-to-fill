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
#ifndef RVG_PATCH_H
#define RVG_PATCH_H

#include <algorithm>

#include "rvg-ptr.h"
#include "rvg-patch-data.h"
#include "rvg-unorm.h"
#include "rvg-i-xformable.h"

namespace rvg {

template <size_t P, size_t C>
class patch final:
    public i_xformable<patch<P, C>>,
    public boost::intrusive_ref_counter<patch<P, C>> {

public:
    using patch_data_type = patch_data<P, C>;

private:

    typename patch_data_type::const_ptr m_data;
    unorm8 m_opacity;

public:

    using ptr = boost::intrusive_ptr<patch<P, C>>;
    using const_ptr = boost::intrusive_ptr<const patch<P, C>>;

    patch(const R2 (&points)[P], const RGBA8 (&colors)[C], unorm8 opacity):
        m_data{make_intrusive<patch_data_type>(points, colors)},
        m_opacity{opacity} {
        ;
    }

    patch(typename patch_data_type::const_ptr data, unorm8 opacity):
        m_data{data},
        m_opacity{opacity} {
        ;
    }

    typename patch_data_type::const_ptr get_patch_data_ptr(void) const {
        return m_data;
    }

    const patch_data_type &get_patch_data(void) const {
        return *m_data;
    }

    unorm8 get_opacity(void) const {
        return m_opacity;
    }

};

} // namespace rvg

#endif
