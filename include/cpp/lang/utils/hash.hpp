#ifndef JSTD_CPP_LANG_UTILS_HASH_H_
#define JSTD_CPP_LANG_UTILS_HASH_H_

#include <cpp/lang/traits/primitive_traits.hpp>
#include <cpp/lang/traits/pointer_traits.hpp>
#include <cstdint>

namespace tc
{

namespace internal
{

/**
 * Primary template for a disabled hash functor.
 *
 * This is a no-op base template. When the second template parameter is false,
 * this template provides an empty, default-constructible type.
 *
 * @tparam T
 *      The type for which hashing is being considered.
 *
 * @tparam bool
 *      A boolean flag indicating whether the hash should be disabled.
 */
template<typename T, bool>
struct disabled_hash {

};

/**
 * Partial specialization of disabled_hash for types that are NOT hashable.
 *
 * When the flag is true, this specialization deletes both the constructor
 * and destructor, effectively disabling the hash functor for non-primitive
 * and non-pointer types.
 *
 * This prevents accidental usage of the default hash implementation for
 * types that do not support it.
 *
 * @tparam T
 *      The type for which hashing is disabled.
 *
 * @note
 *      Deleting the destructor ensures that the type cannot be instantiated
 *      even as a temporary or in any context that requires construction.
 *      This provides a compile-time error when the user attempts to use
 *      hash_for with unsupported types.
 */
template<typename T>
struct disabled_hash<T, true> {
    disabled_hash()     = delete;
    ~disabled_hash()    = delete;
};

} //namespace internal

/**
 * Default hash functor for the library.
 *
 * Provides a default hash implementation for primitive types and pointers
 * by directly casting the key value to std::size_t.
 *
 * For non-primitive, non-pointer types, this template inherits from
 * internal::disabled_hash, which deletes the constructor and destructor,
 * causing a compile-time error upon instantiation.
 *
 * @tparam K
 *      The key type for which the hash is computed.
 *
 * @param key
 *      The key to be hashed.
 *
 * @return
 *      A hash code as std::size_t.
 *
 * @note
 *      This hash functor is enabled only for:
 *      - Primitive types (as defined by is_primitive<K>)
 *      - Pointer types (K*)
 *
 * @warning
 *      Attempting to use hash_for with a non-primitive, non-pointer type
 *      will result in a compile-time error because the type is not
 *      constructible (constructor and destructor are deleted).
 *
 * @example
 *      // Works for primitive types:
 *      hash_for<int> h1;
 *      auto hash1 = h1(42);  // OK
 *
 *      // Works for pointers:
 *      hash_for<int*> h2;
 *      int value = 10;
 *      auto hash2 = h2(&value);  // OK
 *
 *      // Compile-time error for custom types:
 *      struct MyType {};
 *      hash_for<MyType> h3;  // ERROR: deleted constructor
 *
 * @see
 *      internal::disabled_hash
 *      is_primitive
 *      is_pointer
 */
template<typename K>
struct hash_for : internal::disabled_hash<K, (!is_primitive<K>::value && !is_pointer<K>::value) > {
    std::size_t operator() (const K& key) const {
        return static_cast<std::size_t>(key);
    }
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