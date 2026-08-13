#ifndef JSTD_CPP_LANG_UTILS_HASH_H_
#define JSTD_CPP_LANG_UTILS_HASH_H_

#include <cpp/lang/traits/primitive_traits.hpp>
#include <cpp/lang/traits/pointer_traits.hpp>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/numbers.hpp>
#include <cstdint>

namespace tc
{

namespace internal
{

/**
* Basic template for disabling hashing for unsupported types.
*
* By default, all constructors and destructors are removed,
* to prevent creation of a hasher object.
*/
template<typename T, bool = true>
struct disabled_hash {
    disabled_hash()  = delete;
    ~disabled_hash() = delete;
};

/**
 * Specialization for primitive types and types
 * whose hash can be calculated without user-defined specializations.
 */
template<typename T>
struct disabled_hash<T, false> {

};

/**
 * Obtaining a hash for types castable to std::size_t types
 */
template<typename T>
std::size_t get_hash(T v) {
    return static_cast<std::size_t>(v);
}

/**
 * Get a hash for a float
 */
template<>
inline std::size_t get_hash<float>(float v) {
    return static_cast<std::size_t>(num::float_to_uint_bits(v));
}

/**
 * Get a hash for a double
 */
template<>
inline std::size_t get_hash<double>(double v) {
    return static_cast<std::size_t>(num::double_to_uint_bits(v));
}

/**
 * Getting a hash for pointers
 */
template<typename T>
std::size_t get_hash(T* v) {
    return reinterpret_cast<std::size_t>(v);
}

template<typename T>
struct base_hash_for : disabled_hash<T, (!is_pointer<T>::value && !is_primitive<T>::value)> {
    std::size_t operator() (const T& t) const {
        return get_hash(t);
    }
};

} //namespace internal

template<typename T>
struct hash_for : internal:: base_hash_for<T> {
    
};

template<typename T>
struct hash_for<const T> : hash_for<T> {

};

/**
 * Компаратор равенства по умолчанию.
 * 
 * @tparam K 
 *      Тип значения, для которого выполняется сравнение.
 */
template<typename K>
struct equal_to {
    /**
     * Проверяет равенство двух значений.
     * 
     * @param v1 
     *      Первое значение.
     * 
     * @param v2 
     *      Второе значение.
     * 
     * @return 
     *      true, если значения равны, иначе false.
     */
    bool operator() (const K& v1, const K& v2) const {
        return v1 == v2;
    }
};

} //namespace jstd



#endif//JSTD_CPP_LANG_UTILS_HASH_H_