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
#include "rvg-named-colors.h"

namespace rvg {
    namespace named_colors {

RGBA8 aliceblue = make_rgb8(240,248,255);
RGBA8 antiquewhite = make_rgb8(250,235,215);
RGBA8 aqua = make_rgb8(0,255,255);
RGBA8 aquamarine = make_rgb8(127,255,212);
RGBA8 azure = make_rgb8(240,255,255);
RGBA8 beige = make_rgb8(245,245,220);
RGBA8 bisque = make_rgb8(255,228,196);
RGBA8 black = make_rgb8(0,0,0);
RGBA8 blanchedalmond = make_rgb8(255,235,205);
RGBA8 blue = make_rgb8(0,0,255);
RGBA8 blueviolet = make_rgb8(138,43,226);
RGBA8 brown = make_rgb8(165,42,42);
RGBA8 burlywood = make_rgb8(222,184,135);
RGBA8 cadetblue = make_rgb8(95,158,160);
RGBA8 chartreuse = make_rgb8(127,255,0);
RGBA8 chocolate = make_rgb8(210,105,30);
RGBA8 coral = make_rgb8(255,127,80);
RGBA8 cornflowerblue = make_rgb8(100,149,237);
RGBA8 cornsilk = make_rgb8(255,248,220);
RGBA8 crimson = make_rgb8(220,20,60);
RGBA8 cyan = make_rgb8(0,255,255);
RGBA8 darkblue = make_rgb8(0,0,139);
RGBA8 darkcyan = make_rgb8(0,139,139);
RGBA8 darkgoldenrod = make_rgb8(184,134,11);
RGBA8 darkgray = make_rgb8(169,169,169);
RGBA8 darkgreen = make_rgb8(0,100,0);
RGBA8 darkgrey = make_rgb8(169,169,169);
RGBA8 darkkhaki = make_rgb8(189,183,107);
RGBA8 darkmagenta = make_rgb8(139,0,139);
RGBA8 darkolivegreen = make_rgb8(85,107,47);
RGBA8 darkorange = make_rgb8(255,140,0);
RGBA8 darkorchid = make_rgb8(153,50,204);
RGBA8 darkred = make_rgb8(139,0,0);
RGBA8 darksalmon = make_rgb8(233,150,122);
RGBA8 darkseagreen = make_rgb8(143,188,143);
RGBA8 darkslateblue = make_rgb8(72,61,139);
RGBA8 darkslategray = make_rgb8(47,79,79);
RGBA8 darkslategrey = make_rgb8(47,79,79);
RGBA8 darkturquoise = make_rgb8(0,206,209);
RGBA8 darkviolet = make_rgb8(148,0,211);
RGBA8 deeppink = make_rgb8(255,20,147);
RGBA8 deepskyblue = make_rgb8(0,191,255);
RGBA8 dimgray = make_rgb8(105,105,105);
RGBA8 dimgrey = make_rgb8(105,105,105);
RGBA8 dodgerblue = make_rgb8(30,144,255);
RGBA8 firebrick = make_rgb8(178,34,34);
RGBA8 floralwhite = make_rgb8(255,250,240);
RGBA8 forestgreen = make_rgb8(34,139,34);
RGBA8 fuchsia = make_rgb8(255,0,255);
RGBA8 gainsboro = make_rgb8(220,220,220);
RGBA8 ghostwhite = make_rgb8(248,248,255);
RGBA8 gold = make_rgb8(255,215,0);
RGBA8 goldenrod = make_rgb8(218,165,32);
RGBA8 gray = make_rgb8(128,128,128);
RGBA8 green = make_rgb8(0,128,0);
RGBA8 greenyellow = make_rgb8(173,255,47);
RGBA8 grey = make_rgb8(128,128,128);
RGBA8 honeydew = make_rgb8(240,255,240);
RGBA8 hotpink = make_rgb8(255,105,180);
RGBA8 indianred = make_rgb8(205,92,92);
RGBA8 indigo = make_rgb8(75,0,130);
RGBA8 ivory = make_rgb8(255,255,240);
RGBA8 khaki = make_rgb8(240,230,140);
RGBA8 lavender = make_rgb8(230,230,250);
RGBA8 lavenderblush = make_rgb8(255,240,245);
RGBA8 lawngreen = make_rgb8(124,252,0);
RGBA8 lemonchiffon = make_rgb8(255,250,205);
RGBA8 lightblue = make_rgb8(173,216,230);
RGBA8 lightcoral = make_rgb8(240,128,128);
RGBA8 lightcyan = make_rgb8(224,255,255);
RGBA8 lightgoldenrodyellow = make_rgb8(250,250,210);
RGBA8 lightgray = make_rgb8(211,211,211);
RGBA8 lightgreen = make_rgb8(144,238,144);
RGBA8 lightgrey = make_rgb8(211,211,211);
RGBA8 lightpink = make_rgb8(255,182,193);
RGBA8 lightsalmon = make_rgb8(255,160,122);
RGBA8 lightseagreen = make_rgb8(32,178,170);
RGBA8 lightskyblue = make_rgb8(135,206,250);
RGBA8 lightslategray = make_rgb8(119,136,153);
RGBA8 lightslategrey = make_rgb8(119,136,153);
RGBA8 lightsteelblue = make_rgb8(176,196,222);
RGBA8 lightyellow = make_rgb8(255,255,224);
RGBA8 lime = make_rgb8(0,255,0);
RGBA8 limegreen = make_rgb8(50,205,50);
RGBA8 linen = make_rgb8(250,240,230);
RGBA8 magenta = make_rgb8(255,0,255);
RGBA8 maroon = make_rgb8(128,0,0);
RGBA8 mediumaquamarine = make_rgb8(102,205,170);
RGBA8 mediumblue = make_rgb8(0,0,205);
RGBA8 mediumorchid = make_rgb8(186,85,211);
RGBA8 mediumpurple = make_rgb8(147,112,219);
RGBA8 mediumseagreen = make_rgb8(60,179,113);
RGBA8 mediumslateblue = make_rgb8(123,104,238);
RGBA8 mediumspringgreen = make_rgb8(0,250,154);
RGBA8 mediumturquoise = make_rgb8(72,209,204);
RGBA8 mediumvioletred = make_rgb8(199,21,133);
RGBA8 midnightblue = make_rgb8(25,25,112);
RGBA8 mintcream = make_rgb8(245,255,250);
RGBA8 mistyrose = make_rgb8(255,228,225);
RGBA8 moccasin = make_rgb8(255,228,181);
RGBA8 navajowhite = make_rgb8(255,222,173);
RGBA8 navy = make_rgb8(0,0,128);
RGBA8 oldlace = make_rgb8(253,245,230);
RGBA8 olive = make_rgb8(128,128,0);
RGBA8 olivedrab = make_rgb8(107,142,35);
RGBA8 orange = make_rgb8(255,165,0);
RGBA8 orangered = make_rgb8(255,69,0);
RGBA8 orchid = make_rgb8(218,112,214);
RGBA8 palegoldenrod = make_rgb8(238,232,170);
RGBA8 palegreen = make_rgb8(152,251,152);
RGBA8 paleturquoise = make_rgb8(175,238,238);
RGBA8 palevioletred = make_rgb8(219,112,147);
RGBA8 papayawhip = make_rgb8(255,239,213);
RGBA8 peachpuff = make_rgb8(255,218,185);
RGBA8 peru = make_rgb8(205,133,63);
RGBA8 pink = make_rgb8(255,192,203);
RGBA8 plum = make_rgb8(221,160,221);
RGBA8 powderblue = make_rgb8(176,224,230);
RGBA8 purple = make_rgb8(128,0,128);
RGBA8 red = make_rgb8(255,0,0);
RGBA8 rosybrown = make_rgb8(188,143,143);
RGBA8 royalblue = make_rgb8(65,105,225);
RGBA8 saddlebrown = make_rgb8(139,69,19);
RGBA8 salmon = make_rgb8(250,128,114);
RGBA8 sandybrown = make_rgb8(244,164,96);
RGBA8 seagreen = make_rgb8(46,139,87);
RGBA8 seashell = make_rgb8(255,245,238);
RGBA8 sienna = make_rgb8(160,82,45);
RGBA8 silver = make_rgb8(192,192,192);
RGBA8 skyblue = make_rgb8(135,206,235);
RGBA8 slateblue = make_rgb8(106,90,205);
RGBA8 slategray = make_rgb8(112,128,144);
RGBA8 slategrey = make_rgb8(112,128,144);
RGBA8 snow = make_rgb8(255,250,250);
RGBA8 springgreen = make_rgb8(0,255,127);
RGBA8 steelblue = make_rgb8(70,130,180);
RGBA8 tan = make_rgb8(210,180,140);
RGBA8 teal = make_rgb8(0,128,128);
RGBA8 thistle = make_rgb8(216,191,216);
RGBA8 tomato = make_rgb8(255,99,71);
RGBA8 turquoise = make_rgb8(64,224,208);
RGBA8 violet = make_rgb8(238,130,238);
RGBA8 wheat = make_rgb8(245,222,179);
RGBA8 white = make_rgb8(255,255,255);
RGBA8 whitesmoke = make_rgb8(245,245,245);
RGBA8 yellow = make_rgb8(255,255,0);
RGBA8 yellowgreen = make_rgb8(154,205,50);

std::unordered_map<std::string, RGBA8> string_to_RGBA8 = {
    {"aliceblue", aliceblue},
    {"antiquewhite", antiquewhite},
    {"aqua", aqua},
    {"aquamarine", aquamarine},
    {"azure", azure},
    {"beige", beige},
    {"bisque", bisque},
    {"black", black},
    {"blanchedalmond", blanchedalmond},
    {"blue", blue},
    {"blueviolet", blueviolet},
    {"brown", brown},
    {"burlywood", burlywood},
    {"cadetblue", cadetblue},
    {"chartreuse", chartreuse},
    {"chocolate", chocolate},
    {"coral", coral},
    {"cornflowerblue", cornflowerblue},
    {"cornsilk", cornsilk},
    {"crimson", crimson},
    {"cyan", cyan},
    {"darkblue", darkblue},
    {"darkcyan", darkcyan},
    {"darkgoldenrod", darkgoldenrod},
    {"darkgray", darkgray},
    {"darkgreen", darkgreen},
    {"darkgrey", darkgrey},
    {"darkkhaki", darkkhaki},
    {"darkmagenta", darkmagenta},
    {"darkolivegreen", darkolivegreen},
    {"darkorange", darkorange},
    {"darkorchid", darkorchid},
    {"darkred", darkred},
    {"darksalmon", darksalmon},
    {"darkseagreen", darkseagreen},
    {"darkslateblue", darkslateblue},
    {"darkslategray", darkslategray},
    {"darkslategrey", darkslategrey},
    {"darkturquoise", darkturquoise},
    {"darkviolet", darkviolet},
    {"deeppink", deeppink},
    {"deepskyblue", deepskyblue},
    {"dimgray", dimgray},
    {"dimgrey", dimgrey},
    {"dodgerblue", dodgerblue},
    {"firebrick", firebrick},
    {"floralwhite", floralwhite},
    {"forestgreen", forestgreen},
    {"fuchsia", fuchsia},
    {"gainsboro", gainsboro},
    {"ghostwhite", ghostwhite},
    {"gold", gold},
    {"goldenrod", goldenrod},
    {"gray", gray},
    {"green", green},
    {"greenyellow", greenyellow},
    {"grey", grey},
    {"honeydew", honeydew},
    {"hotpink", hotpink},
    {"indianred", indianred},
    {"indigo", indigo},
    {"ivory", ivory},
    {"khaki", khaki},
    {"lavender", lavender},
    {"lavenderblush", lavenderblush},
    {"lawngreen", lawngreen},
    {"lemonchiffon", lemonchiffon},
    {"lightblue", lightblue},
    {"lightcoral", lightcoral},
    {"lightcyan", lightcyan},
    {"lightgoldenrodyellow", lightgoldenrodyellow},
    {"lightgray", lightgray},
    {"lightgreen", lightgreen},
    {"lightgrey", lightgrey},
    {"lightpink", lightpink},
    {"lightsalmon", lightsalmon},
    {"lightseagreen", lightseagreen},
    {"lightskyblue", lightskyblue},
    {"lightslategray", lightslategray},
    {"lightslategrey", lightslategrey},
    {"lightsteelblue", lightsteelblue},
    {"lightyellow", lightyellow},
    {"lime", lime},
    {"limegreen", limegreen},
    {"linen", linen},
    {"magenta", magenta},
    {"maroon", maroon},
    {"mediumaquamarine", mediumaquamarine},
    {"mediumblue", mediumblue},
    {"mediumorchid", mediumorchid},
    {"mediumpurple", mediumpurple},
    {"mediumseagreen", mediumseagreen},
    {"mediumslateblue", mediumslateblue},
    {"mediumspringgreen", mediumspringgreen},
    {"mediumturquoise", mediumturquoise},
    {"mediumvioletred", mediumvioletred},
    {"midnightblue", midnightblue},
    {"mintcream", mintcream},
    {"mistyrose", mistyrose},
    {"moccasin", moccasin},
    {"navajowhite", navajowhite},
    {"navy", navy},
    {"oldlace", oldlace},
    {"olive", olive},
    {"olivedrab", olivedrab},
    {"orange", orange},
    {"orangered", orangered},
    {"orchid", orchid},
    {"palegoldenrod", palegoldenrod},
    {"palegreen", palegreen},
    {"paleturquoise", paleturquoise},
    {"palevioletred", palevioletred},
    {"papayawhip", papayawhip},
    {"peachpuff", peachpuff},
    {"peru", peru},
    {"pink", pink},
    {"plum", plum},
    {"powderblue", powderblue},
    {"purple", purple},
    {"red", red},
    {"rosybrown", rosybrown},
    {"royalblue", royalblue},
    {"saddlebrown", saddlebrown},
    {"salmon", salmon},
    {"sandybrown", sandybrown},
    {"seagreen", seagreen},
    {"seashell", seashell},
    {"sienna", sienna},
    {"silver", silver},
    {"skyblue", skyblue},
    {"slateblue", slateblue},
    {"slategray", slategray},
    {"slategrey", slategrey},
    {"snow", snow},
    {"springgreen", springgreen},
    {"steelblue", steelblue},
    {"tan", tan},
    {"teal", teal},
    {"thistle", thistle},
    {"tomato", tomato},
    {"turquoise", turquoise},
    {"violet", violet},
    {"wheat", wheat},
    {"white", white},
    {"whitesmoke", whitesmoke},
    {"yellow", yellow},
    {"yellowgreen", yellowgreen}
};

std::unordered_multimap<RGBA8, std::string> RGBA8_to_string = {
    {aliceblue, "aliceblue"},
    {antiquewhite, "antiquewhite"},
    {aqua, "aqua"},
    {aquamarine, "aquamarine"},
    {azure, "azure"},
    {beige, "beige"},
    {bisque, "bisque"},
    {black, "black"},
    {blanchedalmond, "blanchedalmond"},
    {blue, "blue"},
    {blueviolet, "blueviolet"},
    {brown, "brown"},
    {burlywood, "burlywood"},
    {cadetblue, "cadetblue"},
    {chartreuse, "chartreuse"},
    {chocolate, "chocolate"},
    {coral, "coral"},
    {cornflowerblue, "cornflowerblue"},
    {cornsilk, "cornsilk"},
    {crimson, "crimson"},
    {cyan, "cyan"},
    {darkblue, "darkblue"},
    {darkcyan, "darkcyan"},
    {darkgoldenrod, "darkgoldenrod"},
    {darkgray, "darkgray"},
    {darkgreen, "darkgreen"},
    {darkgrey, "darkgrey"},
    {darkkhaki, "darkkhaki"},
    {darkmagenta, "darkmagenta"},
    {darkolivegreen, "darkolivegreen"},
    {darkorange, "darkorange"},
    {darkorchid, "darkorchid"},
    {darkred, "darkred"},
    {darksalmon, "darksalmon"},
    {darkseagreen, "darkseagreen"},
    {darkslateblue, "darkslateblue"},
    {darkslategray, "darkslategray"},
    {darkslategrey, "darkslategrey"},
    {darkturquoise, "darkturquoise"},
    {darkviolet, "darkviolet"},
    {deeppink, "deeppink"},
    {deepskyblue, "deepskyblue"},
    {dimgray, "dimgray"},
    {dimgrey, "dimgrey"},
    {dodgerblue, "dodgerblue"},
    {firebrick, "firebrick"},
    {floralwhite, "floralwhite"},
    {forestgreen, "forestgreen"},
    {fuchsia, "fuchsia"},
    {gainsboro, "gainsboro"},
    {ghostwhite, "ghostwhite"},
    {gold, "gold"},
    {goldenrod, "goldenrod"},
    {gray, "gray"},
    {green, "green"},
    {greenyellow, "greenyellow"},
    {grey, "grey"},
    {honeydew, "honeydew"},
    {hotpink, "hotpink"},
    {indianred, "indianred"},
    {indigo, "indigo"},
    {ivory, "ivory"},
    {khaki, "khaki"},
    {lavender, "lavender"},
    {lavenderblush, "lavenderblush"},
    {lawngreen, "lawngreen"},
    {lemonchiffon, "lemonchiffon"},
    {lightblue, "lightblue"},
    {lightcoral, "lightcoral"},
    {lightcyan, "lightcyan"},
    {lightgoldenrodyellow, "lightgoldenrodyellow"},
    {lightgray, "lightgray"},
    {lightgreen, "lightgreen"},
    {lightgrey, "lightgrey"},
    {lightpink, "lightpink"},
    {lightsalmon, "lightsalmon"},
    {lightseagreen, "lightseagreen"},
    {lightskyblue, "lightskyblue"},
    {lightslategray, "lightslategray"},
    {lightslategrey, "lightslategrey"},
    {lightsteelblue, "lightsteelblue"},
    {lightyellow, "lightyellow"},
    {lime, "lime"},
    {limegreen, "limegreen"},
    {linen, "linen"},
    {magenta, "magenta"},
    {maroon, "maroon"},
    {mediumaquamarine, "mediumaquamarine"},
    {mediumblue, "mediumblue"},
    {mediumorchid, "mediumorchid"},
    {mediumpurple, "mediumpurple"},
    {mediumseagreen, "mediumseagreen"},
    {mediumslateblue, "mediumslateblue"},
    {mediumspringgreen, "mediumspringgreen"},
    {mediumturquoise, "mediumturquoise"},
    {mediumvioletred, "mediumvioletred"},
    {midnightblue, "midnightblue"},
    {mintcream, "mintcream"},
    {mistyrose, "mistyrose"},
    {moccasin, "moccasin"},
    {navajowhite, "navajowhite"},
    {navy, "navy"},
    {oldlace, "oldlace"},
    {olive, "olive"},
    {olivedrab, "olivedrab"},
    {orange, "orange"},
    {orangered, "orangered"},
    {orchid, "orchid"},
    {palegoldenrod, "palegoldenrod"},
    {palegreen, "palegreen"},
    {paleturquoise, "paleturquoise"},
    {palevioletred, "palevioletred"},
    {papayawhip, "papayawhip"},
    {peachpuff, "peachpuff"},
    {peru, "peru"},
    {pink, "pink"},
    {plum, "plum"},
    {powderblue, "powderblue"},
    {purple, "purple"},
    {red, "red"},
    {rosybrown, "rosybrown"},
    {royalblue, "royalblue"},
    {saddlebrown, "saddlebrown"},
    {salmon, "salmon"},
    {sandybrown, "sandybrown"},
    {seagreen, "seagreen"},
    {seashell, "seashell"},
    {sienna, "sienna"},
    {silver, "silver"},
    {skyblue, "skyblue"},
    {slateblue, "slateblue"},
    {slategray, "slategray"},
    {slategrey, "slategrey"},
    {snow, "snow"},
    {springgreen, "springgreen"},
    {steelblue, "steelblue"},
    {tan, "tan"},
    {teal, "teal"},
    {thistle, "thistle"},
    {tomato, "tomato"},
    {turquoise, "turquoise"},
    {violet, "violet"},
    {wheat, "wheat"},
    {white, "white"},
    {whitesmoke, "whitesmoke"},
    {yellow, "yellow"},
    {yellowgreen, "yellowgreen"}
};

} } // named_colors
