#ifndef _ALLOCATORS_UTILS_H_
#define _ALLOCATORS_UTILS_H_

#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>
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
        return static_cast<T>(internal::bswap<T, sizeof(U)>(static_cast<U>(x)));
    }

    /**
     * Копирует блок памяти с изменением порядка байт в каждом элементе.
     *
     * Функция копирует данные из исходного буфера в целевой буфер,
     * меняя порядок байт в каждом элементе типа T.
     *
     * @tparam T 
     *      Тип элементов, для которых меняется порядок байт.
     * 
     * @param dst 
     *      Указатель на буфер в который будет происходить копирование.
     * 
     * @param src 
     *      Указатель на буфер из которого будет происходить копирование.
     * 
     * @param n 
     *      Количество элементов типа T.
     * 
     * @since 1.1
     * 
     * @IntrinsicCandidate
     */
    template<typename T>
    void copy_swap_memory(void* dst, const void* src, std::size_t n) {
        T tmp;
        unsigned char* dest         = reinterpret_cast<unsigned char*>(dst);
        const unsigned char* source = reinterpret_cast<const unsigned char*>(src);
        for (std::size_t i = 0; i < n; ++i) {
            std::memcpy(&tmp, source, sizeof(T));
            tmp = bswap<T>(tmp);
            std::memcpy(dest, &tmp, sizeof(T));
            dest    += sizeof(T);
            source  += sizeof(T);
        }
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
     * Читает значение из указателя с преобразованием порядка байт.
     * 
     * @warning Данная функция работает только с примитивами!
     * 
     * Для типов размером больше 1 байта выполняет преобразование порядка байт,
     * если целевой порядок (out_order) не совпадает с порядком системы.
     * Для однобайтовых типов возвращает значение без изменений.
     * 
     * @tparam T 
     *      Тип читаемого значения.
     * 
     * @param ptr 
     *      Указатель на читаемое значение.
     * 
     * @param out_order 
     *      Порядок байт возвращаемого значения.
     * 
     * @return 
     *      Прочитанное значение, преобразованное в передаваемый порядок байт.
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
     * Записывает значение по указателю с преобразованием порядка байт.
     * 
     * @warning Данная функция работает только с примитивами!
     * 
     * Для типов размером больше 1 байта выполняет преобразование порядка байт,
     * если целевой порядок (out_order) не совпадает с порядком системы.
     * Для однобайтовых типов записывает значение без изменений.
     * 
     * @tparam T 
     *      Тип записываемого значения.
     * 
     * @param ptr 
     *      Указатель, по которому будет записано значение.
     * 
     * @param v 
     *      Значение для записи.
     * 
     * @param out_order 
     *      Порядок байт для записи.
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