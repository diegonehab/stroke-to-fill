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

#include "rvg-lua.h"
#include "rvg-lua-facade.h"
#include "rvg-scene-f-print-rvg.h"
#include "rvg-driver-rvg-cpp.h"

namespace rvg {
    namespace driver {
        namespace rvg_cpp {

const scene &accelerate(const scene &c, const viewport &vp) {
    (void) vp;
    return c;
}

void render(const scene &c, const window &w, const viewport &v,
    std::ostream &out, const std::vector<std::string> &args) {
    auto sp = make_scene_f_print_rvg_cpp(out);
    out << "// Automatically generated. Do not modify.";
    out << "\n#include \"rvg-facade.h\"";
    out << "\nauto ";
    const char *name = "load";
    int begin = 0, end = 0;
	if (!args.empty()) {
        sscanf(args[0].c_str(), "-name:%n%*s%n", &begin, &end);
        if (begin > 0 && end > begin && args[0][end] == 0) {
            name = args[0].c_str()+begin;
        }
    }
	out << name << "(void) {";
    out << "\n  using namespace rvg_facade::driver;";
    out << "\n  auto c = scene({";
    c.get_scene_data().iterate(sp);
    out << "\n  })";
    sp.print_xform(c.get_xf(), out);
    const auto &wcor = w.corners();
    out << ";\n  auto w = window(" << wcor[0] << ',' << wcor[1] << ',' << wcor[2] << ',' << wcor[3] << ");";
    const auto &vcor = v.corners();
    out << "\n  auto v = viewport(" << vcor[0] << ',' << vcor[1] << ',' << vcor[2] << ',' << vcor[3] << ");";
    out << "\n  return rvg_facade::description{c, w, v};";
    out << "\n}\n";
}

} } } // namespace rvg::driver::rvg_cpp

// Lua version of the accelerate function.
// Since there is no acceleration, we simply
// and return the input scene unmodified.
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of render function
static int luarender(lua_State *L) {
    auto c = rvg_lua_check<rvg::scene>(L, 1);
    auto w = rvg_lua_check<rvg::window>(L, 2);
    auto v = rvg_lua_check<rvg::viewport>(L, 3);
    FILE *f = rvg_lua_check_file(L, 4);
    auto args = rvg_lua_optargs(L, 5);
    std::ostringstream sout;
    rvg::driver::rvg_cpp::render(c, w, v, sout, args);
    fwrite(sout.str().data(), 1, sout.str().size(), f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modrvglua[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.cpp"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_rvg_cpp(lua_State *L) {
    rvg_lua_facade_new_driver(L, modrvglua);
    return 1;
}
