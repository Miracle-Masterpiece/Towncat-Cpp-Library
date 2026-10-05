#ifndef A647B5FB_A481_4E2B_AE2B_E5B3373A98C3
#define A647B5FB_A481_4E2B_AE2B_E5B3373A98C3

namespace tc
{

/**
 * Meta-structure for checking whether a type is const-qualified.
 *
 * Base template. Sets value to false for non-const types.
 *
 * @tparam T 
 *      The type to check.
 *
 * @var value
 *      true if T is const-qualified, false otherwise.
 */
template<typename T>
struct is_const {
    static const bool value = false;
};

/**
 * Specialization of is_const for const-qualified types.
 *
 * Sets value to true for const-qualified types.
 *
 * @tparam T 
 *      The underlying type.
 */
template<typename T>
struct is_const<const T> {
    static const bool value = true;
};

/**
 * Meta-structure for checking whether a type is volatile-qualified.
 *
 * Base template. Sets value to false for non-volatile types.
 *
 * @tparam T 
 *      The type to check.
 *
 * @var value
 *      true if T is volatile-qualified, false otherwise.
 */
template<typename T>
struct is_volatile {
    static const bool value = false;
};

/**
 * Specialization of is_volatile for volatile-qualified types.
 *
 * Sets value to true for volatile-qualified types.
 *
 * @tparam T 
 *      The underlying type.
 */
template<typename T>
struct is_volatile<volatile T> {
    static const bool value = true;
};

/**
 * Meta-structure for checking whether FROM_T can be cast to TO_T
 * with respect to cv-qualifiers (const and volatile).
 *
 * Determines if a cv-qualified type FROM_T can be safely converted
 * to a cv-qualified type TO_T. The conversion is valid if:
 * - const can be added but not removed: !is_const<FROM_T> || is_const<TO_T>
 * - volatile can be added but not removed: !is_volatile<FROM_T> || is_volatile<TO_T>
 *
 * In other words, casting FROM_T to TO_T is allowed if TO_T has
 * the same or more cv-qualifiers than FROM_T.
 *
 * @tparam FROM_T
 *      The source type.
 *
 * @tparam TO_T
 *      The target type.
 *
 * @var value
 *      true if the cv-qualifier conversion is valid, false otherwise.
 *
 * @example
 * is_cv_castable<int, const int>::value       // true  (adding const)
 * is_cv_castable<const int, int>::value       // false (removing const)
 * is_cv_castable<int, volatile int>::value    // true  (adding volatile)
 * is_cv_castable<volatile int, int>::value    // false (removing volatile)
 * is_cv_castable<const int, const volatile int>::value // true (adding volatile)
 */
template<typename FROM_T, typename TO_T>
struct is_cv_castable {
    static const bool value =   (!is_const<FROM_T>::value || is_const<TO_T>::value) &&      //const check
                                (!is_volatile<FROM_T>::value || is_volatile<TO_T>::value);  //volatile check
};

/**
 * Meta-structure for removing const and volatile qualifiers from the top level of a type.
 *
 * The remove_cv structure provides a type alias 'type' in which the
 * top-level const and volatile qualifiers have been removed from type T.
 * 
 * Nested qualifiers, such as those in pointers (const int*) or references (const int&),
 * remain unchanged.
 *
 * This implementation is equivalent to std::remove_cv from the C++ standard library.
 *
 * @tparam T 
 *      The type from which top-level cv-qualifiers are to be removed.
 *
 * @note 
 *      Pointers and references are not modified; only the type T itself is affected.
 *
 * Usage examples:
 * @example
 *      remove_cv<const int>::type           // int
 *      remove_cv<int>::type                 // int
 *      remove_cv<const int*>::type          // const int*
 *      remove_cv<int* const>::type          // int*
 *      remove_cv<const int&>::type          // const int& (unchanged)
 *      remove_cv<volatile int>::type        // int
 *      remove_cv<const volatile int>::type  // int
 */
template<typename T>
struct remove_cv {
    typedef T type;
};

/**
 * Partial specialization of remove_cv for const-qualified types.
 *
 * Removes the top-level const qualifier, leaving nested const qualifiers unchanged.
 *
 * @tparam T 
 *      The underlying type wrapped in const.
 */
template<typename T>
struct remove_cv<const T> {
    typedef T type;
};

/**
 * Partial specialization of remove_cv for volatile-qualified types.
 *
 * Removes the top-level volatile qualifier, leaving nested volatile qualifiers unchanged.
 *
 * @tparam T 
 *      The underlying type wrapped in volatile.
 */
template<typename T>
struct remove_cv<volatile T> {
    typedef T type;
};

/**
 * Adds a const qualifier to type T.
 *
 * Template structure equivalent to std::add_const from the C++ standard library.
 * Adds top-level const to type T without modifying pointers or references.
 *
 * @tparam T 
 *      The type to which const should be added.
 *
 * @typedef type
 *      The resulting type with const added.
 *
 * Usage examples:
 * @example
 *      add_const<int>::type           // const int
 *      add_const<int*>::type          // int* const
 *      add_const<const int*>::type    // const int* const
 *      add_const<int&>::type          // int& (unchanged)
 */
template<typename T>
struct add_const {
    typedef const T type;
};

/**
 * Adds a volatile qualifier to type T.
 *
 * Template structure equivalent to std::add_volatile from the C++ standard library.
 * Adds top-level volatile to type T without modifying pointers or references.
 *
 * @tparam T 
 *      The type to which volatile should be added.
 *
 * @typedef type
 *      The resulting type with volatile added.
 *
 * Usage examples:
 * @example
 *      add_volatile<int>::type           // volatile int
 *      add_volatile<int*>::type          // int* volatile
 *      add_volatile<volatile int*>::type // volatile int* volatile
 *      add_volatile<int&>::type          // int& (unchanged)
 */
template<typename T>
struct add_volatile {
    typedef volatile T type;
};

}


#endif /* A647B5FB_A481_4E2B_AE2B_E5B3373A98C3 */
