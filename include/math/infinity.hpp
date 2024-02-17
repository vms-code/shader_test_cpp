
#ifndef GRAPHICS_INFINITY_HPP
#define GRAPHICS_INFINITY_HPP

#include <limits>

namespace graphics {

template <typename T>
constexpr T Infinity = std::numeric_limits<T>::infinity();

}

#endif
