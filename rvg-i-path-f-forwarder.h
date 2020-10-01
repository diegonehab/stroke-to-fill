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
#ifndef RVG_I_PATH_F_FORWARDER_H
#define RVG_I_PATH_F_FORWARDER_H

#include "rvg-i-input-path-f-forwarder.h"
#include "rvg-i-regular-path-f-forwarder.h"
#include "rvg-i-decorated-path-f-forwarder.h"
#include "rvg-i-monotonic-parameters-f-forwarder.h"
#include "rvg-i-cubic-parameters-f-forwarder.h"
#include "rvg-i-offsetting-parameters-f-forwarder.h"
#include "rvg-i-join-parameters-f-forwarder.h"
#include "rvg-i-dashing-parameters-f-forwarder.h"

namespace rvg {

template <typename DERIVED>
class i_path_f_forwarder:
    public i_input_path_f_forwarder<DERIVED>,
    public i_regular_path_f_forwarder<DERIVED>,
    public i_decorated_path_f_forwarder<DERIVED>,
    public i_monotonic_parameters_f_forwarder<DERIVED>,
    public i_offsetting_parameters_f_forwarder<DERIVED>,
    public i_join_parameters_f_forwarder<DERIVED>,
    public i_dashing_parameters_f_forwarder<DERIVED>,
    public i_cubic_parameters_f_forwarder<DERIVED> {

};

} // namespace rvg

#endif
