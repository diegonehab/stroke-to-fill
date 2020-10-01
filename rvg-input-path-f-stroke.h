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
#ifndef RVG_INPUT_PATH_F_STROKE_H
#define RVG_INPUT_PATH_F_STROKE_H

#include "rvg-stroke-style.h"
#include "rvg-path-f-find-offsetting-parameters.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-input-path-f-find-cubic-parameters.h"
#include "rvg-input-path-f-to-regular-path.h"
#include "rvg-regular-path-f-to-decorated-path.h"
#include "rvg-regular-path-f-orient.h"
#include "rvg-decorated-path-f-simplify-joins.h"
#include "rvg-decorated-path-f-forward-and-backward.h"
#include "rvg-decorated-path-f-thicken.h"
#include "rvg-input-path-f-simplify.h"

#define RVG_STROKE_APPROXIMATION_TOLERANCE (2.e-1f)

namespace rvg {

template <typename SINK>
static auto
make_input_path_f_stroke(
    rvgf width,
    stroke_style::const_ptr style,
    rvgf ptol,
    rvgf alpha,
    rvgf delta,
    SINK &&sink) {
  return make_input_path_f_close_contours(
#ifndef RVG_THICKEN_WITH_CUBICS
      make_input_path_f_find_cubic_parameters(
#endif
        make_path_f_find_offsetting_parameters(ptol,
          make_input_path_f_to_regular_path(ptol, alpha, delta,
            make_regular_path_f_orient(
              make_path_f_find_offsetting_parameters(width/2,
                make_regular_path_f_to_decorated_path(width, style,
                  make_decorated_path_f_simplify_joins(width/2,
                    make_decorated_path_f_forward_and_backward(
                      make_decorated_path_f_thicken(width, style, ptol,
                        make_input_path_f_simplify(ptol, sink)))))))))
#ifndef RVG_THICKEN_WITH_CUBICS
        )
#endif
      );
}

template <typename SINK>
static auto
make_input_path_f_stroke(
    rvgf width,
    stroke_style::const_ptr style,
    SINK &&sink) {
  return make_input_path_f_stroke(width, style,
      RVG_STROKE_APPROXIMATION_TOLERANCE,
      RVG_REGULARITY_ANGULAR_TOLERANCE,
      RVG_REGULARITY_NUMERICAL_TOLERANCE*
          std::numeric_limits<rvgf>::epsilon(),
    std::forward<SINK>(sink));
}

} // namespace rvg

#endif
