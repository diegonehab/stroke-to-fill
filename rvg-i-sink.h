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
#ifndef RVG_I_SINK_H
#define RVG_I_SINK_H

#include "rvg-meta.h"

namespace rvg {

template <typename DERIVED>
class i_sink {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    auto &sink(void) {
        return derived().do_sink();
    }

    const auto &sink(void) const {
        return derived().do_sink();
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_sink = std::integral_constant<
    bool,
        is_template_base_of<
        rvg::i_sink,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
