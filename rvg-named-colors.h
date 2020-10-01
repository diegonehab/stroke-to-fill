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
#ifndef RVG_NAMED_COLORS_H
#define RVG_NAMED_COLORS_H

#include <unordered_map>
#include <string>

#include "rvg-rgba.h"

namespace rvg {
    namespace named_colors {

// Each name maps to a single color
extern std::unordered_map<std::string, RGBA8> string_to_RGBA8;
// A color can have many names
extern std::unordered_multimap<RGBA8, std::string> RGBA8_to_string;

extern RGBA8 aliceblue;
extern RGBA8 antiquewhite;
extern RGBA8 aqua;
extern RGBA8 aquamarine;
extern RGBA8 azure;
extern RGBA8 beige;
extern RGBA8 bisque;
extern RGBA8 black;
extern RGBA8 blanchedalmond;
extern RGBA8 blue;
extern RGBA8 blueviolet;
extern RGBA8 brown;
extern RGBA8 burlywood;
extern RGBA8 cadetblue;
extern RGBA8 chartreuse;
extern RGBA8 chocolate;
extern RGBA8 coral;
extern RGBA8 cornflowerblue;
extern RGBA8 cornsilk;
extern RGBA8 crimson;
extern RGBA8 cyan;
extern RGBA8 darkblue;
extern RGBA8 darkcyan;
extern RGBA8 darkgoldenrod;
extern RGBA8 darkgray;
extern RGBA8 darkgreen;
extern RGBA8 darkgrey;
extern RGBA8 darkkhaki;
extern RGBA8 darkmagenta;
extern RGBA8 darkolivegreen;
extern RGBA8 darkorange;
extern RGBA8 darkorchid;
extern RGBA8 darkred;
extern RGBA8 darksalmon;
extern RGBA8 darkseagreen;
extern RGBA8 darkslateblue;
extern RGBA8 darkslategray;
extern RGBA8 darkslategrey;
extern RGBA8 darkturquoise;
extern RGBA8 darkviolet;
extern RGBA8 deeppink;
extern RGBA8 deepskyblue;
extern RGBA8 dimgray;
extern RGBA8 dimgrey;
extern RGBA8 dodgerblue;
extern RGBA8 firebrick;
extern RGBA8 floralwhite;
extern RGBA8 forestgreen;
extern RGBA8 fuchsia;
extern RGBA8 gainsboro;
extern RGBA8 ghostwhite;
extern RGBA8 gold;
extern RGBA8 goldenrod;
extern RGBA8 gray;
extern RGBA8 green;
extern RGBA8 greenyellow;
extern RGBA8 grey;
extern RGBA8 honeydew;
extern RGBA8 hotpink;
extern RGBA8 indianred;
extern RGBA8 indigo;
extern RGBA8 ivory;
extern RGBA8 khaki;
extern RGBA8 lavender;
extern RGBA8 lavenderblush;
extern RGBA8 lawngreen;
extern RGBA8 lemonchiffon;
extern RGBA8 lightblue;
extern RGBA8 lightcoral;
extern RGBA8 lightcyan;
extern RGBA8 lightgoldenrodyellow;
extern RGBA8 lightgray;
extern RGBA8 lightgreen;
extern RGBA8 lightgrey;
extern RGBA8 lightpink;
extern RGBA8 lightsalmon;
extern RGBA8 lightseagreen;
extern RGBA8 lightskyblue;
extern RGBA8 lightslategray;
extern RGBA8 lightslategrey;
extern RGBA8 lightsteelblue;
extern RGBA8 lightyellow;
extern RGBA8 lime;
extern RGBA8 limegreen;
extern RGBA8 linen;
extern RGBA8 magenta;
extern RGBA8 maroon;
extern RGBA8 mediumaquamarine;
extern RGBA8 mediumblue;
extern RGBA8 mediumorchid;
extern RGBA8 mediumpurple;
extern RGBA8 mediumseagreen;
extern RGBA8 mediumslateblue;
extern RGBA8 mediumspringgreen;
extern RGBA8 mediumturquoise;
extern RGBA8 mediumvioletred;
extern RGBA8 midnightblue;
extern RGBA8 mintcream;
extern RGBA8 mistyrose;
extern RGBA8 moccasin;
extern RGBA8 navajowhite;
extern RGBA8 navy;
extern RGBA8 oldlace;
extern RGBA8 olive;
extern RGBA8 olivedrab;
extern RGBA8 orange;
extern RGBA8 orangered;
extern RGBA8 orchid;
extern RGBA8 palegoldenrod;
extern RGBA8 palegreen;
extern RGBA8 paleturquoise;
extern RGBA8 palevioletred;
extern RGBA8 papayawhip;
extern RGBA8 peachpuff;
extern RGBA8 peru;
extern RGBA8 pink;
extern RGBA8 plum;
extern RGBA8 powderblue;
extern RGBA8 purple;
extern RGBA8 red;
extern RGBA8 rosybrown;
extern RGBA8 royalblue;
extern RGBA8 saddlebrown;
extern RGBA8 salmon;
extern RGBA8 sandybrown;
extern RGBA8 seagreen;
extern RGBA8 seashell;
extern RGBA8 sienna;
extern RGBA8 silver;
extern RGBA8 skyblue;
extern RGBA8 slateblue;
extern RGBA8 slategray;
extern RGBA8 slategrey;
extern RGBA8 snow;
extern RGBA8 springgreen;
extern RGBA8 steelblue;
extern RGBA8 tan;
extern RGBA8 teal;
extern RGBA8 thistle;
extern RGBA8 tomato;
extern RGBA8 turquoise;
extern RGBA8 violet;
extern RGBA8 wheat;
extern RGBA8 white;
extern RGBA8 whitesmoke;
extern RGBA8 yellow;

} } // named_colors

#endif
