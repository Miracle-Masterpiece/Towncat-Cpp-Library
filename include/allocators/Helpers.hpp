#ifndef ALLOCATORS_HELPERS_H
#define ALLOCATORS_HELPERS_H

#include <cctype>
#include <cstddef>

#ifndef NDEBUG
# include <cassert>
#endif

namespace tca
{
    
    template<typename T>
    constexpr T align_up(T size, T align) {
        return (align + size) & ~(align - 1);
    }

    template<typename T>
    T calc_padding_for(T p, std::size_t align) {
        return align_up(p, align) - p;
    }
}

#endif//ALLOCATORS_HELPERS_H