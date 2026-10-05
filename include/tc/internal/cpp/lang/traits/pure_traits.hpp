#ifndef ED0D28B4_2EF2_47B2_8256_83B76C0AD537
#define ED0D28B4_2EF2_47B2_8256_83B76C0AD537

namespace tc
{

/**
 * Base structure for representing a "cleaned" type.
 *
 * Contains two typedefs:
 * - type — the type without const, pointers, and references.
 * - const_type — the const version of the cleaned type.
 *
 * Used as a base class for pure_type specializations.
 *
 * @tparam T 
 *      The original type.
 */
template<typename T>
struct base_pure_type {
    typedef T type;
    typedef const T const_type;
};

/**
 * Structure that cleans a type from const, pointers, references, and rvalue references.
 *
 * Recursively removes top-level qualifiers:
 * - const
 * - pointers (*)
 * - lvalue references (&)
 * - rvalue references (&&)
 *
 * Used to extract the "pure" underlying type, suitable for storage, type comparison,
 * and other meta-operations.
 *
 * @tparam T 
 *      The original type.
 */
template<typename T>
struct pure_type : base_pure_type<T> {};

/**
 * Removes the const qualifier.
 */
template<typename T>
struct pure_type<const T> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the array qualifier.
 */
template<typename T>
struct pure_type<T[]> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the array qualifier.
 */
template<typename T>
struct pure_type<const T[]> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the pointer qualifier.
 */
template<typename T>
struct pure_type<T*> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the const pointer qualifier.
 */
template<typename T>
struct pure_type<const T*> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the reference qualifier.
 */
template<typename T>
struct pure_type<T&> : base_pure_type<typename pure_type<T>::type> {};

/**
 * Removes the r-value reference qualifier.
 */
template<typename T>
struct pure_type<T&&> : base_pure_type<typename pure_type<T>::type> {};


template<typename T>
struct remove_extent {
    typedef T type;
};

template<typename T>
struct remove_extent<T[]> {
    typedef T type;
};

template<typename T>
struct remove_extent<const T[]> {
    typedef const T type;
};

template<typename T, std::size_t N>
struct remove_extent<const T[N]> {
    typedef const T type;
};

}

#endif /* ED0D28B4_2EF2_47B2_8256_83B76C0AD537 */
