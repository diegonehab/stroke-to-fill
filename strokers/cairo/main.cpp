#include "rvg-stroker-cairo.h"

#include "rvg-svg-path-parse.h"
#include "rvg-input-path-f-to-svg-path.h"
#include "rvg-svg-path-f-command-printer.h"
#include "rvg-shape.h"

using namespace rvg;

static shape svg_shape(const char *s) {
    auto p = make_intrusive<path_data>();
    svg_path_iterate(s, *p);
    return shape(p);
}

int main(void) {
    auto input_shape = svg_shape("m 259.45282,94.539597 c -4.375,-24.16667 76.875,-49.16667 39.375,-30");
    auto style = make_intrusive<stroke_style>();
    std::cout << R"+(<?xml version="1.0" standalone="no"?>
<svg
   xmlns:xlink="http://www.w3.org/1999/xlink"
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   version="1.1"
   viewBox="195 -5 178.06238 134.05954"
   height="130.78979"
   width="172.31844">
)+";
    xform id;
    auto output_shape = stroker::cairo_traps(input_shape, id, 108, style);
    std::cout << "  <path fill-opacity=\"0.5\" fill=\"red\" fill-rule=\"nonzero\" d=\"";
    output_shape.as_path_data_ptr()->iterate(
    	make_input_path_f_to_svg_path(
		make_svg_path_f_command_printer(std::cout, ' ')));
    std::cout << "\"/>\n";
    std::cout << "</svg>\n";
    return 0;
}
