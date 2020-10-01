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
#ifndef RVG_INPUT_PATH_FORWARD_IF_H
#define RVG_INPUT_PATH_FORWARD_IF_H

#include "rvg-i-input-path.h"
#include "rvg-i-point-input-path.h"
#include "rvg-path-f-null.h"

namespace rvg {

template <typename SINK,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_input_path<SINK>::value
    >::type
>
SINK &
make_input_path_f_forward_if(SINK &sink) {
    return sink;
}

template <typename SINK,
    typename = typename std::enable_if<
        !rvg::meta::is_an_i_input_path<SINK>::value
    >::type
>
path_f_null &
make_input_path_f_forward_if(SINK &) {
    static path_f_null pfn{};
    return pfn;
}

} // namespace rvg

#endif
