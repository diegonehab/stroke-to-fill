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
#ifndef RVG_I_IMAGE_H
#define RVG_I_IMAGE_H

#include "rvg-ptr.h"
#include <string>
#include <utility>
#include <vector>

namespace rvg {

enum class e_color_space {
    sRGB,
    linear,
    unknown
};

enum class e_organization {
    packed,
    planar
};

enum class e_channel_type {
    float_,
    uint8_t_,
    uint16_t_,
    unknown
};

using image_attribute = std::pair<std::string, std::string>;
using image_attributes = std::vector<image_attribute>;

class i_image: public boost::intrusive_ref_counter<i_image> {
public:
    using ptr = boost::intrusive_ptr<i_image>;
    using const_ptr = boost::intrusive_ptr<const i_image>;

    virtual ~i_image() { ; }

    int get_width(void) const {
        return do_get_width();
    }

    int get_height(void) const {
        return do_get_height();
    }

    int get_num_channels(void) const {
        return do_get_num_channels();
    }

    int get_pitch(void) const {
        return do_get_pitch();
    }

    void resize(int width, int height, int pitch = 0) {
        if (pitch == 0) pitch = width;
        return do_resize(width, height, pitch);
    }

    void set_unorm(int x, int y, int c, float v) {
        return do_set_unorm(x, y, c, v);
    }

    float get_unorm(int x, int y, int c) const {
        return do_get_unorm(x, y, c);
    }

    e_channel_type get_channel_type(void) const {
        return do_get_channel_type();
    }

    e_organization get_organization(void) const {
        return do_get_organization();
    }

    void set_color_space(e_color_space color_space) {
        return do_set_color_space(color_space);
    }

    e_color_space get_color_space(void) const {
        return do_get_color_space();
    }

protected:
    virtual void do_resize(int width, int height, int pitch) = 0;
    virtual void do_set_unorm(int x, int y, int c, float v) = 0;
    virtual float do_get_unorm(int x, int y, int c) const = 0;
    virtual int do_get_width(void) const = 0;
    virtual int do_get_height(void) const = 0;
    virtual int do_get_num_channels(void) const = 0;
    virtual int do_get_pitch(void) const = 0;
    virtual e_channel_type do_get_channel_type(void) const = 0;
    virtual e_organization do_get_organization(void) const = 0;
    virtual void do_set_color_space(e_color_space color_space) = 0;
    virtual e_color_space do_get_color_space(void) const = 0;

};

template <typename T>
inline
e_channel_type channel_type(void) {
    return e_channel_type::unknown;
}

template <>
inline
e_channel_type channel_type<float>(void) {
    return e_channel_type::float_;
}

template <>
inline
e_channel_type channel_type<uint8_t>(void) {
    return e_channel_type::uint8_t_;
}

template <>
inline
e_channel_type channel_type<uint16_t>(void) {
    return e_channel_type::uint16_t_;
}

} // namespace rvg

#endif

