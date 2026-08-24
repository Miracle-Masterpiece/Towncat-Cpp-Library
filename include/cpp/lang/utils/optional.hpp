#ifndef _JSTD_CPP_LANG_UTILS_OPTIONAL_H_
#define _JSTD_CPP_LANG_UTILS_OPTIONAL_H_

#include <new>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/traits/cv_traits.hpp>

namespace tc {

/**
 * Optional value container (nullable type).
 * 
 * A container that may or may not contain a value of type T.
 * Similar to std::optional from C++17, but with a different interface.
 * 
 * Key features:
 * - Can be empty (nullopt) or contain a value
 * - Value is stored in-place (no heap allocation)
 * - Supports move semantics
 * - Provides checked access (throws on empty)
 * - Null-optional construction via null_opt()
 * 
 * @tparam T
 *      The type of the stored value.
 * 
 * @note
 *      The optional uses a union for storage, so T must be
 *      trivially destructible or properly managed.
 * 
 * @example
 *      optional<int> opt;
 *      if (!opt.is_value()) {
 *          // empty
 *      }
 *      opt = 42;
 *      int value = opt.get_value();  // 42
 */
template<typename T>
class optional {
    
    union
    {
        T obj;
    };
    
    bool _is_value;
    
    /**
     * Destroys the stored value if present.
     * 
     * Calls the destructor of T if the optional contains a value.
     * Safe to call even on empty optional.
     */
    void dispose();
    
    /**
     * Checks if the optional contains a value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     * 
     * @note
     *      Called by accessors to ensure valid state.
     */
    void check_access() const {
        if (!_is_value)
            throw_except<illegal_state_exception>("Optional is empty!");
    }

public:
    
    /**
     * Default constructor.
     * 
     * Constructs an empty optional (no value).
     */
    optional();
    
    /**
     * Constructs from a const lvalue reference.
     * 
     * @param value
     *      The value to store (copied).
     */
    optional(const T& value);
    
    /**
     * Constructs from an rvalue reference.
     * 
     * @param value
     *      The value to store (moved).
     */
    optional(T&& value);
    
    /**
     * Copy constructor.
     * 
     * @param other
     *      The optional to copy from.
     */
    optional(const optional<T>& other);
    
    /**
     * Move constructor.
     * 
     * @param other
     *      The optional to move from.
     */
    optional(optional<T>&& other);
    
    /**
     * Copy assignment operator.
     * 
     * @param other
     *      The optional to copy from.
     */
    optional<T>& operator= (const optional<T>& other);
    
    /**
     * Move assignment operator.
     * 
     * @param other
     *      The optional to move from.
     */
    optional<T>& operator= (optional<T>&& other);
    
    /**
     * Destructor.
     * 
     * Destroys the stored value if present.
     */
    ~optional();
    
    /**
     * Checks if the optional contains a value.
     * 
     * @return
     *      true if contains a value, false if empty.
     */
    bool is_value() const;
    
    /**
     * Returns an lvalue reference to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     */
    T& get_value() &;
    
    /**
     * Returns a const lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if the optional is empty.
     */
    const T& get_value() const&;
    
    /**
     * Returns an rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if the optional is empty.
     */
    T&& get_value() &&;
    
    /**
     * Returns a const rvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if the optional is empty.
     */
    const T&& get_value() const&&;
    
    /**
     * Pointer-like access to the stored value.
     * 
     * @return
     *      Pointer to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     * 
     * @note
     *      Allows usage like opt->method().
     */
    T* operator->();
    
    /**
     * Const pointer-like access to the stored value.
     * 
     * @return
     *      Const pointer to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     */
    const T* operator->() const;
    
    /**
     * Dereference access to the stored value.
     * 
     * @return
     *      Lvalue reference to the stored value.
     * 
     * @throws
     *      illegal_state_exception if the optional is empty.
     */
    T& operator*() &;

    /**
     * Const dereference access to the stored value.
     * 
     * @return
     *      Const lvalue reference to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     */
    const T& operator*() const&;
    
    /**
     * Dereference access to the stored value (move).
     * 
     * @return
     *      Rvalue reference to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     */
    T&& operator*() &&;
    
    /**
     * Const dereference access to the stored value (move).
     * 
     * @return
     *      Const rvalue reference to the stored value.
     * 
     * @throws illegal_state_exception
     *      if the optional is empty.
     */
    const T&& operator*() const&&;

    /**
     * Creates an empty optional.
     * 
     * @return
     *      An empty optional.
     * 
     * @note
     *      Equivalent to optional<T>().
     */
    static optional<T> null_opt() {
        return optional<T>();
    }
};

    template<typename T>
    optional<T>::optional() : _is_value(false) {}

    template<typename T>
    optional<T>::optional(const T& value) : _is_value(false) {
        new(const_cast<typename remove_cv<T>::type*>(&obj)) T(value);
        _is_value = true;
    }

    template<typename T>
    optional<T>::optional(T&& value) : _is_value(false) {
        new(const_cast<typename remove_cv<T>::type*>(&obj)) T(std::move(value));
        _is_value = true;
    }

    template<typename T>
    optional<T>::optional(const optional<T>& t) : _is_value(false) {
        if (t._is_value) {
            new(const_cast<typename remove_cv<T>::type*>(&obj)) T(*reinterpret_cast<const T*>(t.obj));
            _is_value = true;
        }
    }
    
    template<typename T>
    optional<T>::optional(optional<T>&& t) : _is_value(false) {
        if (t._is_value)
        {
            new(const_cast<typename remove_cv<T>::type*>(&obj)) T(std::move(obj));
            _is_value = true;
        }
    }

    template<typename T>
    void optional<T>::dispose() {
        if (_is_value) {
            obj.~T();
            _is_value = false;
        }
    }

    template<typename T>
    optional<T>::~optional() {
        dispose();
    }
    
    template<typename T>
    bool optional<T>::is_value() const {
        return _is_value;
    }

    template<typename T>
    T& optional<T>::get_value() & {
        check_access();
        return obj;
    }
    
    template<typename T>
    const T& optional<T>::get_value() const& {
        check_access();
        return obj;
    }
    
    template<typename T>
    T&& optional<T>::get_value() && {
        check_access();
        return std::move(obj);
    }
    
    template<typename T>
    const T&& optional<T>::get_value() const&& {
        check_access();
        return std::move(obj);
    }

    template<typename T>
    T* optional<T>::operator->() {
        check_access();
        return &obj;
    }
    
    template<typename T>
    const T* optional<T>::operator->() const {
        check_access();
        return &obj;
    }
    
    template<typename T>
    T& optional<T>::operator*() & {
        return get_value();
    }
    
    template<typename T>
    const T& optional<T>::operator*() const& {
        return get_value();
    }

    template<typename T>
    T&& optional<T>::operator*() && {
        return get_value();
    }
    
    template<typename T>
    const T&& optional<T>::operator*() const&& {
        return get_value();
    }

}
#endif//_JSTD_CPP_LANG_UTILS_OPTIONAL_H_