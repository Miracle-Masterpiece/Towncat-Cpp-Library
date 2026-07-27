#ifndef DC7409F7_2255_4C74_B267_17B9E891E355
#define DC7409F7_2255_4C74_B267_17B9E891E355

namespace jstd
{

/**
 * Meta-structure for checking whether a type is a pointer.
 * 
 * The template determines whether type T is a pointer.
 * By default, the value is set to false.
 * For template specializations (for regular pointers and pointers to const), the value will be set to true.
 * 
 * @tparam T 
 *      The type being checked for being a pointer.
 */
template<typename T>
struct is_pointer {
    static const bool value = false;
};

/**
 * Specialization for regular pointers.
 * 
 * Specialization of the is_pointer template for types that are pointers.
 * Sets the value to true for regular pointers.
 * 
 * @tparam T 
 *      The type that is a pointer.
 */
template<typename T>
struct is_pointer<T*> {
    static const bool value = true;
};

/**
 * Specialization for pointers to const.
 * 
 * Specialization of the is_pointer template for types that are pointers to const.
 * Sets the value to true for pointers to const.
 * 
 * @tparam T 
 *      The type that is a pointer to const.
 */
template<typename T>
struct is_pointer<const T*> {
    static const bool value = true;
};

/**
 * Meta-structure for adding a pointer to a type.
 *
 * Base template. Adds a top-level pointer qualifier to type T.
 *
 * @tparam T 
 *      The type to which a pointer will be added.
 *
 * @typedef type
 *      The resulting type T*.
 */
template<typename T>
struct add_pointer {
    typedef T* type;
};

/**
 * Specialization of add_pointer for lvalue reference types.
 *
 * Converts an lvalue reference to a pointer to the referenced type.
 * For example: add_pointer<int&>::type yields int*.
 *
 * @tparam T 
 *      The referenced type.
 *
 * @typedef type
 *      The resulting type T*.
 */
template<typename T>
struct add_pointer<T&> {
    typedef T* type;
};

/**
 * Meta-structure for removing a pointer from a type.
 *
 * Base template. If T is not a pointer type, the type remains unchanged.
 *
 * @tparam T 
 *      The type to process.
 *
 * @typedef type
 *      The resulting type (T if not a pointer).
 */
template<typename T>
struct remove_pointer {
    typedef T type;
};

/**
 * Specialization of remove_pointer for pointer types.
 *
 * Strips the top-level pointer qualifier from type T*.
 *
 * @tparam T 
 *      The type being pointed to.
 *
 * @typedef type
 *      The resulting type T (without the pointer).
 */
template<typename T>
struct remove_pointer<T*> {
    typedef T type;
};

/**
 * Meta-structure for removing references from a type.
 *
 * Base template. If T is not a reference type, the type remains unchanged.
 *
 * @tparam T 
 *      The type to process.
 *
 * @typedef type
 *      The resulting type (T if not a reference).
 *
 * @example
 *      remove_reference<int>::type    // int
 */
template<typename T>
struct remove_reference {
    typedef T type;
};

/**
 * Partial specialization of remove_reference for lvalue references.
 *
 * Strips the top-level lvalue reference qualifier from type T&.
 *
 * @tparam T 
 *      The referenced type.
 *
 * @typedef type
 *      The resulting type T (without the reference).
 *
 * @example
 *      remove_reference<int&>::type    // int
 */
template<typename T>
struct remove_reference<T&> {
    typedef T type;
};

/**
 * Partial specialization of remove_reference for rvalue references.
 *
 * Strips the top-level rvalue reference qualifier from type T&&.
 *
 * @tparam T 
 *      The referenced type.
 *
 * @typedef type
 *      The resulting type T (without the reference).
 *
 * @example
 *      remove_reference<int&&>::type    // int
 */
template<typename T>
struct remove_reference<T&&> {
    typedef T type;
};

}
namespace tc = jstd;

#endif /* DC7409F7_2255_4C74_B267_17B9E891E355 */
