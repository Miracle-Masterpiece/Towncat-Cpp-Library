#ifndef JSTD_CPP_LANG_UTILS_HASH_H_
#define JSTD_CPP_LANG_UTILS_HASH_H_

#include <tc/internal/cpp/lang/traits/primitive_traits.hpp>
#include <tc/internal/cpp/lang/traits/pointer_traits.hpp>
#include <tc/internal/cpp/lang/traits/cv_traits.hpp>
#include <tc/internal/cpp/lang/numbers.hpp>
#include <tc/internal/cpp/lang/utils/limits.hpp>
#include <tc/internal/cpp/lang/math/math.hpp>
#include <cstddef>

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

/**
 * @since 1.1.0
 */
template<>
struct equal_to<float> {
    bool operator() (const float& v1, const float& v2) const {
        return tc::math::abs(v1 - v2) < num_limits<float>::epsilon();
    }
};

/**
 * @since 1.1.0
 */
template<>
struct equal_to<double> {
    bool operator() (const double& v1, const double& v2) const {
        return tc::math::abs(v1 - v2) < num_limits<double>::epsilon();
    }
};

/**
 * @since 1.1.0
 */
template<>
struct equal_to<long double> {
    bool operator() (const long double& v1, const long double& v2) const {
        return tc::math::abs(v1 - v2) < num_limits<long double>::epsilon();
    }
};

} //namespace jstd



#endif//JSTD_CPP_LANG_UTILS_HASH_H_