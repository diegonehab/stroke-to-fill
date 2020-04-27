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
#include <iostream>

#include "rvg-svg-path-token.h"

namespace rvg {

std::ostream &operator<<(std::ostream &out, const svg_path_token &token) {
    out << "svg_path_token{";
    switch (token.type) {
        case svg_path_token::e_type::command:
            out << "command, '" << static_cast<char>(token.value.i) << '\'';
            break;
        case svg_path_token::e_type::number:
            out << "number, " << token.value.f;
            break;
        case svg_path_token::e_type::underflow: out << "underflow, " << token.value.i;
            break;
        case svg_path_token::e_type::overflow: out << "overflow, " << token.value.i;
            break;
        case svg_path_token::e_type::error: out << "error, " << token.value.i;
            break;
        case svg_path_token::e_type::end: out << "end";
            break;
    }
    out << '}';
    return out;
}

} // namespace rvg
