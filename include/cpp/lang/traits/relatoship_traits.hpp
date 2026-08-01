#ifndef A28F06C4_5886_42EA_9E1F_2F6F55AEB2BC
#define A28F06C4_5886_42EA_9E1F_2F6F55AEB2BC

#include <cpp/lang/traits/cv_traits.hpp>

namespace tc
{
/**
 * Meta-function for checking type equivalence.
 * 
 * is_same<T, D> determines whether two types T and D are the same type.
 * 
 * If T and D are the same type, value is true; otherwise, value is false.
 * 
 * @tparam T 
 *      The first type to compare.
 * 
 * @tparam D 
 *      The second type to compare.
 * 
 * @example
 * is_same<int, int>::value      // true
 * is_same<int, const int>::value // false
 */
template<typename T, typename D>
struct is_same {
    /**
     * Значение, указывающее, являются ли T и D одним и тем же типом.
     * 
     * По умолчанию устанавливается в false, если T и D различны.
     */
    static const bool value = false;
};

/**
 * Partial specialization of is_same for identical types.
 * 
 * This specialization sets value to true when both template parameters
 * refer to the same type.
 * 
 * @tparam T 
 *      The type being compared with itself.
 */
template<typename T>
struct is_same<T, T> {
    /**
     * Значение true, указывающее, что T и T — это один и тот же тип.
     */
    static const bool value = true;
};

/**
 * Structure that checks whether type DERIVED_T is derived from type BASE_T.
 * 
 * Determines if DERIVED_T is a class derived from BASE_T, or if both types
 * are the same (depending on your implementation).
 * 
 * @tparam DERIVED_T
 *      The type being checked for derivation from BASE_T.
 * 
 * @tparam BASE_T
 *      The base type.
 * 
 * @note This trait requires both types to be complete types.
 */
template<typename DERIVED_T, typename BASE_T>
struct is_base_of {
private:
    struct dummy{
        char ignored[sizeof(void*) + 1];
    };
    static dummy test(const BASE_T* v);
    static char test(const void* v);
public:
    static const bool value = sizeof(test((DERIVED_T*) 1)) == sizeof(dummy);
};

/**
 * Checks whether two classes are in the same inheritance hierarchy.
 *
 * This trait returns true if one of the types is a base class
 * of the other, and false otherwise.
 * 
 * Usage examples:
 *      is_related<Cat, Animal>::value  == true
 *      is_related<Animal, Cat>::value  == true
 *      is_related<Dog, Cat>::value     == false
 *
 * @tparam DERIVED_T
 *      The first type to check.
 * 
 * @tparam BASE_T
 *      The second type to check.
 * 
 * @return {@code true} if the classes are related by inheritance in either direction;
 *         {@code false} otherwise.
 */
template<typename DERIVED_T, typename BASE_T>
struct is_related {
    static const bool value = (is_base_of<DERIVED_T, BASE_T>::value) || is_base_of<BASE_T, DERIVED_T>::value;
};

}

#endif /* A28F06C4_5886_42EA_9E1F_2F6F55AEB2BC */
