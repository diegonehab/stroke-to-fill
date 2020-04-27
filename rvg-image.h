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
#ifndef RVG_IMAGE_H
#define RVG_IMAGE_H

#include <vector>
#include <array>
#include <cassert>
#include <string>
#include <type_traits>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "rvg-unorm.h"

#include "rvg-i-image.h"

namespace rvg {


template <typename T, size_t N>
class image final: public i_image {

    int m_width, m_height, m_pitch;
    e_color_space m_color_space;
    std::vector<T> m_data;

    template <typename C>
    void get_pixel_helper(int) const { }

    template <typename C, typename U, typename ...Args>
    void get_pixel_helper(int i, U &first, Args&... others) const {
        C convert;
        first = convert(m_data[i]);
        get_pixel_helper<C>(i+m_pitch*m_height, others...);
    }

    template <typename C>
    void set_pixel_helper(int) { ; }

    template <typename C, typename U, typename ...Args>
    void set_pixel_helper(int i, const U first, Args... others) {
        C convert;
        m_data[i] = convert(first);
        set_pixel_helper<C>(i+m_pitch*m_height, others...);
    }

public:

    using ptr = boost::intrusive_ptr<image<T,N>>;
    using const_ptr = boost::intrusive_ptr<const image<T,N>>;

    virtual ~image<T,N>() { ; }

    image(void):
        m_width(0),
        m_height(0),
        m_pitch(0),
        m_color_space(e_color_space::sRGB) { ; }

    const std::vector<T> &get_data(void) const { return m_data; }
    std::vector<T> &get_data(void) { return m_data; }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void get_pixel(int x, int y, Args&... channels) const {
        get_pixel_helper<C>(x+m_pitch*y, channels...);
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void get_pixel(int x, int y, Args&... channels) const {
        get_pixel_helper<unorm_converter<T,T>>(x+m_pitch*y, channels...);
    }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void set_pixel(int x, int y, Args... channels) {
        set_pixel_helper<C>(x+m_pitch*y, channels...);
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void set_pixel(int x, int y, Args... channels) {
        set_pixel_helper<unorm_converter<T,T>>(x+m_pitch*y, channels...);
    }

    template <typename C = unorm_converter<T,T>, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void load_from(int pitch, int width_in, int height_in, int pitch_in,
        int advance_in, const Args*... channels) {
        resize(width_in, height_in, pitch);
        int base_in = 0, base = 0;
        for (int i = 0; i < height_in; i++) {
            int offset_in = 0, offset = 0;
            for (int j = 0; j < width_in; j++) {
                set_pixel_helper<C>(base+offset,
                    channels[base_in+offset_in]...);
                offset_in += advance_in;
                offset += 1;
            }
            base_in += pitch_in;
            base += pitch;
        }
    }

    template <typename C = unorm_converter<T,T>, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void store_into(int width_out, int height_out, int pitch_out,
        int advance_out, Args*... channels) const {
        assert(width_out == m_width && height_out == m_height);
        // ??D exception here?
        if (width_out != m_width || height_out != m_height) return;
        int base = 0, base_out = 0;
        for (int i = 0; i < m_height; i++) {
            int offset = 0, offset_out = 0;
            for (int j = 0; j < m_width; j++) {
                get_pixel_helper<C>(base+offset,
                    channels[base_out+offset_out]...);
                offset += 1;
                offset_out += advance_out;
            }
            base += m_pitch;
            base_out += pitch_out;
        }
    }

protected:

    int do_get_width(void) const {
        return m_width;
    }

    int do_get_height(void) const {
        return m_height;
    }

    int do_get_pitch(void) const {
        return m_pitch;
    }

    int do_get_num_channels(void) const {
        return static_cast<int>(N);
    }

    e_organization do_get_organization(void) const {
        return e_organization::planar;
    }

    e_channel_type do_get_channel_type(void) const {
        return rvg::channel_type<T>();
    }

    e_color_space do_get_color_space(void) const {
        return m_color_space;
    }

    void do_set_color_space(e_color_space s) {
        m_color_space = s;
    }

    void do_resize(int width, int height, int pitch) {
        size_t old_size = m_data.size();
        size_t size = size_t(pitch*height*N);
        if (size > old_size || 2*size < old_size) {
            m_data.resize(size);
        }
        m_width = width;
        m_height = height;
        m_pitch = pitch;
    };

    void do_set_unorm(int x, int y, int c, float v) {
        m_data[x+m_pitch*(y+m_height*c)] = unorm<T>{unorm<float>{v}};
    }

    float do_get_unorm(int x, int y, int c) const {
        return unorm<float>{unorm<T>{m_data[x+m_pitch*(y+m_height*c)]}};
    }
};

} // namespace rvg

#endif // RVG_IMAGE_H
