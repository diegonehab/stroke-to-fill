-- Stroke-to-fill conversion program and test harness
-- Copyright (C) 2020 Diego Nehab
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as published
-- by the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- Contact information: diego.nehab@gmail.com
--
local _M = {}

local freetype = require"freetype"
local harfbuzz = require"harfbuzz"

_M.files = {
    "fonts/microsoft/times.ttf",
    "fonts/microsoft/timesbd.ttf",
    "fonts/microsoft/timesbi.ttf",
    "fonts/microsoft/timesi.ttf",
    "fonts/microsoft/andalemo.ttf",
    "fonts/microsoft/arial.ttf",
    "fonts/microsoft/arialbd.ttf",
    "fonts/microsoft/arialbi.ttf",
    "fonts/microsoft/ariali.ttf",
    "fonts/microsoft/ariblk.ttf",
    "fonts/microsoft/comic.ttf",
    "fonts/microsoft/comicbd.ttf",
    "fonts/microsoft/cour.ttf",
    "fonts/microsoft/courbd.ttf",
    "fonts/microsoft/courbi.ttf",
    "fonts/microsoft/couri.ttf",
    "fonts/microsoft/georgia.ttf",
    "fonts/microsoft/georgiab.ttf",
    "fonts/microsoft/georgiai.ttf",
    "fonts/microsoft/georgiaz.ttf",
    "fonts/microsoft/impact.ttf",
    "fonts/microsoft/trebuc.ttf",
    "fonts/microsoft/trebucbd.ttf",
    "fonts/microsoft/trebucbi.ttf",
    "fonts/microsoft/trebucit.ttf",
    "fonts/microsoft/verdana.ttf",
    "fonts/microsoft/verdanab.ttf",
    "fonts/microsoft/verdanai.ttf",
    "fonts/microsoft/verdanaz.ttf",
    "fonts/microsoft/webdings.ttf",
    "fonts/urw/C059-BdIta.t1",
    "fonts/urw/C059-Bold.t1",
    "fonts/urw/C059-Italic.t1",
    "fonts/urw/C059-Roman.t1",
    "fonts/urw/D050000L.t1",
    "fonts/urw/NimbusMonoPS-Bold.t1",
    "fonts/urw/NimbusMonoPS-BoldItalic.t1",
    "fonts/urw/NimbusMonoPS-Italic.t1",
    "fonts/urw/NimbusMonoPS-Regular.t1",
    "fonts/urw/NimbusRoman-Bold.t1",
    "fonts/urw/NimbusRoman-BoldItalic.t1",
    "fonts/urw/NimbusRoman-Italic.t1",
    "fonts/urw/NimbusRoman-Regular.t1",
    "fonts/urw/NimbusSans-Bold.t1",
    "fonts/urw/NimbusSans-BoldOblique.t1",
    "fonts/urw/NimbusSans-Oblique.t1",
    "fonts/urw/NimbusSans-Regular.t1",
    "fonts/urw/NimbusSansNarrow-BdOblique.t1",
    "fonts/urw/NimbusSansNarrow-Bold.t1",
    "fonts/urw/NimbusSansNarrow-Oblique.t1",
    "fonts/urw/NimbusSansNarrow-Regular.t1",
    "fonts/urw/P052-Bold.t1",
    "fonts/urw/P052-BoldItalic.t1",
    "fonts/urw/P052-Italic.t1",
    "fonts/urw/P052-Roman.t1",
    "fonts/urw/StandardSymbolsPS.t1",
    "fonts/urw/URWBookman-Demi.t1",
    "fonts/urw/URWBookman-DemiItalic.t1",
    "fonts/urw/URWBookman-Light.t1",
    "fonts/urw/URWBookman-LightItalic.t1",
    "fonts/urw/URWGothic-Book.t1",
    "fonts/urw/URWGothic-BookOblique.t1",
    "fonts/urw/URWGothic-Demi.t1",
    "fonts/urw/URWGothic-DemiOblique.t1",
    "fonts/urw/Z003-MediumItalic.t1",
    "fonts/google/Cabin-Bold.ttf",
    "fonts/google/Cabin-BoldItalic.ttf",
    "fonts/google/Cabin-Italic.ttf",
    "fonts/google/Cabin-Medium.ttf",
    "fonts/google/Cabin-MediumItalic.ttf",
    "fonts/google/Cabin-Regular.ttf",
    "fonts/google/Cabin-SemiBold.ttf",
    "fonts/google/Cabin-SemiBoldItalic.ttf",
    "fonts/google/CabinCondensed-Bold.ttf",
    "fonts/google/CabinCondensed-Medium.ttf",
    "fonts/google/CabinCondensed-Regular.ttf",
    "fonts/google/CabinCondensed-SemiBold.ttf",
    "fonts/google/CormorantGaramond-Bold.ttf",
    "fonts/google/CormorantGaramond-BoldItalic.ttf",
    "fonts/google/CormorantGaramond-Italic.ttf",
    "fonts/google/CormorantGaramond-Light.ttf",
    "fonts/google/CormorantGaramond-LightItalic.ttf",
    "fonts/google/CormorantGaramond-Medium.ttf",
    "fonts/google/CormorantGaramond-MediumItalic.ttf",
    "fonts/google/CormorantGaramond-Regular.ttf",
    "fonts/google/CormorantGaramond-SemiBold.ttf",
    "fonts/google/CormorantGaramond-SemiBoldItalic.ttf",
    "fonts/google/Eczar-Bold.ttf",
    "fonts/google/Eczar-ExtraBold.ttf",
    "fonts/google/Eczar-Medium.ttf",
    "fonts/google/Eczar-Regular.ttf",
    "fonts/google/Eczar-SemiBold.ttf",
    "fonts/google/GenBasB.ttf",
    "fonts/google/GenBasBI.ttf",
    "fonts/google/GenBasI.ttf",
    "fonts/google/GenBasR.ttf",
    "fonts/google/Inconsolata-Bold.ttf",
    "fonts/google/Inconsolata-Regular.ttf",
    "fonts/google/LibreBaskerville-Bold.ttf",
    "fonts/google/LibreBaskerville-Italic.ttf",
    "fonts/google/LibreBaskerville-Regular.ttf",
    "fonts/google/LibreFranklin-Black.ttf",
    "fonts/google/LibreFranklin-BlackItalic.ttf",
    "fonts/google/LibreFranklin-Bold.ttf",
    "fonts/google/LibreFranklin-BoldItalic.ttf",
    "fonts/google/LibreFranklin-ExtraBold.ttf",
    "fonts/google/LibreFranklin-ExtraBoldItalic.ttf",
    "fonts/google/LibreFranklin-ExtraLight.ttf",
    "fonts/google/LibreFranklin-ExtraLightItalic.ttf",
    "fonts/google/LibreFranklin-Italic.ttf",
    "fonts/google/LibreFranklin-Light.ttf",
    "fonts/google/LibreFranklin-LightItalic.ttf",
    "fonts/google/LibreFranklin-Medium.ttf",
    "fonts/google/LibreFranklin-MediumItalic.ttf",
    "fonts/google/LibreFranklin-Regular.ttf",
    "fonts/google/LibreFranklin-SemiBold.ttf",
    "fonts/google/LibreFranklin-SemiBoldItalic.ttf",
    "fonts/google/LibreFranklin-Thin.ttf",
    "fonts/google/LibreFranklin-ThinItalic.ttf",
    "fonts/google/Nunito-Black.ttf",
    "fonts/google/Nunito-BlackItalic.ttf",
    "fonts/google/Nunito-Bold.ttf",
    "fonts/google/Nunito-BoldItalic.ttf",
    "fonts/google/Nunito-ExtraBold.ttf",
    "fonts/google/Nunito-ExtraBoldItalic.ttf",
    "fonts/google/Nunito-ExtraLight.ttf",
    "fonts/google/Nunito-ExtraLightItalic.ttf",
    "fonts/google/Nunito-Italic.ttf",
    "fonts/google/Nunito-Light.ttf",
    "fonts/google/Nunito-LightItalic.ttf",
    "fonts/google/Nunito-Regular.ttf",
    "fonts/google/Nunito-SemiBold.ttf",
    "fonts/google/Nunito-SemiBoldItalic.ttf",
    "fonts/google/Pacifico-Regular.ttf",
    "fonts/google/ProzaLibre-Bold.ttf",
    "fonts/google/ProzaLibre-BoldItalic.ttf",
    "fonts/google/ProzaLibre-ExtraBold.ttf",
    "fonts/google/ProzaLibre-ExtraBoldItalic.ttf",
    "fonts/google/ProzaLibre-Italic.ttf",
    "fonts/google/ProzaLibre-Medium.ttf",
    "fonts/google/ProzaLibre-MediumItalic.ttf",
    "fonts/google/ProzaLibre-Regular.ttf",
    "fonts/google/ProzaLibre-SemiBold.ttf",
    "fonts/google/ProzaLibre-SemiBoldItalic.ttf",
    "fonts/google/Quicksand-Bold.ttf",
    "fonts/google/Quicksand-Light.ttf",
    "fonts/google/Quicksand-Medium.ttf",
    "fonts/google/Quicksand-Regular.ttf",
    "fonts/google/Rubik-Black.ttf",
    "fonts/google/Rubik-BlackItalic.ttf",
    "fonts/google/Rubik-Bold.ttf",
    "fonts/google/Rubik-BoldItalic.ttf",
    "fonts/google/Rubik-Italic.ttf",
    "fonts/google/Rubik-Light.ttf",
    "fonts/google/Rubik-LightItalic.ttf",
    "fonts/google/Rubik-Medium.ttf",
    "fonts/google/Rubik-MediumItalic.ttf",
    "fonts/google/Rubik-Regular.ttf",
    "fonts/google/Trirong-Black.ttf",
    "fonts/google/Trirong-BlackItalic.ttf",
    "fonts/google/Trirong-Bold.ttf",
    "fonts/google/Trirong-BoldItalic.ttf",
    "fonts/google/Trirong-ExtraBold.ttf",
    "fonts/google/Trirong-ExtraBoldItalic.ttf",
    "fonts/google/Trirong-ExtraLight.ttf",
    "fonts/google/Trirong-ExtraLightItalic.ttf",
    "fonts/google/Trirong-Italic.ttf",
    "fonts/google/Trirong-Light.ttf",
    "fonts/google/Trirong-LightItalic.ttf",
    "fonts/google/Trirong-Medium.ttf",
    "fonts/google/Trirong-MediumItalic.ttf",
    "fonts/google/Trirong-Regular.ttf",
    "fonts/google/Trirong-SemiBold.ttf",
    "fonts/google/Trirong-SemiBoldItalic.ttf",
    "fonts/google/Trirong-Thin.ttf",
    "fonts/google/Trirong-ThinItalic.ttf",
    "fonts/google/WorkSans-Black.ttf",
    "fonts/google/WorkSans-Bold.ttf",
    "fonts/google/WorkSans-ExtraBold.ttf",
    "fonts/google/WorkSans-ExtraLight.ttf",
    "fonts/google/WorkSans-Light.ttf",
    "fonts/google/WorkSans-Medium.ttf",
    "fonts/google/WorkSans-Regular.ttf",
    "fonts/google/WorkSans-SemiBold.ttf",
    "fonts/google/WorkSans-Thin.ttf",
}

local function splitwords(style)
    local t = {}
    string.gsub(style, "(%S+)", function(s) t[#t+1] = s end)
    return t
end

local function permute_helper(t, n, f, u, m)
    if n > 0 then
        for j = 1, n do
            local tj = t[j]
            u[m] = tj
            t[j] = t[n]
            permute_helper(t, n-1, f, u, m+1)
            t[j] = tj
        end
    else
        f(u)
    end
end

local function permute(t, f)
    permute_helper(t, #t, f, {}, 1)
end

local function build_face_to_filename(files)
    local face_to_filename = {}
    for i,filename in pairs(files) do
        local freetypeface = assert(freetype.face(filename, 0),
            "freetype face failed")
        local attributes = freetypeface:get_attributes()
        local style = splitwords(attributes.style_name)
        permute(style, function(u)
            local name = string.lower(attributes.face_family .. " " ..
                table.concat(u, " "))
            face_to_filename[name] = filename
        end)
    end
    return face_to_filename
end

local face_to_filename = build_face_to_filename(_M.files)

-- face cache
local faces = setmetatable({}, { __mode = "kv" })

function _M.loadface(family_and_styles_or_filename)
    -- try to interpret as family name with styles
    -- otherwise, proceed assuming it is filename
    local filename =
        face_to_filename[string.lower(family_and_styles_or_filename)] or
        face_to_filename[string.lower(family_and_styles_or_filename ..
            " Regular")] or
        family_and_styles_or_filename
    if faces[filename] then return faces[filename] end
    local freetypeface = assert(freetype.face(filename), "freetype face failed")
    local harfbuzzfont = assert(harfbuzz.font(filename), "harfbuzz font failed")
    local attributes = freetypeface:get_attributes()
    harfbuzzfont:set_scale(attributes.units_per_EM)
    harfbuzzfont:set_ft_funcs()
    f = {
        freetypeface = freetypeface,
        harfbuzzfont = harfbuzzfont,
        attributes = attributes,
        glyphs = {}
    }
    faces[filename] = f
    return f
end

return _M
