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
#ifndef RVG_SVG_PATH_F_COMMAND_PRINTER_H
#define RVG_SVG_PATH_F_COMMAND_PRINTER_H

#include <iostream>

#include "rvg-i-svg-path.h"

namespace rvg {

class svg_path_f_command_printer final:
    public i_svg_path<svg_path_f_command_printer> {

    std::ostream &m_out;
    int m_prev;
    char m_sep;

    template <typename ...DATA>
    void print_cmd(int cmd, DATA ...data) {
        if (m_prev != cmd) {
            if (m_prev != ' ') m_out << m_sep;
            m_out << static_cast<char>(cmd);
            m_prev = cmd;
        }
        print_data(data...);
    }

    void print_data(void) { }

    template <typename ...REST>
    void print_data(rvgf f, REST... rest) {
        m_out << m_sep << f;
        print_data(rest...);
    }

public:
    svg_path_f_command_printer(std::ostream &out, char sep):
        m_out(out),
        m_prev(' '),
        m_sep(sep) { ; }

private:
    friend i_svg_path<svg_path_f_command_printer>;

    void do_move_to_abs(rvgf x0, rvgf y0) {
        print_cmd('M', x0, y0);
    }

    void do_move_to_rel(rvgf x0, rvgf y0) {
        print_cmd('m', x0, y0);
    }

    void do_close_path(void) {
        print_cmd('Z');
    }

    void do_line_to_abs(rvgf x1, rvgf y1) {
        print_cmd('L', x1, y1);
    }

    void do_line_to_rel(rvgf x1, rvgf y1) {
        print_cmd('l', x1, y1);
    }

    void do_hline_to_abs(rvgf x1) {
        print_cmd('H', x1);
    }

    void do_hline_to_rel(rvgf x1) {
        print_cmd('h', x1);
    }

    void do_vline_to_abs(rvgf y1) {
        print_cmd('V', y1);
    }

    void do_vline_to_rel(rvgf y1) {
        print_cmd('v', y1);
    }

    void do_quad_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        print_cmd('Q', x1, y1, x2, y2);
    }

    void do_quad_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2) {
        print_cmd('q', x1, y1, x2, y2);
    }

    void do_squad_to_abs(rvgf x2, rvgf y2) {
        print_cmd('T', x2, y2);
    }

    void do_squad_to_rel(rvgf x2, rvgf y2) {
        print_cmd('t', x2, y2);
    }

    void do_rquad_to_abs(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        print_cmd('R', x1, y1, w1, x2, y2);
    }

    void do_rquad_to_rel(rvgf x1, rvgf y1, rvgf w1, rvgf x2, rvgf y2) {
        print_cmd('r', x1, y1, w1, x2, y2);
    }

    void do_svg_arc_to_abs(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        print_cmd('A', rx, ry, a, fa, fs, x2, y2);
    }

    void do_svg_arc_to_rel(rvgf rx, rvgf ry, rvgf a, rvgf fa, rvgf fs,
        rvgf x2, rvgf y2) {
        print_cmd('a', rx, ry, a, fa, fs, x2, y2);
    }

    void do_cubic_to_abs(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        print_cmd('C', x1, y1, x2, y2, x3, y3);
    }

    void do_cubic_to_rel(rvgf x1, rvgf y1, rvgf x2, rvgf y2,
        rvgf x3, rvgf y3) {
        print_cmd('c', x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        print_cmd('S', x2, y2, x3, y3);
    }

    void do_scubic_to_rel(rvgf x2, rvgf y2, rvgf x3, rvgf y3) {
        print_cmd('s', x2, y2, x3, y3);
    }
};

inline svg_path_f_command_printer make_svg_path_f_command_printer(
    std::ostream &out, char sep) {
    return svg_path_f_command_printer{out, sep};
}

} // namespace rvg

#endif
