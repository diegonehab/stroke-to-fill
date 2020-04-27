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
#ifndef RVG_TUPLE_H
#define RVG_TUPLE_H

#include <tuple>

#include "rvg-util.h"
#include "rvg-meta.h"

namespace rvg {

// Maps function over tuple elements and return tuple with results
//
namespace detail {
    template <typename TUP, typename F, size_t ... Is>
    auto tuple_map_helper(const TUP &t, F f, std::index_sequence<Is...>) {
        return std::make_tuple(f(std::get<Is>(t))...);
    }
}

template <typename TUP, typename F>
auto tuple_map(const TUP &t, F f) {
    return detail::tuple_map_helper(t, f,
        std::make_index_sequence<std::tuple_size<TUP>::value>{});
}

// Same, but function receives element index as well
//
namespace detail {
    template <typename TUP, typename F, size_t ... Is>
    auto tuple_map_indexed_helper(const TUP &t, F f,
        std::index_sequence<Is...>) {
        return std::make_tuple(f(std::get<Is>(t), Is)...);
    }
}

template <typename TUP, typename F>
auto tuple_map_indexed(const TUP &t, F f) {
    return detail::tuple_map_indexed_helper(t, f,
        std::make_index_sequence<std::tuple_size<TUP>::value>{});
}

// Maps function over elements of two tuples and return tuple with results
//
namespace detail {
    template <typename P_TUP, typename Q_TUP,
        typename F, size_t ... Is>
    auto tuple_zip_map_helper(const P_TUP &P, const Q_TUP &Q,
        F f, std::index_sequence<Is...>) {
        return std::make_tuple(f(std::get<Is>(P), std::get<Is>(Q))...);
    }
}

template <typename P_TUP, typename Q_TUP, typename F,
    typename = typename std::enable_if<
        (std::tuple_size<P_TUP>::value ==
        std::tuple_size<P_TUP>::value)>::type>
auto tuple_zip_map(const P_TUP &P, const Q_TUP &Q, F f) {
    return detail::tuple_zip_map_helper(P, Q, f,
        std::make_index_sequence<std::tuple_size<P_TUP>::value>{});
}

// Return tuple with a selection of elements
//
template <typename TUP, size_t ... Is>
auto tuple_select(const TUP &t, std::index_sequence<Is...>) {
    return std::make_tuple(std::get<Is>(t)...);
}

// Return tuple with N first elements dropped
//
template <ptrdiff_t N, typename TUP,
    typename = typename std::enable_if<
        (N >= 0 && N <= std::tuple_size<TUP>::value)
    >::type
>
auto tuple_drop(const TUP &t) {
    return tuple_select(t, meta::make_index_range<N,
        std::tuple_size<TUP>::value>());
}

// Return tuple with last -N elements dropped
//
template <ptrdiff_t N, typename TUP,
    typename = typename std::enable_if<
        (N < 0 && -N <= std::tuple_size<TUP>::value)
    >::type
>
auto tuple_drop(const TUP &t, void * = nullptr) {
    return tuple_select(t, meta::make_index_range<0,
        static_cast<size_t>(std::tuple_size<TUP>::value + N)>());
}

// Return tuple reversed
//
template <typename TUP>
auto tuple_reverse(const TUP &t) {
    return tuple_select(t, meta::make_reverse_index_sequence<
        std::tuple_size<TUP>::value>());
}

// Reduce tuple elements through binary function
//
namespace detail {
    template <size_t I, size_t J, typename TUP, typename F, typename U,
        typename = typename std::enable_if<(I >= J)>::type>
        U tuple_reduce_helper(const TUP &, F, U u, void * = nullptr) {
        return u;
    }

    template <size_t I, size_t J, typename TUP, typename F, typename U,
        typename = typename std::enable_if<(I < J)>::type>
    U tuple_reduce_helper(const TUP &t, F f, U u) {
        return tuple_reduce_helper<I+1, J>(t, f, f(u, std::get<I>(t)));
    }
}

template <typename TUP, typename F, typename U>
U tuple_reduce(const TUP &t, F f, U u) {
    return detail::tuple_reduce_helper<0, std::tuple_size<TUP>::value>(t, f, u);
}

// Same, but function receives element index as well
//
namespace detail {
    template <size_t I, size_t J, typename TUP, typename F, typename U,
        typename = typename std::enable_if<(I >= J)>::type>
        U tuple_reduce_indexed_helper(const TUP &, F, U u, void * = nullptr) {
        return u;
    }

    template <size_t I, size_t J, typename TUP, typename F, typename U,
        typename = typename std::enable_if<(I < J)>::type>
    U tuple_reduce_indexed_helper(const TUP &t, F f, U u) {
        return tuple_reduce_indexed_helper<I+1, J>(t, f,
            f(u, std::get<I>(t), I));
    }
}

template <typename TUP, typename F, typename U>
U tuple_reduce_indexed(const TUP &t, F f, U u) {
    return detail::tuple_reduce_indexed_helper<0,
        std::tuple_size<TUP>::value>(t, f, u);
}

template <typename TUP>
auto tuple_first(const TUP &t) {
    return std::get<0>(t);
}

template <typename TUP>
auto tuple_last(const TUP &t) {
    return std::get<std::tuple_size<TUP>::value-1>(t);
}

// Invoke function on each tuple element
//
namespace detail {

    template <size_t I, size_t J, typename TUP, typename F,
         typename = typename std::enable_if<
            (I >= J)
         >::type>
    void tuple_for_each_helper(const TUP &, F, void * = nullptr) {
        ;
    }

    template <size_t I, size_t J, typename TUP, typename F,
         typename = typename std::enable_if<
            (I < J)
         >::type>
    void tuple_for_each_helper(const TUP &t, F f) {
        f(std::get<I>(t));
        tuple_for_each_helper<I+1,J>(t, f);
    }
}

template <typename TUP, typename F>
void tuple_for_each(const TUP &t, F f) {
    detail::tuple_for_each_helper<0,std::tuple_size<TUP>::value>(t, f);
}

} // namespace rvg

#endif
