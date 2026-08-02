#ifndef A10D59B8_10B4_4E4E_9BB5_F8A243A463DB
#define A10D59B8_10B4_4E4E_9BB5_F8A243A463DB

#include <utility>
#include <cpp/lang/traits/SFINAE.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/traits/pure_traits.hpp>
#include <cpp/lang/traits/pointer_traits.hpp>
#include <cpp/lang/exceptions.hpp>
#include <new>

namespace tc
{

/**
 * Wrapper for an error value used to construct expected in error state.
 * 
 * @tparam E The error type.
 * 
 * @example
 *      auto err = unexpected(42);
 *      expected<int, int> e = err;
 */
template<typename E>
class unexpect {
    E err;
public:
    /**
     * Constructs from a forwarding reference to error value.
     * 
     * @tparam E_RV The error type (deduced).
     * 
     * @param e
     *      The error value to store.
     * 
     * @note
     *      Enabled only if E and E_RV are the same type (after cleaning).
     */    
    template<typename E_RV, typename = 
                                        typename enable_if<
                                                    is_same<
                                                            typename pure_type<E>::type, typename pure_type<E_RV>::type
                                                           >::value 
                                                 >::type>
    unexpect(E_RV&& e);
    
    /**
     * Returns lvalue reference to the stored error.
     */
    E& error() & {
        return err;
    }
    
    /**
     * Returns const lvalue reference to the stored error.
     */
    const E& error() const& {
        return err;
    }
    
    /**
     * Returns rvalue reference to the stored error.
     */
    E&& error() && {
        return std::move(err);
    }
    
    /**
     * Returns const rvalue reference to the stored error.
     */
    const E&& error() const&& {
        return std::move(err);
    }
    
    /**
     * Default constructor. Value-initializes the error.
     */
    unexpect();
};

    template<typename E>
    template<typename E_RV, typename>
    unexpect<E>::unexpect(E_RV&& e) : err(std::forward<E_RV>(e)) {
        
    }
    
    template<typename E>
    unexpect<E>::unexpect() {

    }
 
    /**
     * Creates an unexpect object with error value e.
     * 
     * @tparam E The error type (deduced).
     * 
     * @param e
     *      The error value.
     * 
     * @return
     *      unexpect with cv-qualifiers and references removed.
     * 
     * @example
     *      expected<int, std::string> e = unexpected("something went wrong");
     */
    template<typename E>
    unexpect<typename remove_cv<typename remove_reference<E>::type>::type> unexpected(E&& e) {
        return unexpect<typename remove_cv<typename remove_reference<E>::type>::type>(std::forward<E>(e));
    }

/**
 * Represents either a value of type T or an error of type E.
 * 
 * This type is similar to std::expected (C++23) or Rust's Result.
 * Contains exactly one of: a value (T) or an error (E).
 * 
 * @tparam T Value type.
 * @tparam E Error type.
 * 
 * @example
 *      tc::expected<int, tc::error_code> divide(int a, int b) {
 *          if (b == 0)
 *          {
 *              return tc::unexpected(tc::error_code(EINVAL, tc::generic_category()));
 *          }
 *          return a / b;
 *      }
 * 
 *      auto result = divide(10, 2);
 *      if (result.has_value()) {
 *          std::cout << *result;  // 5
 *      } else {
 *          std::cout << result.error().get_message();
 *      }
 */
template<typename T, typename E>
class expected {
    
    union {
        T val;
        E err;
    };
    
    bool is_val;

    /**
     * Checks if contains value. Throws if not.
     */
    void check_access_val() const {
        JSTD_DEBUG_CODE(
            if (!is_val)
                throw_except<illegal_state_exception>("expected not contain result");
        )
    }
    
    /**
     * Checks if contains error. Throws if not.
     */
    void check_access_err() const {
        JSTD_DEBUG_CODE(
            if (is_val)
                throw_except<illegal_state_exception>("expected not contain error");
        )
    }

    typedef typename remove_cv<T>::type non_const_t;
    typedef typename remove_cv<E>::type non_const_e;

public:
    
    /**
     * Default constructor. Value-initializes T.
     */
    expected();

    /**
     * Constructs from value.
     * 
     * @tparam T_RV The value type (deduced).
     * 
     * @param v
     *      The value to store.
     * 
     * @note
     *      Enabled only if T and T_RV are the same type (after cleaning).
     */
    template<typename T_RV, typename = typename enable_if<
                                                            is_same<
                                                                typename pure_type<T>::type, typename pure_type<T_RV>::type
                                                            >::value 
                                                    >::type>
    expected(T_RV&&);
    
    /**
     * Constructs from unexpect (error state).
     * 
     * @param u
     *      The unexpect object containing the error.
     */
    expected(const unexpect<E>&);
    
    /**
     * Constructs from unexpect (error state, move).
     * 
     * @param u
     *      The unexpect object containing the error.
     */
    expected(unexpect<E>&&);

    /**
     * Checks whether expected contains a value.
     * 
     * @return
     *      true if contains a value, false if contains an error.
     */
    bool has_value() const;

    /**
     * Returns lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    T& value() &;

    /**
     * Returns const lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    const T& value() const&;
    
    /**
     * Returns rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    T&& value() &&;
    
    /**
     * Returns const rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    const T&& value() const&&;

    /**
     * Returns lvalue reference to the stored error.
     * 
     * @throws
     *      illegal_state_exception if contains value.
     */
    E& error() &;

    /**
     * Returns const lvalue reference to the stored error.
     * 
     * @throws
     *      illegal_state_exception if contains value.
     */
    const E& error() const&;
    
    /**
     * Returns rvalue reference to the stored error.
     * 
     * @throws
     *      illegal_state_exception if contains value.
     */
    E&& error() &&;
    
    /**
     * Returns const rvalue reference to the stored error.
     * 
     * @throws
     *      illegal_state_exception if contains value.
     */
    const E&& error() const&&;

    /**
     * Pointer-like access to value.
     * 
     * @return
     *      Pointer to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    T* operator-> () {
        check_access_val();
        return &val;
    }
    
    /**
     * Const pointer-like access to value.
     * 
     * @return
     *      Const pointer to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    const T* operator-> () const {
        check_access_val();
        return &val;
    }

    /**
     * Dereference access to value.
     * 
     * @return
     *      Lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    T& operator* () & {
        check_access_val();
        return val;
    }
    
    /**
     * Const dereference access to value.
     * 
     * @return
     *      Const lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    const T& operator*() const& {
        check_access_val();
        return val;
    }
    
    /**
     * Dereference access to value (move).
     * 
     * @return
     *      Rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    T&& operator* () && {
        check_access_val();
        return std::move(val);
    }
    
    /**
     * Const dereference access to value (move).
     * 
     * @return
     *      Const rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if contains error.
     */
    const T&& operator*() const&& {
        check_access_val();
        return std::move(val);
    }
    
    /**
     * Checks whether expected contains a value.
     * 
     * @return
     *      true if contains a value, false if contains an error.
     */
    operator bool() const {
        return is_val;
    }
    
    /**
     * Copy constructor.
     * 
     * @param e
     *      The expected object to copy.
     */
    expected(const expected<T, E>& e);
    
    /**
     * Move constructor.
     * 
     * @param e
     *      The expected object to move.
     */
    expected(expected<T, E>&& e);
    
    /**
     * Copy assignment operator.
     * 
     * @param e
     *      The expected object to copy.
     * 
     * @return
     *      Reference to this.
     */
    expected<T, E>& operator= (const expected<T, E>& e);
    
    /**
     * Move assignment operator.
     * 
     * @param e
     *      The expected object to move.
     * 
     * @return
     *      Reference to this.
     */
    expected<T, E>& operator= (expected<T, E>&& e);
    
    /**
     * Destructor. Calls destructor of the active member.
     */
    ~expected();
};

    template<typename T, typename E>
    expected<T, E>::expected() : is_val(false) {
        new( static_cast<void*>(const_cast<non_const_t*>(&val)) ) T();
        is_val = true;
    }

    template<typename T, typename E>
    template<typename T_RV, typename>
    expected<T, E>::expected(T_RV&& v) {
        new ( static_cast<void*>(const_cast<non_const_t*>(&val)) ) T(std::forward<T_RV>(v));
        is_val = true;
    }
    
    template<typename T, typename E>
    expected<T, E>::expected(const unexpect<E>& u) {
        new ( static_cast<void*>(const_cast<non_const_e*>(&err)) ) E(u.error());
        is_val = false;
    }
    
    template<typename T, typename E>
    expected<T, E>::expected(unexpect<E>&& u) {
        new ( static_cast<void*>(const_cast<non_const_e*>(&err)) ) E(u.error());
        is_val = false;
    }

    template<typename T, typename E>
    bool expected<T, E>::has_value() const {
        return is_val;
    }
    
    template<typename T, typename E>
    T& expected<T, E>::value() & {
        check_access_val();
        return val;
    }
    
    template<typename T, typename E>
    const T& expected<T, E>::value() const& {
        check_access_val();
        return val;
    }
    
    template<typename T, typename E>
    T&& expected<T, E>::value() && {
        check_access_val();
        return std::move(val);
    }
    
    template<typename T, typename E>
    const T&& expected<T, E>::value() const && {
        check_access_val();
        return std::move(val);
    }

    template<typename T, typename E>
    E& expected<T, E>::error() & {
        check_access_err();
        return err;
    }
    
    template<typename T, typename E>
    const E& expected<T, E>::error() const& {
        check_access_err();
        return err;
    }
    
    template<typename T, typename E>
    E&& expected<T, E>::error() && {
        check_access_err();
        return std::move(err);
    }
    
    template<typename T, typename E>
    const E&& expected<T, E>::error() const&& {
        check_access_err();
        return std::move(err);
    }

    template<typename T, typename E>
    expected<T, E>::expected(const expected<T, E>& e) {
        if (e.has_value())
        {
            new(&val) T(e.value());
            is_val = true;
        }
        else
        {
            new(&val) T(e.value());
            is_val = true;
        }
    }

    template<typename T, typename E>
    expected<T, E>::expected(expected<T, E>&& e) : expected<T, E>() {
        if (e.has_value())  std::swap(val,      e.val);
        else                std::swap(err,      e.err);
        std::swap(is_val,   e.is_val);
    }
    
    template<typename T, typename E>
    expected<T, E>& expected<T, E>::operator= (const expected<T, E>& e) {
        if (&e == this)
            return *this;

        if (e.has_value())
        {
            non_const_t tmp = e.value();

            if (has_value())
                val.~T();
            else
                err.~E();   

            new( static_cast<void*>(const_cast<non_const_t*>(&val)) ) T(std::move(tmp));

            is_val = true;
        }
        else
        {
            
            non_const_e tmp = e.error();

            if (has_value())
                val.~T();
            else
                err.~E();
            
            new( static_cast<void*>(const_cast<non_const_e*>(&err)) ) E(std::move(tmp));   

            is_val = false;
        }

        return *this;
    }
    
    template<typename T, typename E>
    expected<T, E>& expected<T, E>::operator= (expected<T, E>&& e) {
        
        if (e.has_value())
        {
            if (has_value())
            {
                val = std::move(e.val);
            }
            else
            {
                err.~E();
                new( static_cast<void*>(const_cast<non_const_t*>(&val)) ) T(std::move(e.val));
                is_val = true;
            }
        }
        else
        {
            if (!has_value())
            {
                err = std::move(e.err);
            }
            else
            {
                val.~T();
                new( static_cast<void*>(const_cast<non_const_e*>(&err)) ) E(std::move(e.err));
                is_val = false;
            }
        }

        return *this;
    }

    template<typename T, typename E>
    expected<T, E>::~expected() {
        if (is_val)
        {
            val.~T();
        }
        else
        {
            err.~E();
        }
    }

}

#endif /* A10D59B8_10B4_4E4E_9BB5_F8A243A463DB */
