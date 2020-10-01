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
#ifndef RVG_UNIT_TEST_H
#define RVG_UNIT_TEST_H

#include <cstdio>
#include <cstdlib>

#define unit_test(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d: failed %s\n", __FILE__, __LINE__, \
                #condition); \
            exit(1); \
        } \
    } while (0)

#endif
