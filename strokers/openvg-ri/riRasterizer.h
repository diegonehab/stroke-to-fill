#ifndef __RIRASTERIZER_H
#define __RIRASTERIZER_H

#ifndef __RIMATH_H
#include "riMath.h"
#endif

namespace OpenVGRI {

class Rasterizer {
public:
	virtual ~Rasterizer() { ; }
	virtual void clear() = 0;
	virtual void addEdge(const Vector2& v0, const Vector2& v1) = 0;
	virtual void fill() = 0;
};

}

#endif /* __RIRASTERIZER_H */
