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
#ifndef RVG_I_POINT_PATH_H
#define RVG_I_POINT_PATH_H

#include "rvg-meta.h"
#include "rvg-i-point-input-path.h"
#include "rvg-i-point-regular-path.h"
#include "rvg-i-point-decorated-path.h"

namespace rvg {

template <typename DERIVED>
class i_point_path:
    public i_point_input_path<DERIVED>,
    public i_point_regular_path<DERIVED>,
    public i_point_decorated_path<DERIVED> {

};


} // namespace rvg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_point_path = std::integral_constant<
    bool,
    is_an_i_point_input_path<DERIVED>::value &&
    is_an_i_point_regular_path<DERIVED>::value &&
    is_an_i_point_decorated_path<DERIVED>::value>;

} } // namespace rvg::meta

#endif
