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
#ifndef RVG_PTR
#define RVG_PTR

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace rvg {

template <typename T, typename ...ARGS>
boost::intrusive_ptr<T> make_intrusive(ARGS&&... args) {
    return boost::intrusive_ptr<T>(new T(std::forward<ARGS>(args)...));
}

template <typename T, typename ...ARGS>
boost::intrusive_ptr<T> make_dashes_intrusive(ARGS&&... args) {
    return boost::intrusive_ptr<T>(new T(std::forward<ARGS>(args)...));
}

template <typename T, typename ...ARGS>
boost::intrusive_ptr<T> make_style_intrusive(ARGS&&... args) {
    return boost::intrusive_ptr<T>(new T(std::forward<ARGS>(args)...));
}

} // namespace rvg

#endif
