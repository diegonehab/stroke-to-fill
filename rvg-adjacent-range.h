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
#ifndef RVG_ADJACENT_RANGE
#define RVG_ADJACENT_RANGE

#include <iterator>
#include <utility>


namespace rvg {

template <typename FwdIt> class adjacent_iterator {
public:
    adjacent_iterator(FwdIt first, FwdIt last)
        : m_first(first), m_next(first == last ? first : std::next(first)) { }

    bool operator!=(const adjacent_iterator& other) const {
        return m_next != other.m_next; // NOT m_first!
    }

    adjacent_iterator& operator++() {
        ++m_first;
        ++m_next;
        return *this;
    }

    typedef typename std::iterator_traits<FwdIt>::reference Ref;
    typedef std::pair<Ref, Ref> Pair;

    Pair operator*() const {
        return Pair(*m_first, *m_next); // NOT std::make_pair()!
    }

private:
    FwdIt m_first;
    FwdIt m_next;
};

template <typename FwdIt> class adjacent_range {
public:
    adjacent_range(FwdIt first, FwdIt last)
        : m_first(first), m_last(last) { }

    adjacent_iterator<FwdIt> begin() const {
        return adjacent_iterator<FwdIt>(m_first, m_last);
    }

    adjacent_iterator<FwdIt> end() const {
        return adjacent_iterator<FwdIt>(m_last, m_last);
    }

private:
    FwdIt m_first;
    FwdIt m_last;
};

template <typename C> auto make_adjacent_range(C& c) -> adjacent_range<decltype(c.begin())> {
    return adjacent_range<decltype(c.begin())>(c.begin(), c.end());
}

}

#endif
