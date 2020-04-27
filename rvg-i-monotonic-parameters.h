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
#ifndef RVG_I_MONOTONIC_PARAMETERS_H
#define RVG_I_MONOTONIC_PARAMETERS_H

#include "rvg-meta.h"
#include "rvg-floatint.h"

namespace rvg {

template <typename DERIVED>
class i_monotonic_parameters {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    void root_dx_parameter(rvgf t) {
        return derived().do_root_dx_parameter(t);
    }

    void root_dy_parameter(rvgf t) {
        return derived().do_root_dy_parameter(t);
    }

    void root_dw_parameter(rvgf t) {
        return derived().do_root_dw_parameter(t);
    }

};

}

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_monotonic_parameters = std::integral_constant<
    bool,
    is_template_base_of<
        rvg::i_monotonic_parameters,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

namespace detail {
	class not_an_i_monotonic_parameters { };
}

template <typename U, typename T>
using inherit_if_i_monotonic_parameters =
	typename std::conditional<
		is_an_i_monotonic_parameters<U>::value,
		T,
		detail::not_an_i_monotonic_parameters
	>::type;


} } // namespace rvg::meta

#endif
