#ifndef A8229C78_54B7_47A2_A539_4D9F8ECA9E27
#define A8229C78_54B7_47A2_A539_4D9F8ECA9E27

#include <cpp/lang/traits/type_properties.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cstddef>

namespace tc
{

namespace internal
{
/**
 * Helper template for storing a single element with EBO support.
 * 
 * This template decides at compile-time whether to store the element
 * as a base class (if empty and non-final) or as a member.
 * 
 * @tparam E
 *      The element type.
 * 
 * @tparam INDEX
 *      Unique index to prevent duplicate base class conflicts.
 * 
 * @tparam empty
 *      Compile-time flag: true if E is empty and non-final.
 */
template<typename E, std::size_t INDEX, bool empty = (tc::is_empty<E>::value) && (!tc::is_final<E>::value)>
struct pair_element {};

/**
 * Specialization for empty, non-final types (EBO).
 * 
 * Stores the element as a private base class (zero overhead).
 * 
 * @tparam E
 *      The element type.
 * 
 * @tparam INDEX
 *      Unique index to prevent base class ambiguity.
 */
template<typename E, std::size_t INDEX>
struct pair_element<E, INDEX, true> : private E {
    /**
     * Returns a reference to the stored element.
     */
    E& get() & {return *this;}
    
    /**
     * Returns a const reference to the stored element.
     */
    const E& get() const& {return *this;}
    
    /**
     * Returns a const lvalue reference to the element.
     */
    E&& get() && {return std::move(*this);}
    
    /**
     * Returns a const rvalue reference to the element.
     */
    const E& get() const&& {return std::move(*this);}
};

/**
 * Two-element pair with EBO support.
 * 
 * A std::pair-like container that stores two values of types A and B.
 * Uses private inheritance for pair_elements to enable EBO.
 * 
 * @tparam A
 *      The type of the first element.
 * 
 * @tparam B
 *      The type of the second element.
 * 
 * @note
 *      Both elements are accessible via first() and second() methods.
 *      All accessor methods are ref-qualified for proper move semantics.
 */
template<typename E, std::size_t INDEX>
struct pair_element<E, INDEX, false> {
private:
    E e;
public:
    pair_element() : e() {}
    template<typename E_>
    pair_element(E_&& e) : e(std::forward<E_>(e)) {}
    E& get() & {return e;}
    E&& get() && {return std::move(e);}
    const E& get() const& {return e;}
    const E&& get() const&& {return std::move(e);}
};

} //namespace internal

/**
 * Two-element pair with EBO support.
 * 
 * A std::pair-like container that stores two values of types A and B.
 * Uses private inheritance for pair_elements to enable EBO.
 * 
 * @tparam A
 *      The type of the first element.
 * 
 * @tparam B
 *      The type of the second element.
 * 
 * @note
 *      Both elements are accessible via first() and second() methods.
 *      All accessor methods are ref-qualified for proper move semantics.
 */
template<typename A, typename B>
struct pair : private internal::pair_element<A, 0>, private internal::pair_element<B, 1> {
    using ABase = internal::pair_element<A, 0>;
    using BBase = internal::pair_element<B, 1>;
    
    /**
     * Default constructor.
     * 
     * Default-initializes both elements.
     */
    pair() : ABase(), BBase() {}
    
    /**
     * Constructs the pair from two values.
     * 
     * @param a
     *      Value for the first element.
     * 
     * @param b
     *      Value for the second element.
     */
    template<typename A_, typename B_>
    pair(A_&& a, B_&& b) : ABase(std::forward<A_>(a)), BBase(std::forward<B_>(b)) {}

    /**
     * Returns a const lvalue reference to the first element.
     */
    const A& first() const& {
        return ABase::get();
    }
    
    /**
     * Returns a const lvalue reference to the second element.
     */
    const B& second() const& {
        return BBase::get();
    }
    
    /**
     * Returns a const rvalue reference to the first element.
     */
    const A&& first() const&& {
        return std::move(ABase::get());
    }
    
    /**
     * Returns a const rvalue reference to the second element.
     */
    const B&& second() const&& {
        return std::move(BBase::get());
    }

    /**
     * Returns an lvalue reference to the first element.
     */
    A& first() & {
        return ABase::get();
    }
    
    /**
     * Returns an lvalue reference to the second element.
     */
    B& second() & {
        return BBase::get();
    }
    
    /**
     * Returns an rvalue reference to the first element.
     */
    A&& first() && {
        return std::move(ABase::get());
    }
    
    /**
     * Returns an rvalue reference to the second element.
     */
    B&& second() && {
        return std::move(BBase::get());
    }
};

    /**
     * Creates a pair with type deduction.
     * 
     * Convenience function that constructs a pair from two values
     * with automatic type deduction.
     * 
     * @tparam A
     *      The first element type (deduced).
     * 
     * @tparam B
     *      The second element type (deduced).
     * 
     * @param a
     *      Value for the first element.
     * 
     * @param b
     *      Value for the second element.
     * 
     * @return 
     *      A pair<A, B> containing the forwarded values.
     * 
     * @example
     *      auto p = make_pair(42, "hello");  // pair<int, const char*>
     *      auto p2 = make_pair(std::string("hello"), 3.14);  // pair<std::string, double>
     */
    template<typename A, typename B>
    pair<A, B> make_pair(A&& a, B&& b) {
        return pair<A, B>(std::forward<A>(a), std::forward<B>(b));
    }

    template<typename T, typename E>
    struct hash_for<pair<T, E>> {
        std::size_t operator() (const pair<T, E>& e) const {
            hash_for<T> khash;
            hash_for<E> vhash;
            return (khash(e.first()) * 17) ^ (vhash(e.second()) >> 4);
        }
    };

    template<typename T, typename E>
    struct equal_to<pair<T, E>> {
        std::size_t operator() (const pair<T, E>& a, const pair<T, E>& b) const {
            equal_to<T> kequal;
            equal_to<E> vequal;
            return kequal(a.first(), b.first()) && vequal(a.second(), b.second());
        }
    };

}
#endif /* A8229C78_54B7_47A2_A539_4D9F8ECA9E27 */
