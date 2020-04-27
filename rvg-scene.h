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
#ifndef RVG_SCENE_H
#define RVG_SCENE_H

#include "rvg-i-xformable.h"
#include "rvg-scene-data.h"
#include "rvg-rgba.h"

namespace rvg {

// This is simply an input scene that can be xformed and has a background color
class scene: public i_xformable<scene> {

    scene_data::const_ptr m_scene_ptr;

    RGBA8 m_background_color;

public:

    explicit scene(scene_data::const_ptr scene_ptr,
        RGBA8 color = RGBA8{255,255,255,255}):
        m_scene_ptr(scene_ptr),
        m_background_color{color} { ; }

    const scene_data &get_scene_data(void) const {
        return *m_scene_ptr;
    }

    scene_data::const_ptr get_scene_data_ptr(void) const {
        return m_scene_ptr;
    }

    RGBA8 get_background_color(void) const {
        return m_background_color;
    }

    void set_background_color(RGBA8 color) {
        m_background_color = color;
    }

    scene without_background_color(void) const {
        return scene{m_scene_ptr};
    }

    scene over(RGBA8 color) const {
        return scene{m_scene_ptr,
            post_divide(
                rvg::over(
                    pre_multiply(color),
                    pre_multiply(m_background_color)))};
    }

};

}

#endif
