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
#ifndef RVG_RGBA_H
#define RVG_RGBA_H

#include <cstdint>
#include <array>

#include "rvg-util.h"
#include "rvg-unorm.h"

namespace rvg {

template <typename U>
class RGBA final {

    std::array<U, 4> m_channels;

public:

    RGBA(void): m_channels{{U{0}, U{0}, U{0}, U{0}}} {
        ;
    }

    template <typename V>
    RGBA(V r, V g, V b, V a):
        m_channels{{
            static_cast<U>(r),
            static_cast<U>(g),
            static_cast<U>(b),
            static_cast<U>(a)
        }} {
        ;
    }

    template <typename V>
    RGBA(const RGBA<V> &o):
        m_channels{{
            static_cast<U>(o[0]),
            static_cast<U>(o[1]),
            static_cast<U>(o[2]),
            static_cast<U>(o[3])
        }} {
        ;
    }

    template <typename V>
    RGBA<U> &operator=(const RGBA<V> &o) {
        if (this != &o) {
            m_channels[0] = static_cast<U>(o[0]);
            m_channels[1] = static_cast<U>(o[1]);
            m_channels[2] = static_cast<U>(o[2]);
            m_channels[3] = static_cast<U>(o[3]);
        }
        return *this;
    }

    U operator[](int k) const {
        return m_channels[k];
    }

    U &operator[](int k) {
        return m_channels[k];
    }

    template <typename V>
    RGBA<U> operator*(const V &v) const {
        return RGBA<U>{
            m_channels[0]*v,
            m_channels[1]*v,
            m_channels[2]*v,
            m_channels[3]*v
        };
    }

    template <typename V>
    RGBA<U> operator*(const RGBA<V> &v) const {
        return RGBA<U>{
            m_channels[0]*v[0],
            m_channels[1]*v[1],
            m_channels[2]*v[2],
            m_channels[3]*v[3]
        };
    }

    template <typename V>
    RGBA<U> operator/(const V &v) const {
        return RGBA<U>{
            m_channels[0]/v,
            m_channels[1]/v,
            m_channels[2]/v,
            m_channels[3]/v
        };
    }

    template <typename V>
    RGBA<U> operator/(const RGBA<V> &v) const {
        return RGBA<U>{
            m_channels[0]/v[0],
            m_channels[1]/v[1],
            m_channels[2]/v[2],
            m_channels[3]/v[3]
        };
    }

    template <typename V>
    RGBA<U> operator+(const RGBA<V> &v) const {
        return RGBA<U>{
            m_channels[0]+v[0],
            m_channels[1]+v[1],
            m_channels[2]+v[2],
            m_channels[3]+v[3]
        };
    }

    template <typename V>
    RGBA<U> operator-(const RGBA<V> &v) const {
        return RGBA<U>{
            m_channels[0]-v[0],
            m_channels[1]-v[1],
            m_channels[2]-v[2],
            m_channels[3]-v[3]
        };
    }

    template <typename V>
    RGBA<U> &operator+=(const RGBA<V> &v) {
        m_channels[0] += v[0];
        m_channels[1] += v[1];
        m_channels[2] += v[2];
        m_channels[3] += v[3];
        return *this;
    }

    template <typename V>
    RGBA<U> &operator-=(const RGBA<V> &v) {
        m_channels[0] -= v[0];
        m_channels[1] -= v[1];
        m_channels[2] -= v[2];
        m_channels[3] -= v[3];
        return *this;
    }

    template <typename V>
    RGBA<U> &operator*=(const V &v) {
        m_channels[0] *= v;
        m_channels[1] *= v;
        m_channels[2] *= v;
        m_channels[3] *= v;
        return *this;
    }

    template <typename V>
    RGBA<U> &operator*=(const RGBA<V> &v) {
        m_channels[0] *= v[0];
        m_channels[1] *= v[1];
        m_channels[2] *= v[2];
        m_channels[3] *= v[3];
        return *this;
    }

    template <typename V>
    RGBA<U> &operator/=(const V &v) {
        m_channels[0] /= v;
        m_channels[1] /= v;
        m_channels[2] /= v;
        m_channels[3] /= v;
        return *this;
    }

    template <typename V>
    RGBA<U> &operator/=(const RGBA<V> &v) {
        m_channels[0] /= v[0];
        m_channels[1] /= v[1];
        m_channels[2] /= v[2];
        m_channels[3] /= v[3];
        return *this;
    }

    template <typename V>
    bool operator==(const RGBA<V> &o) const {
        return m_channels[0] == o[0] &&
            m_channels[1] == o[1] &&
            m_channels[2] == o[2] &&
            m_channels[3] == o[3];
    }

    template <typename V>
    bool operator!=(const RGBA<V> &o) const {
        return !(*this == o);
    }

};

template <typename U>
RGBA<unorm<U>> pre_multiply(const RGBA<unorm<U>> &c) {
    return RGBA<unorm<U>>{ c[0]*c[3], c[1]*c[3], c[2]*c[3], c[3] };
}

template <typename U>
RGBA<unorm<U>> post_divide(const RGBA<unorm<U>> &c) {
    return RGBA<unorm<U>>{ c[0]/c[3], c[1]/c[3], c[2]/c[3], c[3] };
}

template <typename U>
RGBA<unorm<U>> over(const RGBA<unorm<U>> &c, const RGBA<unorm<U>> &b) {
    auto t = ~c[3];
    return RGBA<unorm<U>>{
        b[0]*t + c[0],
        b[1]*t + c[1],
        b[2]*t + c[2],
        b[3]*t + c[3]
    };
}

template <typename U>
RGBA<unorm<U>> add_gamma(const RGBA<unorm<U>> &c) {
    return RGBA<unorm<U>>{
        add_gamma(c[0]),
        add_gamma(c[1]),
        add_gamma(c[2]),
        c[3]
    };
}

template <typename U>
RGBA<unorm<U>> remove_gamma(const RGBA<unorm<U>> &c) {
    return RGBA<unorm<U>>{
        remove_gamma(c[0]),
        remove_gamma(c[1]),
        remove_gamma(c[2]),
        c[3]
    };
}

using RGBA8 = RGBA<unorm8>;

static inline RGBA8 make_rgba(float r, float g, float b, float a) {
    return RGBA8{
        unorm8::clamped(r),
        unorm8::clamped(g),
        unorm8::clamped(b),
        unorm8::clamped(a)
    };
}

static inline RGBA8 make_rgba8(int r, int g, int b, int a) {
    return RGBA8{
        unorm8::clamped(r),
        unorm8::clamped(g),
        unorm8::clamped(b),
        unorm8::clamped(a)
    };
}

static inline RGBA8 make_rgb(float r, float g, float b) {
    return RGBA8{
        unorm8::clamped(r),
        unorm8::clamped(g),
        unorm8::clamped(b),
        ~unorm8{0}
    };
}

static inline RGBA8 make_rgb8(int r, int g, int b) {
    return RGBA8{
        unorm8::clamped(r),
        unorm8::clamped(g),
        unorm8::clamped(b),
        ~unorm8{0}
    };
}

} // namespace rvg

#include <functional>

namespace std {

// so colors can be inserted into unordered maps
template <> struct hash<rvg::RGBA8> {
    size_t operator()(const rvg::RGBA8 &c) const {
        return hash<int32_t>()(
            static_cast<uint8_t>(c[0]) +
            (static_cast<uint8_t>(c[1]) << 8) +
            (static_cast<uint8_t>(c[2]) << 16) +
            (static_cast<uint8_t>(c[3]) << 24)
        );
    }
};

}

#endif

