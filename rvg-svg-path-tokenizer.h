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
#ifndef RVG_SVG_PATH_TOKENIZER_H
#define RVG_SVG_PATH_TOKENIZER_H

#include <boost/iterator/iterator_facade.hpp>
#include <cctype>

#include "rvg-svg-path-token.h"

namespace rvg {

template <typename T> T strtodf(const char* str, char** str_end);

template <>
inline float strtodf<float>(const char* str, char** str_end) {
    return strtof(str, str_end);
}

template <>
inline double strtodf<double>(const char* str, char** str_end) {
    return strtod(str, str_end);
}

// Given a string, iterates over all tokens in it
class svg_path_tokenizer final: public boost::iterator_facade<
    svg_path_tokenizer, svg_path_token const,
    boost::forward_traversal_tag> {

    const char *m_curr;
    svg_path_token m_tok;

public:

    explicit svg_path_tokenizer(const char *curr):
        m_curr(curr) {
        next();
    }

    svg_path_tokenizer(void):
        m_curr(nullptr) {
        set(svg_path_token::e_type::end, rvgi{0});
    }

    svg_path_tokenizer(const svg_path_tokenizer &o):
        m_curr(o.m_curr),
        m_tok(o.m_tok) {
        ;
    }

    ptrdiff_t operator-(const svg_path_tokenizer &o) {
        return m_curr - o.m_curr;
    }

private:

    friend class boost::iterator_core_access;

    void increment(void) {
        next();
    }

    bool equal(const svg_path_tokenizer &other) const {
        return m_curr == other.m_curr;
    }

    svg_path_token const &dereference(void) const {
        return m_tok;
    }

    void set(svg_path_token::e_type t, floatint v) {
        m_tok.type = t; m_tok.value = v;
    }

    void next(void) {
        // trying to move past end of string?
        if (!m_curr)
            return set(svg_path_token::e_type::error, rvgi{0});

        // skip optional spaces
        while (isspace(*m_curr))
            ++m_curr;

        // skip one optional comma
        if (*m_curr== ',') {
            ++m_curr;
            // skip optional spaces following optional comma
            while (isspace(*m_curr))
                ++m_curr;
        }

        // reached end of string
        if (!(*m_curr)) {
            m_curr = nullptr;
            return set(svg_path_token::e_type::end, rvgi{0});
        }

        // try command
        if (isalpha(*m_curr))
            return set(svg_path_token::e_type::command, rvgi{*m_curr++});

        // try number
        char *end;
        rvgf value = strtodf<rvgf>(m_curr, &end);
        // error parsing number
        if (end == m_curr) {
            // advance 1 and error
            ++m_curr;
            return set(svg_path_token::e_type::error, rvgi{0});
        }
        m_curr = end;
        // overflow or underflow
        if (errno == ERANGE) {
            if (value == 0.f)
                return set(svg_path_token::e_type::underflow, rvgf{0});
            else
                return set(svg_path_token::e_type::overflow, value);
        }
        // number is ok
        return set(svg_path_token::e_type::number, value);
    }
};

} // namespace rvg

#endif
