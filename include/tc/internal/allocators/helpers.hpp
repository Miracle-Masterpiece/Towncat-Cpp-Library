#ifndef ALLOCATORS_HELPERS_H
#define ALLOCATORS_HELPERS_H

#include <cstddef>
#include <utility>

#define TC_ALIGN_ASSERT(ptr, align) assert(((uintptr_t) ptr % align) == 0)

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

namespace tc
{
    template<typename T, typename U = T>
    T exchange(T& obj, U&& new_val) {
        T tmp   = std::move(obj);
        obj     = std::move(new_val);
        return tmp;
    }

    template<typename T>
    constexpr T align_up(T size, T align) {
        return tca::align_up(size, align);
    }

    template<typename T>
    T calc_padding_for(T p, std::size_t align) {
        return tca::calc_padding_for(p, align);
    }
}

#endif//ALLOCATORS_HELPERS_H