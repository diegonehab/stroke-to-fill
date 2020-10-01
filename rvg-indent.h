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
#ifndef RVG_INDENT_H
#define RVG_INDENT_H

namespace rvg {

class indent {
    int m_n;
    const char *m_v;
public:
    explicit indent(int n = 0, const char *v = "  "): m_n(n), m_v(v) { ; }

    indent operator++(int) { indent x = *this; ++m_n; return x; }

    indent operator--(int) { indent x = *this; --m_n; return x; }

    indent &operator--() { --m_n; return *this; }

    indent &operator++() { ++m_n; return *this; }

    void print(std::ostream &out) const {
        out << '\n';
        for (int i = 0; i < m_n; i++) out << m_v;
    }
};

inline std::ostream &operator<<(std::ostream &out, const indent &id) {
    id.print(out);
    return out;
}

} // namespace rvg

#endif
