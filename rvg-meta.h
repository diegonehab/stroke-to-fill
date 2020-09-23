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
#ifndef RVG_META_H
#define RVG_META_H

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace rvg {
    namespace meta {

template <typename From, typename To>
struct is_same_or_convertible:
    std::integral_constant<bool,
        std::is_same<From,To>::value ||
        std::is_convertible<From, To>::value> {};

template <typename T>
struct remove_reference_cv {
    typedef typename
        std::remove_reference<typename
            std::remove_cv<T>::type>::type type;
};

template <typename From, typename To>
struct forward_same_or_convertible:
    std::integral_constant<
        bool,
        is_same_or_convertible<
            typename remove_reference_cv<From>::type, To>::value> {};

template <typename T, typename... REST>
struct is_all_same_or_convertible: std::true_type {};

template<typename T, typename FIRST, typename... REST>
struct is_all_same_or_convertible<T, FIRST, REST...>:
  std::integral_constant<bool,
    is_same_or_convertible<T, FIRST>::value &&
    is_all_same_or_convertible<T, REST...>::value> {};

template <template<typename> class Base, typename Derived>
struct is_crtp_of {
    static constexpr bool value = std::is_base_of<Base<Derived>, Derived>::value;
};

namespace detail {
    template <template<typename...> class Base, typename Derived>
    struct is_template_base_of_helper {
        struct no {};
        struct yes {};
        no operator()(...);
        template <typename ...T>
        yes operator()(const Base<T...> &);
    };
}

template <template<typename...> class Base, typename Derived>
using is_template_base_of = std::integral_constant<
    bool,
    std::is_same<
        typename std::result_of<
            detail::is_template_base_of_helper<Base, Derived>(const Derived &)
        >::type,
        typename detail::is_template_base_of_helper<Base, Derived>::yes
    >::value>;

// Function template that returns its nth argument
template <size_t N, typename ...As>
auto nth(As... as) -> typename std::tuple_element<N, std::tuple<As...>>::type {
    return std::get<N>(std::forward_as_tuple(as...));
}

template <size_t Index, size_t ...Indices>
auto make_index_range(std::index_sequence<Indices...>) {
    return std::index_sequence<(Index+Indices)...>{};
}

template <size_t Index, size_t J>
auto make_index_range(void) {
    return make_index_range<Index>(std::make_index_sequence<J-Index>());
}

template <size_t N, size_t ...Indices>
auto make_reverse_index_sequence(std::index_sequence<Indices...>) {
    return std::index_sequence<(N-1-Indices)...>{};
}

template <size_t N>
auto make_reverse_index_sequence(void) {
    return make_reverse_index_sequence<N>(std::make_index_sequence<N>());
}

template <typename T>
class capture {
    T m_t;
public:
    capture(T && t): m_t(std::forward<T>(t)) { ; }
    const typename std::remove_reference_t<T> &get(void) const { return m_t; }
    typename std::remove_reference_t<T> &get(void) { return m_t; }
};

template <typename T>
auto make_capture(T&& t) {
    return capture<T>(std::forward<T>(t));
}


} } // namespace rvg::meta

#endif
