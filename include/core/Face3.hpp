
#ifndef GRAPHICS_FACE3_HPP
#define GRAPHICS_FACE3_HPP

#include "math/Vector3.hpp"

namespace graphics {

class Face3 {
   public:
    unsigned int a;
    unsigned int b;
    unsigned int c;
    Vector3 normal;

    unsigned int materialIndex;

    Face3(unsigned int a, unsigned int b, unsigned int c, const Vector3& normal, unsigned int materialIndex)
        : a(a), b(b), c(c), normal(normal), materialIndex(materialIndex) {}
};

}  // namespace graphics

#endif
