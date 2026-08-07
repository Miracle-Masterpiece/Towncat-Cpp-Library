#ifndef ALLOCATORS_HELPERS_H
#define ALLOCATORS_HELPERS_H

#include <cstddef>
#define TC_ALIGN_ASSERT(ptr, align) assert(((std::uintptr_t) ptr % align) == 0)

namespace tca
{
    template<typename T>
    constexpr T align_up(T size, T align) {
        return (size + align - 1) & ~(align - 1);
    }

    template<typename T>
    T calc_padding_for(T p, std::size_t align) {
        return align_up(p, align) - p;
    }
}

#endif//ALLOCATORS_HELPERS_H