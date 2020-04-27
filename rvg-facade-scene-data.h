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
#ifndef RVG_FACADE_SCENE_DATA_H
#define RVG_FACADE_SCENE_DATA_H

#include <initializer_list>
#include <vector>

#include "rvg-scene-data.h"
#include "rvg-facade-painted-compound.h"

namespace rvg_facade {

rvg::scene_data make_scene_data(const painted_compound &p);
rvg::scene_data make_scene_data(const std::initializer_list<painted_compound> &p);
rvg::scene_data make_scene_data(const std::vector<painted_compound> &p);

} // namespace rvg::facade

#endif
