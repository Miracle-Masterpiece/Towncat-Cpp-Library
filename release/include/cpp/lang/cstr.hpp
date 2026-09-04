#ifndef JSTD_CPP_LANG_CSTRING_H
#define JSTD_CPP_LANG_CSTRING_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/objects.hpp>

namespace tc
{

/**
 * Immutable wrapper for null-terminated C-style strings.
 * 
 * Provides a lightweight wrapper around const char* with cached length
 * and hash computation. The string data is not copied; the wrapper
 * maintains a pointer to the original C-string. Length is computed
 * lazily on first access and cached for subsequent calls.
 * 
 * Supports comparison, hashing, character access by index, and implicit
 * conversion to const char* for compatibility with C APIs.
 */
class c_str {
    const char* m_cstr;
    mutable std::size_t m_length;
    mutable std::size_t m_hashode;
public:
    /**
     * Constructs a c_str wrapper from a C-string.
     * 
     * @param s
     *      Pointer to a null-terminated C-string.
     *      Must remain valid for the lifetime of this object.
     *      If s is nullptr, the string is treated as empty.
     */
    c_str(const char* s);

    /**
     * Implicit conversion to const char*.
     * 
     * Allows using c_str objects where a C-string is expected,
     * such as passing to C standard library functions or APIs
     * that accept const char*.
     * 
     * @return
     *      Pointer to the underlying C-string.
     *      Returns nullptr if the underlying string was nullptr.
     */
    operator const char* () const;

    /**
     * Returns the length of the string.
     * 
     * The length is computed lazily on the first call and cached
     * for subsequent calls. The length does not include the null
     * terminator.
     * 
     * @return
     *      Number of characters in the string (0 if m_cstr is nullptr).
     */
    std::size_t length() const;

     /**
     * Accesses a character by index.
     * 
     * @param idx
     *      Index of the character to access.
     * 
     * @return
     *      Const reference to the character at the specified index.
     * 
     * @throws index_out_of_bound_exception (in DEBUG build)
     *      If idx is out of bounds.
     */
    const char& operator[] (std::size_t idx) const;

    /**
     * Compares two strings for equality.
     * 
     * Performs character-by-character comparison. Strings are equal
     * if they have the same length and all characters match.
     * 
     * @param s
     *      The other string to compare with.
     * 
     * @return
     *      True if the strings are identical, false otherwise.
     */
    bool equals(const c_str& s) const;

    /**
     * Equality comparison operator.
     * 
     * Delegates to equals().
     * 
     * @param s
     *      The other string to compare with.
     * 
     * @return
     *      True if the strings are equal, false otherwise.
     */
    bool operator==(const c_str& s) const;

    /**
     * Inequality comparison operator.
     * 
     * Returns the opposite of equals().
     * 
     * @param s
     *      The other string to compare with.
     * 
     * @return
     *      True if the strings are not equal, false otherwise.
     */
    bool operator!=(const c_str& s) const;

    /**
     * Computes the hash code of the string.
     * 
     * Calculates a hash value for the string using the hash_for<char>()
     * function. The result is cached for subsequent calls.
     * 
     * @return
     *      Hash code of the string (0 if the string is empty).
     */
    std::size_t hashcode() const;

    /**
     * Checks if the string is empty.
     * 
     * Returns true if the string contains no characters (length is 0).
     * This handles both empty strings and nullptr strings.
     * 
     * @return
     *      True if the string is empty, false otherwise.
     */
    bool is_empty() const;
};

template<>
struct hash_for<c_str> {
    std::size_t operator ()(const c_str& s) const {
        return s.hashcode();
    }
};

template<>
struct equal_to<c_str> {
    bool operator ()(const c_str& s0, const c_str& s1) const {
        return s0.equals(s1);
    }
};


}

#endif//JSTD_CPP_LANG_CSTRING_H