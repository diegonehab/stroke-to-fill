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
#ifndef RVG_UNORM_H
#define RVG_UNORM_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <limits>

namespace rvg {

template <typename T>
class unorm {

    T m_value;

    template <typename U, typename F>
    static F conversion(const U& u,
        typename std::enable_if<
            std::is_floating_point<F>::value &&
            std::is_unsigned<U>::value, F *
        >::type = nullptr) {
        return static_cast<F>(u)*(F(1)/F(std::numeric_limits<U>::max()));
    }

    template <typename F, typename U>
    static U conversion(const F& f,
        typename std::enable_if<
            std::is_floating_point<F>::value &&
            std::is_unsigned<U>::value, U *
        >::type = nullptr) {
        return static_cast<U>(f*F(std::numeric_limits<U>::max())+F(.5));
    }


    template <typename F, typename G>
    static G conversion(const F& f,
        typename std::enable_if<
            !std::is_same<F, G>::value &&
            std::is_floating_point<F>::value &&
            std::is_floating_point<G>::value, G *
        >::type = nullptr) {
        return static_cast<G>(f);
    }

    template <typename U, typename V>
    static V conversion(const U& u,
        typename std::enable_if<
            std::is_same<U, V>::value, V *
        >::type = nullptr) {
        return u;
    }

    template <typename U, typename V>
    static V conversion(const U& u,
        typename std::enable_if<
            std::is_same<U,uint16_t>::value &&
            std::is_same<V, uint8_t>::value, V *
        >::type = nullptr) {
        return static_cast<uint8_t>((2*u + 257)/(2*257));
    }

    template <typename U, typename V>
    static V conversion(const U& u,
        typename std::enable_if<
            std::is_same<U, uint8_t>::value &&
            std::is_same<V, uint16_t>::value, V *
        >::type = nullptr) {
        return u*257;
    }

    template <typename U>
    static U multiplication(const U& a, const U& b,
        typename std::enable_if<
            std::is_floating_point<U>::value, U *
        >::type = nullptr) {
        return static_cast<U>(a*b);
    }

    // Integer unorm multiplication guarantees that the results are the same
    // as if we had converted both to a real number, performed the
    // multiplication exactly, and converted back
    template <typename U>
    static U multiplication(const U& a, const U& b,
        typename std::enable_if<
            std::is_same<U, uint8_t>::value, U *
        >::type = nullptr) {
        return static_cast<uint8_t>((2*a*b+255)/(2*255));
    }

    template <typename U>
    static U multiplication(const U& a, const U &b,
        typename std::enable_if<
            std::is_same<U, uint16_t>::value, U *
        >::type = nullptr) {
        return static_cast<uint16_t>(
            (static_cast<uint64_t>(2)*a*b+65535)/(2*65535));
    }

    template <typename U>
    static U division(const U &a, const U &b,
        typename std::enable_if<
            std::is_floating_point<U>::value, U *
        >::type = nullptr) {
        return a/b;
    }

    // Integer unorm division guarantees that the results are the same
    // as if we had converted both to a real number, performed the
    // division exactly, and converted back
    template <typename U>
    static U division(const U &a, const U &b,
        typename std::enable_if<
            std::is_same<U, uint8_t>::value, U *
        >::type = nullptr) {
        return static_cast<uint8_t>(((2*255)*a+b)/(2*b));
    }

    template <typename U>
    static U division(const U &a, const U &b,
        typename std::enable_if<
            std::is_same<U, uint16_t>::value, U *
        >::type = nullptr) {
        return static_cast<uint16_t>((static_cast<uint64_t>(2*65535)*a+b)/(2*b));
    }

    template <typename U>
    static U complement(const U& u,
        typename std::enable_if<
            std::is_unsigned<U>::value, U *
        >::type = nullptr) {
        return std::numeric_limits<U>::max() - u;
    }

    template <typename U>
    static U complement(const U& u,
        typename std::enable_if<
            std::is_floating_point<U>::value, U *
        >::type = nullptr) {
        return U{1} - u;
    }

public:

    using value_type = T;

    unorm() = default;
    unorm(const unorm<T> &o) = default;
    unorm(unorm<T> &&o) = default;
    unorm<T> &operator=(const unorm<T> &o) = default;
    unorm<T> &operator=(unorm<T> &&o) = default;

    // automatic access to raw value
    operator T() const {
        return m_value;
    }

    // automatic construction from converted unorm value
    template <typename V>
    unorm(const unorm<V> &v):
        m_value(conversion<V,T>(v)) {
        ;
    }

    // explicit construction from raw value
    template <typename V>
    explicit unorm(const V &v): m_value(static_cast<T>(v)) {
        ;
    }

    // assignment from raw value
    template <typename V>
    unorm<T> &operator=(const V &v) {
        m_value = v;
        return *this;
    }

    // assignment with conversion from unorm value
    template <typename V>
    unorm<T> &operator=(const unorm<V> &v) {
        m_value = conversion<V,T>(v);
        return *this;
    }

    template <typename V>
    bool operator==(const unorm<V> &o) const {
        return m_value == conversion<V,T>(o);
    }

    template <typename V>
    bool operator!=(const unorm<V> &o) const {
        return !(*this == o);
    }

    template <typename V>
    bool operator<(const unorm<V> &o) const {
        return m_value < conversion<V,T>(o);
    }

    template <typename V>
    bool operator>(const unorm<V> &o) const {
        return m_value > conversion<V,T>(o);
    }

    template <typename V>
    bool operator>=(const unorm<V> &o) const {
        return !(*this < o);
    }

    template <typename V>
    bool operator<=(const unorm<V> &o) const {
        return !(*this > o);
    }

    template <typename V>
    bool operator==(const V &o) const {
        return m_value == o;
    }

    template <typename V>
    bool operator!=(const V &o) const {
        return !(*this == o);
    }

    template <typename V>
    bool operator<(const V &o) const {
        return m_value < o;
    }

    template <typename V>
    bool operator>(const V &o) const {
        return m_value > o;
    }

    template <typename V>
    bool operator>=(const V &o) const {
        return !(*this < o);
    }

    template <typename V>
    bool operator<=(const V &o) const {
        return !(*this > o);
    }


    template <typename V>
    unorm<T> operator+(const V &o) const {
        return unorm<T>{m_value + o};
    }

    template <typename V>
    unorm<T> operator+(const unorm<V> &o) const {
        return unorm<T>{m_value + conversion<V,T>(o)};
    }

    template <typename V>
    unorm<T> &operator+=(const V &o) {
        m_value += o;
        return *this;
    }

    template <typename V>
    unorm<T> &operator+=(const unorm<V> &o) {
        m_value += conversion<V,T>(o);
        return *this;
    }

    template <typename V>
    unorm<T> operator-(const V &o) const {
        return unorm<T>{m_value - o};
    }

    template <typename V>
    unorm<T> operator-(const unorm<V> &o) const {
        return unorm<T>{m_value - conversion<V,T>(o)};
    }

    template <typename V>
    unorm<T> &operator-=(const V &o) {
        m_value -= o;
        return *this;
    }

    template <typename V>
    unorm<T> &operator-=(const unorm<V> &o) {
        m_value -= conversion<V,T>(o);
        return *this;
    }


    template <typename V>
    unorm<T> operator*(const V &o) const {
        return unorm<T>{m_value*o};
    }

    template <typename V>
    unorm<T> operator*(const unorm<V> &o) const {
        return unorm<T>{
            multiplication<T>(m_value, conversion<V,T>(o))
        };
    }

    template <typename V>
    unorm<T> &operator*=(const V &o) {
        m_value *= o;
        return *this;
    }

    template <typename V>
    unorm<T> &operator*=(const unorm<V> &o) {
        m_value = multiplication<T>(m_value, conversion<V,T>(o));
        return *this;
    }

    template <typename V>
    unorm<T> operator/(const V &o) const {
        return unorm<T>{m_value/o};
    }

    template <typename V>
    unorm<T> operator/(const unorm<V> &o) const {
        return unorm<T>{
            division<T>(m_value, conversion<V,T>(o))
        };
    }

    template <typename V>
    unorm<T> &operator/=(const V &o) {
        m_value /= o;
        return *this;
    }

    template <typename V>
    unorm<T> &operator/=(const unorm<V> &o) {
        m_value = division<T>(m_value, conversion<V,T>(o));
        return *this;
    }

    unorm<T> operator~(void) const {
        return unorm<T>{ complement<T>(m_value) };
    }

    template <typename U>
    static unorm<T> clamped(const U &u,
        typename std::enable_if<
            std::is_floating_point<T>::value &&
            std::is_arithmetic<U>::value, T *
        >::type = nullptr) {
        return unorm<T>{std::fmin(T{1}, std::fmax(T{0}, u))};
    }

    template <typename U>
    static unorm<T> clamped(const U &u,
        typename std::enable_if<
            std::is_integral<T>::value &&
            std::is_arithmetic<U>::value, T *
        >::type = nullptr) {
        using I = typename std::common_type<T, U>::type;
        constexpr I ma = std::numeric_limits<T>::max();
        return unorm<T>{std::min(ma, std::max(I{0}, I{u}))};
    }

};

using unorm8 = unorm<uint8_t>;

template <typename F>
inline unorm<F> add_gamma(unorm<F> f, typename std::enable_if<
    std::is_floating_point<F>::value, void *>::type = nullptr) {
    const float a = F(0.055);
    if (f <= F(0.0031308)) return unorm<F>{F(12.92*f)};
    else return unorm<F>{(F(1) + a)*std::pow(f, F(1./2.4))-a};
}

template <typename F>
inline unorm<F> remove_gamma(unorm<F> f, typename std::enable_if<
    std::is_floating_point<F>::value, void *>::type = nullptr) {
	const F a = F(0.055);
	if (f <= F(0.04045f)) return unorm<F>{f*F(1./12.92)};
	else return unorm<F>{std::pow((f+a)/(F(1)+a), F(2.4))};
}

template <typename T, typename = typename std::enable_if<
    std::is_unsigned<T>::value>::type
>
inline unorm<T> add_gamma(unorm<T> t);

template <typename T, typename = typename std::enable_if<
    std::is_unsigned<T>::value>::type
>
inline unorm<T> remove_gamma(unorm<T> t);

extern const uint8_t gamma_addition_uint8_t[];
extern const uint8_t gamma_removal_uint8_t[];

template <>
inline unorm8 add_gamma(unorm8 v) {
	return unorm8{gamma_addition_uint8_t[v]};
}

template <>
inline unorm8 remove_gamma(unorm8 v) {
	return unorm8{gamma_removal_uint8_t[v]};
}

template <>
inline unorm<uint16_t> add_gamma(unorm<uint16_t> u) {
    return unorm<uint16_t>{add_gamma(unorm<float>{u})};
}

template <>
inline unorm<uint16_t> remove_gamma(unorm<uint16_t> u) {
    return unorm<uint16_t>{remove_gamma(unorm<float>{u})};
}

template <typename U, typename V>
class unorm_converter {
public:
    V operator()(U u) {
        return unorm<V>(unorm<U>{u});
    }
};


} // namespace rvg

#endif
