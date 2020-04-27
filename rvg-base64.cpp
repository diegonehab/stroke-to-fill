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
#include <sstream>

#define BUFFERSIZE 1<<12
#include <b64/encode.h>
#include <b64/decode.h>

namespace rvg {

std::string encode_base64(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    base64::encoder E;
    E.encode(sin, sout);
    return sout.str();
}

std::string decode_base64(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    base64::decoder E;
    E.decode(sin, sout);
    return sout.str();
}

} // namespace rvg
