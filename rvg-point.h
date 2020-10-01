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
#ifndef RVG_POINT_H
#define RVG_POINT_H

#include <iostream>
#include <tuple>
#include <array>
#include <cmath>

#include "rvg-util.h"
#include "rvg-i-point.h"

namespace rvg {

using RP1_array = std::array<rvgf, 2>;
using RP1_tuple = std::tuple<rvgf, rvgf>;
using R2_array = std::array<rvgf, 2>;
using RP2_array = std::array<rvgf, 3>;
using R3_array = RP2_array;
using R2_tuple = std::tuple<rvgf, rvgf>;
using RP2_tuple = std::tuple<rvgf, rvgf, rvgf>;
using R3_tuple = RP2_tuple;

class RP1 final: public i_point<RP1> {

    RP1_array m_v;

public:

    RP1(void): m_v{{0, 1}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(void)\n";
#endif
    }

    explicit RP1(rvgf f): m_v{{f, 1}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(rvgf)\n";
#endif
    }

    RP1(rvgf x, rvgf w): m_v{{x, w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(void)\n";
#endif
    }

    // conversion from array and tuple
    explicit RP1(const RP1_array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1(const RP1_array &)\n";
#endif
    }

    explicit RP1(const RP1_tuple &v): m_v{{std::get<0>(v), std::get<1>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1(const RP1_tuple &)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator RP1_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1::operator RP1_array()\n";
#endif
        return m_v;
    }

    explicit operator RP1_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1::operator RP1_tuple()\n";
#endif
        return RP1_tuple{m_v[0], m_v[1]};
    }

    rvgf get_x(void) const { return m_v[0]; }
    rvgf get_w(void) const { return m_v[1]; }

private:

    friend i_point<RP1>;

    const rvgf &do_component(int i) const { return m_v[i]; }

    rvgf &do_component(int i) { return m_v[i]; }

    // note the difference between Euclidean and projective sum
    RP1 do_added(const RP1 &o) const {
        if (m_v[1] == o.m_v[1]) {
            return RP1{ m_v[0]+o.m_v[0], m_v[1] };
        } else {
            return RP1{ m_v[0]*o.m_v[1]+o.m_v[0]*m_v[1], m_v[1]*o.m_v[1] };
        }
    }

    RP1 do_multiplied(const RP1 &o) const {
        return RP1{ m_v[0]*o.m_v[0], m_v[1]*o.m_v[1] };
    }

    // note the difference between Euclidean and projective subtraction
    RP1 do_subtracted(const RP1 &o) const {
        if (m_v[1] == o.m_v[1]) {
            return RP1{ m_v[0]-o.m_v[0], m_v[1] };
        } else {
            return RP1{ m_v[0]*o.m_v[1]-o.m_v[0]*m_v[1], m_v[1]*o.m_v[1] };
        }
    }

    // note the difference between Euclidean and projective
    // scalar multiplication
    RP1 do_multiplied(rvgf s) const {
        return RP1{ s*m_v[0], m_v[1] };
    }

    // note the difference between Euclidean and projective
    // scalar division
    RP1 do_divided(rvgf s) const {
        return RP1{ m_v[0]/s, m_v[1] };
    }

    // note the difference between Euclidean and projective unary minus
    RP1 do_negated(void) const {
        return RP1{ -m_v[0], m_v[1] };
    }

    bool do_is_zero(void) const {
        return m_v[0] == 0 && m_v[1] != 0;
    }

    bool do_is_almost_ideal(int ulp) const {
        const rvgf x = m_v[0];
        const rvgf w =  m_v[1];
        return util::is_relatively_zero(w, x, ulp) || util::is_almost_zero(w);
    }

    bool do_is_equal(const RP1 &o) const {
        if (m_v[1] != 0 || o.m_v[1] != 0) {
            return m_v[0]*o.m_v[1] == o.m_v[0]*m_v[1];
        } else {
            return true;
        }
    }

    bool do_is_almost_equal(const RP1 &o, int ulp) const {
        if (!is_almost_ideal(ulp) || !o.is_almost_ideal(ulp)) {
            return util::is_almost_equal(m_v[0]*o.m_v[1], o.m_v[0]*m_v[1], ulp);
        } else {
            return true;
        }
    }

    RP1_tuple do_untie(void) const {
        return RP1_tuple{m_v[0], m_v[1]};
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "rp1{" << m_v[0] << ',' << m_v[1] << '}';
        return out;
    }

};

static inline RP1 make_RP1(rvgf x, rvgf w = 1.0) {
    return RP1(x, w);
}

// Euclidean point in R2
class R2 final: public i_point<R2> {

    R2_array m_v;

public:

    R2(void): m_v{{0, 0}} {
#ifdef POINT_DEBUG
        std::cerr << "R2(void)\n";
#endif
    }

    R2(rvgf x, rvgf y): m_v{{x, y}} {
#ifdef POINT_DEBUG
        std::cerr << "R2(rvgf, rvgf)\n";
#endif
    }

    // conversion from array and tuple
    explicit R2(const R2_array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2(const R2_array &)\n";
#endif
    }

    explicit R2(const R2_tuple &v): m_v{{std::get<0>(v), std::get<1>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2(const R2_tuple &)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator R2_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R2_array()\n";
#endif
        return m_v;
    }

    explicit operator R2_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R2_tuple()\n";
#endif
        return R2_tuple{m_v[0], m_v[1]};
    }

    explicit operator RP2_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator RP2_array()\n";
#endif
        return RP2_array{{m_v[0], m_v[1], 1}};
    }

    explicit operator RP2_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator RP2_tuple()\n";
#endif
		return RP2_tuple{ m_v[0], m_v[1], rvgf(1) };
    }

#if 0
    // ??D same as RP2_array?
	explicit operator R3_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R3_array()\n";
#endif
        return R3_array{{m_v[0], m_v[1], 1}};
    }
#endif

    rvgf get_x(void) const { return m_v[0]; }
    rvgf get_y(void) const { return m_v[1]; }

private:
    friend i_point<R2>;

    const rvgf &do_component(int i) const { return m_v[i]; }

    rvgf &do_component(int i) { return m_v[i]; }

    R2 do_added(const R2 &o) const {
        return R2{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1] };
    }

    R2 do_multiplied(const R2 &o) const {
        return R2{ m_v[0]*o.m_v[0], m_v[1]*o.m_v[1] };
    }

    R2 do_subtracted(const R2 &o) const {
        return R2{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1] };
    }

    R2 do_negated() const {
        return R2{ -m_v[0], -m_v[1] };
    }

    R2 do_multiplied(rvgf s) const {
        return R2{ s*m_v[0], s*m_v[1] };
    }

    R2 do_divided(rvgf s) const {
        return R2{ m_v[0]/s, m_v[1]/s };
    }

    R2_tuple do_untie(void) const {
        return R2_tuple{m_v[0], m_v[1]};
    }

    bool do_is_zero(void) const {
        return m_v[0] == 0 && m_v[1] == 0;
    }

    bool do_is_equal(const R2 &o) const {
        return m_v[0] == o.m_v[0] && m_v[1] == o.m_v[1];
    }

    bool do_is_almost_ideal(rvgf, rvgf) const {
        return false;
    }

    bool do_is_almost_equal(const R2 &o, int ulp) const {
        rvgf dx = std::fabs(m_v[0]-o.m_v[0]);
        rvgf mx = util::fmaxabs(m_v[0], o.m_v[0]);
        rvgf dy = std::fabs(m_v[1]-o.m_v[1]);
        rvgf my = util::fmaxabs(m_v[1], o.m_v[1]);
        using util::is_almost_zero;
        using util::is_relatively_zero;
        if (dx < dy) {
            std::swap(dx, dy);
            std::swap(mx, my);
        }
        // Two points are certainly almost equal if the
        // coordinates are almost equal independently
        // They are not almost equal if neither coordinate
        // is almost equal independently.
        // But if one of the coordinates is not equal, the
        // points may still be almost equal.
        // This is because the coordinates may have very
        // different scales.
        // The coordinate with smallest magnitude may be
        // different at their scale, but almost equal at the
        // scale of the coordinate with largest magnitude
        // Both coordinates are almost equal in an absolute scale
        if (is_almost_zero(dx)) return true;
        // The coordinate with largest magnitude is
        // almost equal, and the one in smallest
        // magnitude is either almost equal as well or
        // is almost equal in the scale of the largest magnitude
        if (is_relatively_zero(dx, mx, ulp) &&
            (is_almost_zero(dy) ||
             is_relatively_zero(dy, my, ulp) ||
             is_relatively_zero(dy, mx, ulp)) ) return true;
        return false;
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "r2{" << m_v[0] << ',' << m_v[1] << '}';
        return out;
    }
};

static inline R2 make_R2(rvgf x, rvgf y) {
    return R2(x, y);
}

// Projective point in RP2
class RP2 final: public i_point<RP2> {

    RP2_array m_v;

public:

    RP2(void): m_v{{0, 0, 1}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(void)\n";
#endif
    }

    RP2(rvgf x, rvgf y, rvgf w): m_v{{x, y, w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(rvgf, rvgf, rvgf)\n";
#endif
    }

    // convertion from array and tuple
    explicit RP2(const RP2_array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const RP2_array &)\n";
#endif
    }

    explicit RP2(const RP2_tuple &v): m_v{{std::get<0>(v), std::get<1>(v), std::get<2>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const RP2_tuple &)\n";
#endif
    }

    explicit RP2(const R2_array &v): m_v{{v[0], v[1], 1}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const R2_array &)\n";
#endif
    }

    explicit RP2(const R2_tuple &v): m_v{{std::get<0>(v), std::get<1>(v), 1}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const R2_tuple &)\n";
#endif
    }

    // conversion from Euclidean point
    explicit RP2(const R2 &p, rvgf w = 1): m_v{{p[0], p[1], w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(R2 &, rvgf = 1)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator RP2_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator RP2_array()\n";
#endif
        return m_v;
    }

    explicit operator RP2_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "RP2::operator RP2_tuple()\n";
#endif
        return RP2_tuple{m_v[0], m_v[1], m_v[2]};
    }

    // conversion to array and tuple of other type
    explicit operator R2_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2_array()\n";
#endif
        return R2_array{{m_v[0]/m_v[2], m_v[1]/m_v[2]}};
    }

    // explicit conversion to array and tuple
    explicit operator R2_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2_tuple()\n";
#endif
        return R2_tuple{m_v[0]/m_v[2], m_v[1]/m_v[2]};
    }

    // explicit conversion to Euclidean point
    explicit operator R2() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2()\n";
#endif
        return R2{m_v[0]/m_v[2], m_v[1]/m_v[2]};
    }

    // move the exponent and sign from w to x,y
    RP2 adjusted(void) const {
        int e;
        rvgf f = std::fabs(std::frexp(m_v[2], &e));
        e = -e;
        if (m_v[2] > 0) {
            return RP2{std::ldexp(m_v[0], e), std::ldexp(m_v[1], e), f};
        } else {
            return RP2{-std::ldexp(m_v[0], e), -std::ldexp(m_v[1], e), f};
        }
    }

    rvgf get_x(void) const { return m_v[0]; }
    rvgf get_y(void) const { return m_v[1]; }
    rvgf get_w(void) const { return m_v[2]; }

private:
    friend i_point<RP2>;

    const rvgf &do_component(int i) const { return m_v[i]; }

    rvgf &do_component(int i) { return m_v[i]; }

    // note the difference between Euclidean and projective sum
    RP2 do_added(const RP2 &o) const {
        if (m_v[2] == o.m_v[2]) {
            return RP2{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1], m_v[2] };
        } else {
            return RP2{ m_v[0]*o.m_v[2]+o.m_v[0]*m_v[2],
               m_v[1]*o.m_v[2]+o.m_v[1]*m_v[2], m_v[2]*o.m_v[2]
            };
        }
    }

    RP2 do_multiplied(const RP2 &o) const {
        return RP2{ m_v[0]*o.m_v[0], m_v[1]*o.m_v[1], m_v[2]*o.m_v[2] };
    }

    // note the difference between Euclidean and projective subtraction
    RP2 do_subtracted(const RP2 &o) const {
        if (m_v[2] == o.m_v[2]) {
            return RP2{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1], m_v[2] };
        } else {
            return RP2{ m_v[0]*o.m_v[2]-o.m_v[0]*m_v[2],
               m_v[1]*o.m_v[2]-o.m_v[1]*m_v[2], m_v[2]*o.m_v[2]
            };
        }
    }

    // note the difference between Euclidean and projective
    // scalar multiplication
    RP2 do_multiplied(rvgf s) const {
        return RP2{ s*m_v[0], s*m_v[1], m_v[2] };
    }

    // note the difference between Euclidean and projective
    // scalar division
    RP2 do_divided(rvgf s) const {
        return RP2{ m_v[0]/s, m_v[1]/s, m_v[2] };
    }

    // note the difference between Euclidean and projective unary minus
    RP2 do_negated(void) const {
        return RP2{ -m_v[0], -m_v[1], m_v[2] };
    }

    bool do_is_almost_ideal(int ulp) const {
        const rvgf v = util::fminabs(m_v[0], m_v[1]);
        const rvgf w = m_v[2];
        return util::is_relatively_zero(w, v, ulp) || util::is_almost_zero(w);
    }

    bool do_is_zero(void) const {
        return m_v[0] == 0 && m_v[1] == 0 && m_v[2] != 0;
    }

    bool do_is_equal(const RP2 &o) const {
        if (m_v[2] != 0 || o.m_v[2] != 0) {
            return m_v[0]*o.m_v[2] == o.m_v[0]*m_v[2] &&
                   m_v[1]*o.m_v[2] == o.m_v[1]*m_v[2];
        } else {
            return m_v[0]*o.m_v[1] == o.m_v[0]*m_v[1];
        }
    }

    bool do_is_almost_equal(const RP2 &o, int ulp) const {
        if (!is_almost_ideal(ulp) || !o.is_almost_ideal(ulp)) {
            R2 us{m_v[0]*o.m_v[2], m_v[1]*o.m_v[2]};
            R2 them{o.m_v[0]*m_v[2], o.m_v[1]*m_v[2]};
            return us.is_almost_equal(them, ulp);
        } else {
            return rvg::util::is_almost_equal(m_v[0]*o.m_v[1],
                o.m_v[0]*m_v[1], ulp);
        }
    }

    RP2_tuple do_untie(void) const {
        return RP2_tuple{m_v[0], m_v[1], m_v[2]};
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "rp2{" << m_v[0] << ',' << m_v[1] << ',' << m_v[2] << '}';
        return out;
    }
};

static inline RP2 make_RP2(rvgf x, rvgf y, rvgf w = 1.0f) {
    return RP2(x, y, w);
}

// Euclidean point in R3
class R3 final: public i_point<R3> {

    R3_array m_v;

public:

    R3(void): m_v{{0, 0, 0}} {
#ifdef POINT_DEBUG
        std::cerr << "R3(void)\n";
#endif
    }

    R3(rvgf x, rvgf y, rvgf z): m_v{{x, y, z}} {
#ifdef POINT_DEBUG
        std::cerr << "R3(rvgf, rvgf, rvgf)\n";
#endif
    }

    // explicit conversion from RP2
    explicit R3(const RP2 &rp2): m_v{{rp2[0], rp2[1], rp2[2]}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const RP2 &)\n";
#endif
    }

    // explicit conversion from R2
    explicit R3(const R2 &r2, rvgf w = 1): m_v{{r2[0], r2[1], w}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const RP2 &)\n";
#endif
    }

    // conversion from array and tuple
    explicit R3(const R3_array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const R3_array &)\n";
#endif
    }

    explicit R3(const R3_tuple &v): m_v{{std::get<0>(v), std::get<1>(v), std::get<2>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const R3_tuple &)\n";
#endif
    }

    // automatic conversion to array and tuple
    explicit operator R3_array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator R3_array()\n";
#endif
        return m_v;
    }

    explicit operator R3_tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator R3_tuple()\n";
#endif
        return R3_tuple{m_v[0], m_v[1], m_v[2]};
    }

    // explicit conversion to RP2
    explicit operator RP2() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator RP2()\n";
#endif
        return RP2{m_v[0], m_v[1], m_v[2]};
    }

    rvgf get_x(void) const { return m_v[0]; }
    rvgf get_y(void) const { return m_v[1]; }
    rvgf get_z(void) const { return m_v[2]; }

private:
    friend i_point<R3>;

    R3 do_added(const R3 &o) const {
        return R3{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1], m_v[2]+o.m_v[2] };
    }

    R3 do_multiplied(const R3 &o) const {
        return R3{ m_v[0]*o.m_v[0], m_v[1]*o.m_v[1], m_v[2]*o.m_v[2] };
    }

    R3 do_subtracted(const R3 &o) const {
        return R3{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1], m_v[2]-o.m_v[2]};
    }

    R3 do_negated() const {
        return R3{ -m_v[0], -m_v[1], -m_v[2] };
    }

    R3 do_multiplied(rvgf s) const {
        return R3{ s*m_v[0], s*m_v[1], s*m_v[2] };
    }

    R3 do_divided(rvgf s) const {
        return R3{ m_v[0]/s, m_v[1]/s, m_v[2]/s };
    }

    const rvgf &do_component(int i) const { return m_v[i]; }

    rvgf &do_component(int i) { return m_v[i]; }

    R3_tuple do_untie(void) const {
        return R3_tuple{m_v[0], m_v[1], m_v[2]};
    }

    bool do_is_zero(void) const {
        return m_v[0] == 0 && m_v[1] == 0 && m_v[2] == 0;
    }

    bool do_is_equal(const R3 &o) const {
        return m_v[0] == o.m_v[0] && m_v[1] == o.m_v[1] && m_v[2] == o.m_v[2];
    }

    bool do_is_almost_equal(const R3 &o, int ulp) const {
        return
            R2{m_v[0],m_v[1]}.is_almost_equal(R2{o.m_v[0],o.m_v[1]}, ulp) &&
            R2{m_v[1],m_v[2]}.is_almost_equal(R2{o.m_v[1],o.m_v[2]}, ulp) &&
            R2{m_v[0],m_v[2]}.is_almost_equal(R2{o.m_v[0],o.m_v[2]}, ulp);
    }

    bool do_is_almost_ideal(rvgf, rvgf) const { return false; }

    std::ostream &do_print(std::ostream &out) const {
        out << "r3{" << m_v[0] << ',' << m_v[1] << ',' << m_v[2] << '}';
        return out;
    }
};

static inline R3 make_R3(rvgf x, rvgf y, rvgf z) {
    return R3(x, y, z);
}

template <typename P, typename Q,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_point<P>::value &&
        rvg::meta::is_an_i_point<Q>::value &&
        !std::is_same<P,Q>::value &&
        std::is_convertible<P,Q>::value>::type>
decltype(std::declval<Q&>().added(Q()))
operator+(const P &p, const Q &q) {
    return static_cast<Q>(p).added(q);
}

template <typename P, typename Q,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_point<P>::value &&
        rvg::meta::is_an_i_point<Q>::value &&
        !std::is_same<P,Q>::value &&
        std::is_convertible<P,Q>::value>::type>
decltype(std::declval<Q&>().subtracted(Q()))
operator-(const P &p, const Q &q) {
    return static_cast<Q>(p).subtracted(q);
}

template <typename P,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_point<P>::value>::type>
P operator*(rvgf s, const P &p) {
    return p.multiplied(s);
}

template <typename P,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_point<P>::value>::type>
decltype(std::declval<P&>().untie())
untie(const P &p) {
    return p.untie();
}

using std::tie;

static inline R2 perp(const R2 &p) {
    return R2{-p[1], p[0]};
}

static inline RP2 perp(const RP2 &p) {
    return RP2{-p[1], p[0], p[2]};
}

static inline rvgf dot(const R2 &p, const R2 &q) {
    return p[0]*q[0] + p[1]*q[1];
}

static inline rvgf cross(const R2 &p, const R2 &q) {
    return p[0]*q[1] - p[1]*q[0];
}

static inline rvgf det(const R2 &a, const R2 &b) {
    return a[0]*b[1] - b[0]*a[1];
}

static inline rvgf det(const R3 &a, const R3 &b, const R3 &c) {
    return   a[1]*b[2]*c[0] + a[2]*b[0]*c[1] + a[0]*b[1]*c[2]
           - a[0]*b[2]*c[1] - a[1]*b[0]*c[2] - a[2]*b[1]*c[0];
}

static inline rvgf det(const R2 &a, const R2 &b, const R2 &c) {
    return   a[1]*c[0] + b[0]*c[1] + a[0]*b[1]
           - a[0]*c[1] - a[1]*b[0] - b[1]*c[0];
}

static inline rvgf len2(const R2 &p) {
    return dot(p, p);
}

static inline rvgf len(const R2 &p) {
    return std::hypot(p[0], p[1]);
}

static inline rvgf sum(const R2 &p) {
    return p[0] + p[1];
}

static inline rvgf sum(const R3 &p) {
    return p[0] + p[1] + p[2];
}

static R2 inline fabs(const R2 &p) {
    return R2{std::fabs(p[0]), std::fabs(p[1])};
}

static rvgf inline fmaxabs(const R2 &p) {
    return std::fmax(std::fabs(p[0]), std::fabs(p[1]));
}

namespace util {
    template <typename P, typename T, typename = typename
        std::enable_if<
            rvg::meta::is_an_i_point<P>::value
        >::type
    >
    inline bool is_almost_equal(const P &p0, const P &p1, int ulp) {
        return p0.is_almost_equal(p1, ulp);
    }
}

template <typename TO, typename FROM>
static inline TO
project(const FROM &p);

template <>
inline
R2 project<R2,R3>(const R3 &p) {
    return R2{p[0]/p[2], p[1]/p[2]};
}

template <>
inline
R2 project<R2,RP2>(const RP2 &p) {
    return R2{p[0]/p[2], p[1]/p[2]};
}

template <>
inline
R2 project<R2,R2>(const R2 &p) {
    return p;
}

template <>
inline
rvgf project<rvgf,R2>(const R2 &p) {
    return p[0]/p[1];
}

template <>
inline
rvgf project<rvgf,RP1>(const RP1 &p) {
    return p[0]/p[1];
}

static inline R2 tangent_direction(const R3 &s, const R3 &ds, const R3 &dds) {
    auto t = R2{s[2]*ds[0]-s[0]*ds[2], s[2]*ds[1]-s[1]*ds[2]};
    if (!util::is_almost_zero(t[0]) || !util::is_almost_zero(t[1])) return t;
    return R2{s[2]*dds[0]-s[0]*dds[2], s[2]*dds[1]-s[1]*dds[2]};
}

static inline R2 tangent_direction(const R2 &, const R2 &ds, const R2 &dds) {
    if (!util::is_almost_zero(ds[0]) || !util::is_almost_zero(ds[1])) {
        return ds;
    } else {
        return dds;
    }
}

// numerator of rational derivative
// assume a = {u, v, w} and da = {u', v', w'}
// (u/w)' = (wu'-uw')/w^2
// (v/w)' = (wv'-vw')/w^2
// this function returns R2{wu'-uw', wv'-vw'}
// i.e., the numerators of (u/w)' and (v/w)'.
static inline R2 nrd(const R3 &a, const R3 &da) {
    return R2{da[0]*a[2]-da[2]*a[0], da[1]*a[2]-da[2]*a[1]};
}

static inline RP1 radius_of_curvature(const R3 &s, const R3 &ds, const R3 &dds) {
    auto pds = perp(nrd(s, ds));
    auto pdds = nrd(s, dds);
    auto w2 = s[2]*s[2];
    auto lds2 = len2(pds);
    auto den = w2*dot(pds, pdds);
    return RP1{lds2*std::sqrt(lds2), den};
}

static inline rvgf bounded_radius_of_curvature(rvgf max_r, const R3 &s,
    const R3 &ds, const R3 &dds) {
    auto rp = radius_of_curvature(s, ds, dds);
    auto r = project<rvgf>(rp);
    if (util::is_almost_zero(rp[1]) || std::fabs(r) > max_r) {
        return std::copysign(max_r, rp[1]);
    } else {
        return r;
    }
}

static inline R2 center_of_curvature(rvgf max_r,
    const R3 &s, const R3 &ds, const R3 &dds) {
    auto t = tangent_direction(s, ds, dds);
    rvgf r = bounded_radius_of_curvature(max_r, s, ds, dds);
    return project<R2>(s) + (r/len(t))*perp(t);
}

static inline RP1 radius_of_curvature(const R2 &s, const R2 &ds,
    const R2 &dds) {
    (void) s;
    auto lds2 = len2(ds);
    auto den = dot(perp(ds), dds);
    return RP1{lds2*std::sqrt(lds2), den};
}

static inline rvgf bounded_radius_of_curvature(rvgf max_r, const R2 &s,
    const R2 &ds, const R2 &dds) {
    auto rp = radius_of_curvature(s, ds, dds);
    auto r = project<rvgf>(rp);
    if (util::is_almost_zero(rp[1]) || std::fabs(r) > max_r) {
        return std::copysign(max_r, rp[1]);
    } else {
        return r;
    }
}

static inline R2 center_of_curvature(rvgf max_r, const R2 &s, const R2 &ds,
    const R2 &dds) {
    auto t = tangent_direction(s, ds, dds);
    auto r = bounded_radius_of_curvature(max_r, s, ds, dds);
    return s + (r/len(t))*perp(t);
}

static inline RP1 inverse(const RP1 &p) {
    return RP1{p[1], p[0]};
}

// Computes the trigonometric bisector of two normal vectors
static inline R2 trigonometric_bisector(const R2 &n1, const R2 &n2) {
    rvgf c = static_cast<rvgf>(dot(n1,n2) >= 0);
    rvgf s = static_cast<rvgf>(1-2*(dot(perp(n1),n2) < 0));
    auto p = n2+n1;
    auto m = perp(n1-n2);
    auto b = s*c*p+(1-c)*m;
    return b/len(b);
}

// Computes the clockwise bisector of two normal vectors
static inline R2 clockwise_bisector(const R2 &n1, const R2 &n2) {
    return trigonometric_bisector(n2, n1);
}

static inline R2 xy(const R3 &p) {
    return R2{p[0], p[1]};
}

static inline R2 xy(const R2 &p) {
    return p;
}

static inline auto
linear_solve(const R2 &a_0, const R2 &a_1, const R2 &b, R2 *x) {
    rvgf x0, x1;
    bool ok = util::linear_solve(a_0[0], a_1[0], a_0[1], a_1[1],
        b[0], b[1], &x0, &x1);
    *x = R2{x0, x1};
    return ok;
}

static inline bool positive_curvature(const R2 &s, const R2 &ds,
    const R2 &dds) {
    (void) s;
	return dot(perp(ds), dds) > 0;
}

static inline bool positive_curvature(const R3 &s, const R3 &ds, const R3 &dds) {
    auto pds = perp(nrd(s, ds));
    auto pdds = nrd(s, dds);
	return dot(pds, pdds) > 0;
}

} // namespace rvg

#endif
