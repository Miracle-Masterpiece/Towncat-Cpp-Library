#ifndef _ALLOCATORS_UTILS_H_
#define _ALLOCATORS_UTILS_H_

#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/numbers.hpp>
#include <cpp/lang/system.hpp>
#include <cstdint>
#include <utility>
#include <cstring>
#include <climits>

namespace tc
{

namespace utils
{

namespace internal
{
    /**
     * Recursive byte-swap for integer types.
     *
     * Reverses the byte order of value x of type T with size SZ bytes.
     *
     * @tparam T
     *      Unsigned integer type.
     *
     * @tparam SZ
     *      Size of T in bytes (must be a power of two).
     *
     * @param x
     *      Value to byte-swap.
     *
     * @return
     *      Byte-swapped value.
     *
     * @note
     *      Recursively swaps halves: for SZ > 1, the lower and upper halves
     *      are swapped after each is individually byte-reversed.
     */
    template<typename T, std::size_t SZ>
    T bswap(T x) {
        if (SZ == 1)
        {
            return x & ~ ((unsigned char) 0);
        }
        
        const std::size_t HSZ   = SZ / 2;
        const std::size_t HBS   = CHAR_BIT * HSZ;
        const std::size_t HMASK = (T(1) << HBS) - T(1);
        
        return (bswap<T, HSZ>(x & HMASK) << HBS) | bswap<T, HSZ>(x >> HBS);
    }
}
    
    /**
     * Byte-swaps an integer value.
     * 
     * @tparam T
     *      The integer type of the value to swap.
     *
     * @param x
     *      The value to byte-swap.
     *
     * @return
     *      Byte-swapped value.
     *
     * @note
     *      The input is cast to unsigned to ensure defined behavior during shifts.
     *      The result is cast back to T, preserving the binary representation.
     */
    template<typename T>
    T bswap(T x) {
        typedef typename make_unsigned<T>::type U;
        return static_cast<T>(internal::bswap<U, sizeof(U)>(static_cast<U>(x)));
    }
    
    /**
     * Byte-swaps an float value.
     * 
     * @param x
     *      The float value to byte-swap.
     *
     * @return
     *      Byte-swapped float value.
     */
    template<>
    inline float bswap(float x) {
        uint_float_bits fbits;
        std::memcpy(&fbits, &x, sizeof(float));
        fbits = bswap<uint_float_bits>(fbits);
        std::memcpy(&x, &fbits, sizeof(float));
        return x;
    }
    
    /**
     * Byte-swaps an double value.
     * 
     * @param x
     *      The double value to byte-swap.
     *
     * @return
     *      Byte-swapped double value.
     */
    template<>
    inline double bswap(double x) {
        uint_double_bits dbits;
        std::memcpy(&dbits, &x, sizeof(double));
        dbits = bswap<uint_double_bits>(dbits);
        std::memcpy(&x, &dbits, sizeof(double));
        return x;
    }

    /**
     * Assigns a source value to a target using static_cast with perfect forwarding.
     *
     * @tparam TARGET_T
     *      The target type.
     *
     * @tparam SOURCE_T
     *      The source type (deduced, supports perfect forwarding).
     *
     * @param t
     *      Target reference to assign to.
     *
     * @param s
     *      Source value to cast and assign (forwarded as rvalue reference).
     *
     * @note
     *      Equivalent to: t = static_cast<TARGET_T>(std::forward<SOURCE_T>(s));
     */
    template<typename TARGET_T, typename SOURCE_T>
    static void assign_static_cast(TARGET_T& t, SOURCE_T&& s) {
        t = static_cast<TARGET_T>(std::forward<SOURCE_T>(s));
    }

    /**
     * Copies the T type from one memory location to another, reversing the byte order.
     * Memory locations may overlap.
     *
     * Used to reorder the byte order during copying.
     *
     * @param d
     *      Pointer to the memory location where the type will be copied.
     *
     * @param s
     *      Pointer to the source from which to copy the type.
     */
    template<typename T>
    void swap_copy(void* d, const void* s) {
        T tmp;
        std::memcpy(&tmp, s, sizeof(T));
        tmp = bswap(tmp);
        std::memcpy(d, &tmp, sizeof(T));
    }

    /**
     * Reads a value from a pointer with byte order conversion.
     *
     * @warning This function only works with primitives!
     *
     * For types larger than 1 byte, performs byte order conversion
     * if the target order (out_order) does not match the system order.
     * For single-byte types, returns the value unchanged.
     *
     * @tparam T
     * The type of the value to read.
     *
     * @param ptr
     * A pointer to the value to read.
     *
     * @param out_order
     * The byte order of the returned value.
     *
     * @return
     * The read value, converted to the transmitted byte order.
     *
     * @since 1.0
     *
     * @IntrinsicCandidate
     */
    template<typename T>
    T read_with_order(const void* ptr, byte_order out_order) {
        static_assert(tc::is_primitive<T>::value, "T must be primitive");
        T v;
        std::memcpy(&v, ptr, sizeof(T));
        if (out_order != system::native_byte_order())
            v = bswap<T>(v);
        return v;
    }
    
    /**
     * Writes the value at the pointer, converting the byte order.
     *
     * @warning This function only works with primitives!
     *
     * For types larger than 1 byte, performs a byte order conversion
     * if the target order (out_order) does not match the system order.
     * For single-byte types, writes the value unchanged.
     *
     * @tparam T
     * The type of the value to write.
     *
     * @param ptr
     * The pointer to which the value will be written.
     *
     * @param v
     * The value to write.
     *
     * @param out_order
     * The byte order to write.
     *
     * @since 1.0
     *
     * @IntrinsicCandidate
     */
    template<typename T>
    void write_with_order(void* ptr, T v, byte_order out_order) {
        static_assert(is_primitive<T>::value, "T must be primitive");
        if (out_order != system::native_byte_order())
            v = bswap<T>(v);
        std::memcpy(ptr, &v, sizeof(T));
    }
}

}

#endif//_ALLOCATORS_UTILS_H_