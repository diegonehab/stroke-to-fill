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
#ifndef RVG_CHRONOS_H
#define RVG_CHRONOS_H

namespace rvg {

class chronos {
public:
    chronos(void);
    void reset(void);
    double elapsed(void);
    double time(void);
private:
    double m_reset;
};

} // namespace rvg

#endif // RVG_CHRONOS_H
