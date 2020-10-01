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
#ifndef RVG_SVG_PATH_PARSE_H
#define RVG_SVG_PATH_PARSE_H

#include <type_traits>
#include <array>
#include <utility>

#include "rvg-i-path.h"
#include "rvg-i-svg-path.h"
#include "rvg-input-path-f-close-contours.h"
#include "rvg-svg-path-f-to-input-path.h"
#include "rvg-svg-path-tokenizer.h"
#include "rvg-meta.h"

namespace rvg {

namespace detail {

template <typename RET, typename CO, typename CM, typename... Args,
    size_t... Is>
typename std::enable_if<rvg::meta::is_all_same_or_convertible<rvgf, Args...>::value, RET>::type
pass_helper(CO &obj, RET (CM::*method)(Args...),
    const std::array<rvgf, sizeof...(Args)> &args,
    std::index_sequence<Is...>) {
    static_assert(sizeof...(Is) == sizeof...(Args),
        "auxiliary sequence is incompatible with arguments");
    return (obj.*method)(args[Is]...);
}

// ??D This will look a lot better with std::invoke from c++17
template <typename IT, typename RET, typename CO, typename CM, typename... Args>
typename std::enable_if<
    is_an_svg_path_token_iterator<IT>::value &&
    rvg::meta::is_all_same_or_convertible<rvgf, Args...>::value, int>::type
pass(IT &begin, const IT &end, CO &obj, RET (CM::*method)(Args...)) {
    const int NARGS = sizeof...(Args);
    std::array<rvgf, NARGS> args;
    args.fill(0.f); // avoid stupid "potentially uninitialized" warning
    for (int i = 0; i < NARGS && begin != end; i++) {
        if (begin == end) return 0;
        if (!begin->is_number()) return 0;
        args[i] = begin->value.f;
        ++begin;
    }
    pass_helper(obj, method, args, std::make_index_sequence<NARGS>{});
    return 1;
}

} // namespace detail

// ??D Make sure this reports an error if the end of string
// is reached before we are done with a command.
template <typename IT, typename SVGSINK>
typename std::enable_if<
    is_an_svg_path_token_iterator<IT>::value &&
    rvg::meta::is_an_i_svg_path<SVGSINK>::value,
int>::type
svg_path_iterate(IT token, IT end, SVGSINK &sink) {
    using detail::pass;
    if (token == end) {
        return 1;
    }
    // must start with M or m
    if (!token->is_command() ||
        (token->value.i != 'm' && token->value.i != 'M')) {
        return 0;
    }
    int command = token->value.i;
    ++token;
    while (1) {
		// depending on command, try to load the appropriate number of
		// arguments and forward them to the appropriate method
		int ret = 1;
        switch (command) {
			case 'T':
                ret = pass(token, end, sink, &SVGSINK::squad_to_abs);
                break;
			case 't':
                ret = pass(token, end, sink, &SVGSINK::squad_to_rel);
                break;
			case 'R':
                ret = pass(token, end, sink, &SVGSINK::rquad_to_abs);
                break;
			case 'r':
                ret = pass(token, end, sink, &SVGSINK::rquad_to_rel);
                break;
			case 'A':
                ret = pass(token, end, sink, &SVGSINK::svg_arc_to_abs);
                break;
			case 'a':
                ret = pass(token, end, sink, &SVGSINK::svg_arc_to_rel);
                break;
			case 'C':
                ret = pass(token, end, sink, &SVGSINK::cubic_to_abs);
                break;
			case 'c':
                ret = pass(token, end, sink, &SVGSINK::cubic_to_rel);
                break;
			case 'H':
                ret = pass(token, end, sink, &SVGSINK::hline_to_abs);
                break;
			case 'h':
                ret = pass(token, end, sink, &SVGSINK::hline_to_rel);
                break;
			case 'L':
                ret = pass(token, end, sink, &SVGSINK::line_to_abs);
                break;
			case 'l':
                ret = pass(token, end, sink, &SVGSINK::line_to_rel);
                break;
			case 'M':
                ret = pass(token, end, sink, &SVGSINK::move_to_abs);
                command = 'L';
                break;
			case 'm':
                ret = pass(token, end, sink, &SVGSINK::move_to_rel);
                command = 'l';
                break;
			case 'Q':
                ret = pass(token, end, sink, &SVGSINK::quad_to_abs);
                break;
			case 'q':
                ret = pass(token, end, sink, &SVGSINK::quad_to_rel);
                break;
			case 'S':
                ret = pass(token, end, sink, &SVGSINK::scubic_to_abs);
                break;
			case 's':
                ret = pass(token, end, sink, &SVGSINK::scubic_to_rel);
                break;
			case 'V':
                ret = pass(token, end, sink, &SVGSINK::vline_to_abs);
                break;
			case 'v':
                ret = pass(token, end, sink, &SVGSINK::vline_to_rel);
                break;
			case 'Z': case 'z':
                sink.close_path();
                break;
			default:
                return 0;
        }
        // in case of error, quit
        if (ret == 0) {
            return 0;
        }
        // if stream ended
        if (token == end) {
            sink.end();
            return 1;
        // otherwise, if next token is a command
        } else if (token->is_command()) {
            // replace  current command and advance to next token
			command = token->value.i;
            ++token;
        // if we just closed a path, next token must be a new command or end
        // otherwise, if we se a number, we are repeating the previous command
        // otherwise, quit in error
        } else if (command == 'Z' || command == 'z' || !token->is_number()) {
            return 0;
        }
    }
    return 1;
}

template <typename IT, typename SVGSINK>
typename std::enable_if<
    is_an_svg_path_token_iterator<IT>::value &&
    rvg::meta::is_an_i_svg_path<SVGSINK>::value,
int>::type
svg_path_iterate(IT begin, IT end, SVGSINK &&sink) {
    return svg_path_iterate(begin, end, sink);
}

template <typename IT, typename SINK>
typename std::enable_if<
    is_an_svg_path_token_iterator<IT>::value &&
    rvg::meta::is_an_i_path<SINK>::value,
int>::type
svg_path_iterate(IT token, IT end, SINK &&p) {
    return svg_path_iterate(token, end,
        make_svg_path_f_to_input_path(
            make_input_path_f_close_contours(
                std::forward<SINK>(p))));
}

template <typename SINK>
int svg_path_iterate(const char *svg, SINK &&p) {
    return svg_path_iterate(svg_path_tokenizer(svg),
        svg_path_tokenizer(nullptr), std::forward<SINK>(p));
}

template <typename SINK>
int svg_path_iterate(const std::initializer_list<svg_path_token> &svg,
    SINK &&p) {
    return svg_path_iterate(svg.begin(), svg.end(),
        std::forward<SINK>(p));
}

} // namespace rvg

#endif
