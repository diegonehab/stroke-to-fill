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
#ifndef RVG_SCENE_BRACKET_H
#define RVG_SCENE_BRACKET_H

#include "rvg-xform.h"

namespace rvg {

class scene_bracket {
public:

    enum class e_type: uint8_t {
        begin_clip,
        activate_clip,
        end_clip,
        begin_fade,
        end_fade,
        begin_blur,
        end_blur,
        begin_transform,
        end_transform,
        empty
    };

    using element_id = uint32_t;

    struct begin_clip_data {
        element_id matching_bracket;
    };

    struct activate_clip_data {
        element_id matching_bracket;
    };

    struct end_clip_data {
        element_id matching_bracket;
    };

    struct begin_fade_data {
        unorm8 opacity;
    };

    struct end_fade_data {
        unorm8 opacity;
    };

    struct begin_blur_data {
        float radius;
    };

    struct end_blur_data {
        float radius;
    };

    struct begin_transform_data {
        xform xf;
    };

    struct end_transform_data {
        xform xf;
    };

private:

    e_type m_type;           // bracket type
    element_id m_element_id; // index in element array where bracket lives
    uint16_t m_depth;        // nesting depth

    //??D Change to std::variant when we switch to C++17?
    union data {
        data() { ; }
        ~data() { ; }
        begin_transform_data begin_transform;
        end_transform_data end_transform;
        begin_fade_data begin_fade;
        end_fade_data end_fade;
        begin_clip_data begin_clip;
        activate_clip_data activate_clip;
        end_clip_data end_clip;
        begin_blur_data begin_blur;
        end_blur_data end_blur;
    } m_union;

public:

    scene_bracket(void): m_type{e_type::empty} { ; }

    scene_bracket(const begin_fade_data &fade, element_id eid, uint16_t d):
        m_type{e_type::begin_fade},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.begin_fade) begin_fade_data{fade};
    }

    scene_bracket(const end_fade_data &fade, element_id eid, uint16_t d):
        m_type{e_type::end_fade},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.end_fade) end_fade_data{fade};
    }

    scene_bracket(const begin_transform_data &transform,
        element_id eid, uint16_t d):
        m_type{e_type::begin_transform},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.begin_transform) begin_transform_data{transform};
    }

    scene_bracket(const end_transform_data &transform,
        element_id eid, uint16_t d):
        m_type{e_type::end_transform},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.end_transform) end_transform_data{transform};
    }

    scene_bracket(const begin_blur_data &blur, element_id eid, uint16_t d):
        m_type{e_type::begin_blur},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.begin_blur) begin_blur_data{blur};
    }

    scene_bracket(const end_blur_data &blur, element_id eid, uint16_t d):
        m_type{e_type::end_blur},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.end_blur) end_blur_data{blur};
    }

    scene_bracket(const begin_clip_data &clip, element_id eid, uint16_t d):
        m_type{e_type::begin_clip},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.begin_clip) begin_clip_data{clip};
    }

    scene_bracket(const activate_clip_data &clip, element_id eid, uint16_t d):
        m_type{e_type::activate_clip},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.activate_clip) activate_clip_data{clip};
    }

    scene_bracket(const end_clip_data &clip, element_id eid, uint16_t d):
        m_type{e_type::end_clip},
        m_element_id{eid},
        m_depth{d} { 
        new (&m_union.end_clip) end_clip_data{clip};
    }

    scene_bracket(const scene_bracket &other):
        m_type{other.m_type},
        m_element_id{other.m_element_id},
        m_depth{other.m_depth} {
        switch (m_type) {
            case e_type::begin_clip:
                new (&m_union.begin_clip)
                    begin_clip_data(other.m_union.begin_clip);
                break;
            case e_type::activate_clip:
                new (&m_union.activate_clip)
                    activate_clip_data(other.m_union.activate_clip);
                break;
            case e_type::end_clip:
                new (&m_union.end_clip)
                    end_clip_data(other.m_union.end_clip);
                break;
            case e_type::begin_blur:
                new (&m_union.begin_blur)
                    begin_blur_data(other.m_union.begin_blur);
                break;
            case e_type::end_blur:
                new (&m_union.end_blur)
                    end_blur_data(other.m_union.end_blur);
                break;
            case e_type::begin_fade:
                new (&m_union.begin_fade)
                    begin_fade_data(other.m_union.begin_fade);
                break;
            case e_type::end_fade:
                new (&m_union.end_fade)
                    end_fade_data(other.m_union.end_fade);
                break;
            case e_type::begin_transform:
                new (&m_union.begin_transform)
                    begin_transform_data(other.m_union.begin_transform);
                break;
            case e_type::end_transform:
                new (&m_union.end_transform)
                    end_transform_data(other.m_union.end_transform);
                break;
            case e_type::empty:
                break;
        }
    }

    scene_bracket(scene_bracket &&other):
        m_type{std::move(other.m_type)},
        m_element_id{std::move(other.m_element_id)},
        m_depth{std::move(other.m_depth)} {
        switch (m_type) {
            case e_type::begin_clip:
                new (&m_union.begin_clip)
                    begin_clip_data(std::move(other.m_union.begin_clip));
                break;
            case e_type::activate_clip:
                new (&m_union.activate_clip)
                    activate_clip_data(std::move(other.m_union.activate_clip));
                break;
            case e_type::end_clip:
                new (&m_union.end_clip)
                    end_clip_data(std::move(other.m_union.end_clip));
                break;
            case e_type::begin_blur:
                new (&m_union.begin_blur)
                    begin_blur_data(std::move(other.m_union.begin_blur));
                break;
            case e_type::end_blur:
                new (&m_union.end_blur)
                    end_blur_data(std::move(other.m_union.end_blur));
                break;
            case e_type::begin_fade:
                new (&m_union.begin_fade)
                    begin_fade_data(std::move(other.m_union.begin_fade));
                break;
            case e_type::end_fade:
                new (&m_union.end_fade)
                    end_fade_data(std::move(other.m_union.end_fade));
                break;
            case e_type::begin_transform:
                new (&m_union.begin_transform)
                    begin_transform_data(std::move(other.m_union.
                        begin_transform));
                break;
            case e_type::end_transform:
                new (&m_union.end_transform)
                    end_transform_data(std::move(other.m_union.end_transform));
                break;
            case e_type::empty:
                break;
        }
    }

    const char *type_name(void) const {
        switch (m_type) {
            case e_type::begin_clip: return "begin_clip";
            case e_type::activate_clip: return "activate_clip";
            case e_type::end_clip: return "end_clip";
            case e_type::begin_fade: return "begin_fade";
            case e_type::end_fade: return "end_fade";
            case e_type::begin_blur: return "begin_blur";
            case e_type::end_blur: return "end_blur";
            case e_type::begin_transform: return "begin_transform";
            case e_type::end_transform: return "end_transform";
            case e_type::empty: return "empty";
            default: return "uninitialized?"; // shouldn't happen
        }
    }

    ~scene_bracket() {
        switch (m_type) {
            case e_type::begin_clip:
                m_union.begin_clip.~begin_clip_data();
                break;
            case e_type::activate_clip:
                m_union.activate_clip.~activate_clip_data();
                break;
            case e_type::end_clip:
                m_union.end_clip.~end_clip_data();
                break;
            case e_type::begin_blur:
                m_union.begin_blur.~begin_blur_data();
                break;
            case e_type::end_blur:
                m_union.end_blur.~end_blur_data();
                break;
            case e_type::begin_transform:
                m_union.begin_transform.~begin_transform_data();
                break;
            case e_type::end_transform:
                m_union.end_transform.~end_transform_data();
                break;
            case e_type::begin_fade:
                m_union.begin_fade.~begin_fade_data();
                break;
            case e_type::end_fade:
                m_union.end_fade.~end_fade_data();
                break;
            case e_type::empty:
                break;
        }
    }

    scene_bracket &operator=(const scene_bracket &other) {
        if (this != &other) {
            m_element_id = other.m_element_id;
            m_depth = other.m_depth;
            if (m_type == other.m_type) {
                switch (m_type) {
                    case e_type::begin_clip:
                        m_union.begin_clip = other.m_union.begin_clip;
                        break;
                    case e_type::activate_clip:
                        m_union.activate_clip = other.m_union.activate_clip;
                        break;
                    case e_type::end_clip:
                        m_union.end_clip = other.m_union.end_clip;
                        break;
                    case e_type::begin_blur:
                        m_union.begin_blur = other.m_union.begin_blur;
                        break;
                    case e_type::end_blur:
                        m_union.end_blur = other.m_union.end_blur;
                        break;
                    case e_type::begin_fade:
                        m_union.begin_fade = other.m_union.begin_fade;
                        break;
                    case e_type::end_fade:
                        m_union.end_fade = other.m_union.end_fade;
                        break;
                    case e_type::begin_transform:
                        m_union.begin_transform = other.m_union.begin_transform;
                        break;
                    case e_type::end_transform:
                        m_union.end_transform = other.m_union.end_transform;
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~scene_bracket();
                new (this) scene_bracket(other);
            }
        }
        return *this;
    }

    scene_bracket &operator=(scene_bracket &&other) {
        if (this != &other) {
            m_element_id = std::move(other.m_element_id);
            m_depth = std::move(other.m_depth);
            if (m_type == other.m_type) {
                switch (m_type) {
                    case e_type::begin_clip:
                        m_union.begin_clip =
                            std::move(other.m_union.begin_clip);
                        break;
                    case e_type::activate_clip:
                        m_union.activate_clip =
                            std::move(other.m_union.activate_clip);
                        break;
                    case e_type::end_clip:
                        m_union.end_clip = std::move(other.m_union.end_clip);
                        break;
                    case e_type::begin_blur:
                        m_union.begin_blur =
                            std::move(other.m_union.begin_blur);
                        break;
                    case e_type::end_blur:
                        m_union.end_blur = std::move(other.m_union.end_blur);
                        break;
                    case e_type::begin_fade:
                        m_union.begin_fade =
                            std::move(other.m_union.begin_fade);
                        break;
                    case e_type::end_fade:
                        m_union.end_fade = std::move(other.m_union.end_fade);
                        break;
                    case e_type::begin_transform:
                        m_union.begin_transform =
                            std::move(other.m_union.begin_transform);
                        break;
                    case e_type::end_transform:
                        m_union.end_transform =
                            std::move(other.m_union.end_transform);
                        break;
                    case e_type::empty:
                        break;
                }
            } else {
                this->~scene_bracket();
                new (this) scene_bracket(std::move(other));
            }
        }
        return *this;
    }

    e_type get_type(void) const {
        return m_type;
    }

    const begin_blur_data &get_begin_blur_data(void) const {
        return m_union.begin_blur;
    }

    const end_blur_data &get_end_blur_data(void) const {
        return m_union.end_blur;
    }

    const begin_fade_data &get_begin_fade_data(void) const {
        return m_union.begin_fade;
    }

    const end_fade_data &get_end_fade_data(void) const {
        return m_union.end_fade;
    }

    const begin_transform_data &get_begin_transform_data(void) const {
        return m_union.begin_transform;
    }

    const end_transform_data &get_end_transform_data(void) const {
        return m_union.end_transform;
    }

    const begin_clip_data &get_begin_clip_data(void) const {
        return m_union.begin_clip;
    }

    begin_clip_data &get_begin_clip_data(void) {
        return m_union.begin_clip;
    }

    const activate_clip_data &get_activate_clip_data(void) const {
        return m_union.activate_clip;
    }

    const end_clip_data &get_end_clip_data(void) const {
        return m_union.end_clip;
    }

    element_id get_element_id(void) const {
        return m_element_id;
    }

    uint16_t get_depth(void) const {
        return m_depth;
    }

};


} // namespace rvg
#endif
