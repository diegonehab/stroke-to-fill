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
#ifndef RVG_I_POINT_H
#define RVG_I_POINT_H

#include "rvg-meta.h"
#include "rvg-floatint.h"

namespace rvg {

template <typename DERIVED>
class i_point {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:
    DERIVED added(const DERIVED &o) const {
        return derived().do_added(o);
    }

    DERIVED subtracted(const DERIVED &o) const {
        return derived().do_subtracted(o);
    }

    DERIVED negated(const DERIVED &o) const {
        return derived().do_negated(o);
    }

    DERIVED multiplied(rvgf s) const {
        return derived().do_multiplied(s);
    }

    DERIVED divided(rvgf s) const {
        return derived().do_divided(s);
    }

	const rvgf &component(int i) const {
        return derived().do_component(i);
	}

	rvgf &component(int i) {
        return derived().do_component(i);
	}

	bool is_zero(void) const {
		return derived().do_is_zero();
	}

	bool is_almost_equal(const DERIVED &o, int ulp = 8) const {
		return derived().do_is_almost_equal(o, ulp);
	}

	bool is_almost_ideal(int ulp = 8) const {
		return derived().do_is_almost_ideal(ulp);
	}

	std::ostream &print(std::ostream &out) const {
		return derived().do_print(out);
	}

	const rvgf &operator[](int i) const {
		return derived().do_component(i);
	}

	rvgf &operator[](int i) {
		return derived().do_component(i);
	}

	DERIVED operator+(const DERIVED &o) const {
		return derived().do_added(o);
	}

	DERIVED operator-(const DERIVED &o) const {
		return derived().do_subtracted(o);
	}

	DERIVED operator-(void) const {
		return derived().do_negated();
	}

	DERIVED operator*(rvgf s) const {
		return derived().do_multiplied(s);
	}

	DERIVED operator/(rvgf s) const {
		return derived().do_divided(s);
	}

    bool is_equal(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    bool operator==(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    bool operator!=(const DERIVED &o) const {
        return !derived().do_is_equal(o);
    }

    auto untie(void) const {
        return derived().do_untie();
    }

    // element by element product
    DERIVED operator*(const DERIVED &o) const {
        return derived().do_multiplied(o);
    }

};

template <typename DERIVED>
std::ostream &operator<<(std::ostream &out, const i_point<DERIVED> &p) {
    return p.print(out);
}

} // namespace rvg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_i_point = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::i_point,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // rvg::meta

#endif
