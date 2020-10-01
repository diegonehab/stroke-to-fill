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
template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value &&
        rvg::meta::is_an_i_xform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,projectivity>::value &&
        std::is_convertible<N,projectivity>::value &&
        (!std::is_convertible<M,affinity>::value ||
         !std::is_convertible<N,affinity>::value)>::type>
projectivity operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
    std::cerr << "projectivity operator*(static_cast<projectivity>, static_cast<projectivity>)\n";
#endif
    return static_cast<projectivity>(n).transformed(static_cast<projectivity>(m));
}

template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value &&
        rvg::meta::is_an_i_xform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,affinity>::value &&
        std::is_convertible<N,affinity>::value &&
        (!std::is_convertible<M,linearity>::value ||
        !std::is_convertible<N,linearity>::value)>::type>
affinity operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(static_cast<affinity>, static_cast<affinity>)\n";
#endif
    return static_cast<affinity>(n).transformed(static_cast<affinity>(m));
}

template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value &&
        rvg::meta::is_an_i_xform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,linearity>::value &&
        std::is_convertible<N,linearity>::value>::type>
linearity operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
    std::cerr << "linearity operator*(static_cast<linearity>, static_cast<linearity>)\n";
#endif
    return static_cast<linearity>(n).transformed(static_cast<linearity>(m));
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value &&
        !std::is_same<M, identity>::value>::type>
M operator*(const M &m, const identity &) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(const i_xform &, const identity &)\n";
#endif
    return m;
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value &&
        !std::is_same<M, identity>::value>::type>
M operator*(const identity &, const M &m) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(const i_xform &, const identity &)\n";
#endif
    return m;
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_i_xform<M>::value>::type>
decltype(std::declval<M&>().scaled(rvgf()))
operator*(rvgf s, const M &m)
{
#ifdef XFORM_DEBUG
        std::cerr << "operator*(rvgf s, const i_xform &)\n";
#endif
    return m.scaled(s);
}
