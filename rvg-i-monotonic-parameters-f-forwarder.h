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
#ifndef RVG_I_MONOTONIC_PARAMETERS_F_FORWARDER_H
#define RVG_I_MONOTONIC_PARAMETERS_F_FORWARDER_H

#include "rvg-i-monotonic-parameters.h"

namespace rvg {

template <typename DERIVED>
class i_monotonic_parameters_f_forwarder: public i_monotonic_parameters<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_monotonic_parameters_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_monotonic_parameters interface
friend i_monotonic_parameters<DERIVED>;

    void do_root_dx_parameter(rvgf t) {
        return derived().sink().root_dx_parameter(t);
    }

    void do_root_dy_parameter(rvgf t) {
        return derived().sink().root_dy_parameter(t);
    }

    void do_root_dw_parameter(rvgf t) {
        return derived().sink().root_dw_parameter(t);
    }

};

} // namespace rvg

#endif
