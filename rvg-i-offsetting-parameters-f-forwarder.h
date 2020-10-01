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
#ifndef RVG_I_OFFSETTING_PARAMETERS_F_FORWARDER_H
#define RVG_I_OFFSETTING_PARAMETERS_F_FORWARDER_H

#include "rvg-i-offsetting-parameters.h"

namespace rvg {

template <typename DERIVED>
class i_offsetting_parameters_f_forwarder: public i_offsetting_parameters<DERIVED> {

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    i_offsetting_parameters_f_forwarder(void) {
        static_assert(meta::is_an_i_sink<DERIVED>::value,
            "derived class is not an i_sink");
    }

private:

// i_offsetting_parameters interface
friend i_offsetting_parameters<DERIVED>;

    void do_offset_cusp_parameter(rvgf t) {
        return derived().sink().offset_cusp_parameter(t);
    }

    void do_evolute_cusp_parameter(rvgf t) {
        return derived().sink().evolute_cusp_parameter(t);
    }

};

} // namespace rvg

#endif
