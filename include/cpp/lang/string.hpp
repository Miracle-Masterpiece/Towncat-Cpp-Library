#ifndef A6379E4D_4085_4DC3_B5E7_3C4A783981AF
#define A6379E4D_4085_4DC3_B5E7_3C4A783981AF

#include <cpp/lang/utils/comparator.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/common.hpp>
#include <cpp/lang/math.hpp>
#include <utility>

namespace tc
{

/**
 * This class represents a template string.
 * The string is not constant, mutable, or thread-safe.
 *
 * The string is null-terminated, so it can be passed to C functions that require const char*.
 *
 * @example
 *      tc::string str = "Hello, world!";
 *      std::cout << str << "\n";
 */
template<typename TCHAR>
class tstring {
    /**
     * 
     */
    enum {
        INLINE_BUFFER_SIZE = 16 / sizeof(TCHAR)
    };

    /**
     * 
     */
    union
    {
        /**
         * Pointer to data on the heap.
         */
        TCHAR*  data;

        /**
         * Built-in buffer for short strings.
         */
        TCHAR   inline_data[INLINE_BUFFER_SIZE];
    };
    
    /**
     * The allocator that manages this row's memory.
     */
    tca::allocator* const allocator;
    
    /**
     * The capacity of this string.
     * The value up to which characters can be added to the string without allocating it.
     *
     * The capacity must never be less than INLINE_BUFFER_SIZE
     */
    std::size_t cap;
    
    /**
     * Current string length.
     * A value that specifies the actual number of characters in the string.
     */
    std::size_t size;

    /**
     * @return
     *      Returns the number of characters between the maximum capacity and the current size.
     */
    std::size_t rem() const {
        return cap - size;
    }

    /**
    * @return
    *       true if and only if the internal string buffer is used.
    *       otherwise, false.
    */
    bool is_inline_string() const {
        return cap  <= INLINE_BUFFER_SIZE;
    }

    /**
     * Increases the array size by the new length.
     *
     * In fact, one will be added to the new length to accommodate the null terminator.
     * If the actual size is less than or equal to INLINE_BUFFER_SIZE, the function does nothing.
     *
     * Strict exception guarantee.
     * If the function throws an exception, the string will remain unchanged.
     *
     * @throws out_of_memory_error
     *      If there is insufficient memory.
     */
    void ensure_cap(std::size_t new_size);

    /**
     * Checks if a string contains a substring.
     *
     * @param a
     *      Pointer to the string to search.
     *
     * @param b
     *      Pointer to the string to be checked.
     *
     * @param blen
     *      Length of the string to be checked.
     *
     * @return
     *      true if a match is found; otherwise, false.
     */
    static bool match(const TCHAR* a, const TCHAR* b, std::size_t blen) {
        for (std::size_t i = 0; i < blen; ++i)
            if (a[i] != b[i]) return false;
        return true;
    }

    /**
     * Normalizes the length of a string.
     *
     * If the 'len' parameter is tc::npos(),
     * then the function calls str_len to calculate its length; otherwise, it returns the passed length.
     *
     * @return
     *      The length of the input string.
     */
    static std::size_t normalize_length(const TCHAR* s, std::size_t len) {
        return len == npos() ? str_len(s) : len;
    }

public:
    /**
     * Returns the length of the c-string.
     * The passed string must be null-terminated.
     * Passing a non-null-terminated string will result in undefined behavior.
     *
     * @param s
     *      The string whose length to return.
     *
     * @return
     *      The length of the input string.
     */
    static std::size_t str_len(const TCHAR* s) {
        std::size_t len = 0;
        while(*s++) ++len;
        return len;
    }

    /**
     * Initializes a zero-length string with the passed allocator.
     *
     * @param allocator (Optional)
     *      The allocator that will manage this string's memory.
     *
     * @example
     *      tc::malloc_free_allocator alloc;
     *      tc::string s(&alloc);
     *
     *      s.append("Hello, world!"); //the 'alloc' allocator will be used to allocate memory
     *
     *      std::cout << s << "\n";
     */
    explicit tstring(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Initializes a string using a c-string.
     *
     * @param s
     *      A null-terminated string.
     *      Passing nullptr results in undefined behavior.
     *
     * @param allocator (Optional)
     *      The allocator that will manage this string's memory.
     *
     * @example
     *      tc::malloc_free_allocator alloc;
     *      tc::string s(&alloc);
     *
     *      s.append("Hello, world!"); //the 'alloc' allocator will be used to allocate memory
     *
     *      std::cout << s << "\n";
     */
    tstring(const TCHAR* s, tca::allocator* alloc = tca::get_default_allocator());
    
    /**
     * Copy constructor.
     *
     * Copies the value of the input string to this string.
     * The input string's allocator is used when copying.
     *
     * @param s
     *      The string whose values ​​will be copied.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     *
     * @example
     *      tc::string s0 = "C++ the best";
     *      tc::string s1 = s0;
     *
     *      std::cout << s0 << "\n";
     *      std::cout << s1 << "\n";
     *      std::cout << std::boolalpha << s1.equals(s0) << "\n";
     *
     */
    tstring(const tstring<TCHAR>& s);
    
    /**
     * Move constructor.
     *
     * Moves the input string's resources into this string.
     * A pointer to the allocated memory is passed to the new string.
     *
     * After moving, the input string will retain its allocator
     * and will be in a valid but unspecified state.
     *
     * After moving, this string will use the input string's allocator.
     *
     * @param s
     *      The string whose resources will be moved into this string.
     *
     * @example
     *      tc::string str = "Hello, World!!!";
     *      tc::string moved = std::move(str);
     *
     *      assert(moved.get_allocator() == str.get_allocator());
     *      assert(moved.length() == 15);
     */
    tstring(tstring<TCHAR>&& s);

    /**
     * Copy operator.
     *
     * Copies the value of the input string to this string.
     * The current string's allocator is not modified during copying.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string is left unchanged.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     *
     * @example
     *      tc::string s0 = "Hello, world";
     *
     *      tca::malloc_free_allocator alloc;
     *      tc::string s1(&alloc);
     *
     *      s1 = s0;
     *
     *      assert(s1.equals(s0));
     *      assert(s1.get_allocator() == &alloc);
     *      assert(s1.length() == s0.length());
     */
    tstring<TCHAR>& operator=(const tstring<TCHAR>& s);
    
    /**
     * Move operator.
     *
     * Moves the input string's resources into this string.
     * A pointer to the allocated memory is passed to the new string.
     *
     * During the move, this string's allocator is not changed.
     *
     * If this string's allocator is not equal to the input string's allocator, a copy is made.
     * Otherwise, a normal resource move occurs.
     *
     * After the move, the input string will retain its allocator
     * and will be in a valid, but unspecified, state.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will remain in its unmodified state.
     *
     * @param s
     *      The string whose resources will be moved into this one.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     *
     * @example
     *      tc::string str = "Hello, World!!!";
     *      tc::string moved = std::move(str); 
     * 
     *      assert(moved.get_allocator() == str.get_allocator()); 
     *      assert(moved.length() == 15); 
     */
    tstring<TCHAR>& operator=(tstring<TCHAR>&& s);
    
    /**
     * 
     */
    ~tstring();

    /**
    * Returns a pointer to a c-string.
    * The string is null-terminated.
    *
    * The function is guaranteed not to return nullptr, even if the string's length is 0.
    *
    * @return
    *       A pointer to the string.
    */
    TCHAR* c_str();
    
    /**
     * Returns a constant pointer to a c-string.
     * The string is null-terminated.
     *
     * The function is guaranteed not to return nullptr, even if the string's length is 0.
     *
     * @return
     *      A pointer to the string.
     */
    const TCHAR* c_str() const;

    /**
     * Reserves space for a string of length 'sz'.
     * The actual reserved size will be one larger to accommodate the null terminator.
     *
     * Strict exception guarantee.
     * If the function throws an exception, the string will remain unchanged.
     *
     * @param sz
     *      The size of the string for which space is being reserved.
     *
     * @param
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& reserve(std::size_t newcap); 
    
    /**
     * Replaces each substring of this string
     * that matches the 'target' argument with the string 'replacement'
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string is left unchanged.
     *
     * @param target
     *      Pointer to a character string whose character values ​​should be replaced.
     *
     * @param replacement
     *      Pointer to a character string replacing the 'target' string.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& replace_all(const TCHAR* target, const TCHAR* replacement);
    
    /**
     * Replaces each substring of this string
     * that matches the 'target' argument with the string 'replacement'
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string is left unchanged.
     *
     * @param target
     *      Pointer to the string whose character values ​​should be replaced.
     *
     * @param replacement
     *      Pointer to the string replacing the 'target' string.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& replace_all(const tstring<TCHAR>& target, const tstring<TCHAR>& replacement) {
        return replace_all(target.c_str(), replacement.c_str());
    }

    /**
     * Replaces the character sequence from 'start' to 'end' with the string
     * that matches the 'replacement' argument
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param start
     *      The position to start the replacement from.
     *
     * @param end
     *      The position to replace up to.
     *
     * @param replacement
     *      A pointer to the replacement C string.
     *
     * @return
     *      A reference to this string.
     *
     * @throws illegal_argument_exception (In DEBUG build)
     *      If start > length()
     *      If end > length()
     *      If end < start
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& replace(std::size_t start, std::size_t end, const TCHAR* replacement);
    
    /**
     * Replaces the character sequence from 'start' to 'end' with the string
     * that matches the 'replacement' argument
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param start
     *      The position to start the replacement from.
     *
     * @param end
     *      The position to replace up to.
     *
     * @param replacement
     *      A pointer to the replacement string.
     *
     * @return
     *      A reference to this string.
     *
     * @throws illegal_argument_exception (In DEBUG build)
     *      If start > length()
     *      If end > length()
     *      If end < start
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& replace(std::size_t start, std::size_t end, const tstring<TCHAR>& replacement) {
        return replace(start, end, replacement.c_str());
    }
    
    /**
     * Appends a C-string to this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param idx
     *      The string position where the input string should be added.
     *
     * @param s
     *      The C-string to append to this string.
     *
     * @param len (Optional)
     *      The size of the appended string.
     *      If the parameter has the default value,
     *      the string length will be calculated internally, but the string must be null-terminated.
     *
     * @return
     *      A reference to this string.
     *
     * @throws index_out_of_bound_exception
     *      If idx > length()
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& append(std::size_t idx, const TCHAR* s, std::size_t len = npos());

    /**
     * Appends a C-string to the end of this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param s
     *      The C-string to append to this string.
     *
     * @param len (Optional)
     *      The size of the appended string.
     *      If the parameter has the default value,
     *      the string length will be calculated internally, but the string must be null-terminated.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& append(const TCHAR* s, std::size_t len = npos()) {
        return append(size, s, len);
    }

    /**
     * Appends a string to the end of this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param s
     *      The string to append to this string.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& append(const tstring<TCHAR>& s) {
        return append(s.c_str(), s.length());
    }
    
    /**
     * Appends a character to the end of this string at the specified offset.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string is left unchanged.
     *
     * @param idx
     *      The position at which the character will be added.
     *
     * @param ch
     *      The character to append.
     *
     * @return
     *      A reference to this string.
     *
     * @throws index_out_of_bound_exception
     *      If idx > length()
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& append(std::size_t idx, const TCHAR ch) {
        return append(idx, &ch, 1);
    }

    /**
     * Appends a character to the end of this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param ch
     *      The character to append.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& append(const TCHAR ch) {
        return append(&ch, 1);
    }
    
    /**
    * Removes the substring from index 'start' to index 'end' (not inclusive)
    *
    * @param start
    *       Start index.
    *
    * @param end
    *       End index.
    *
    * @return
    *       A reference to this string.
    */
    tstring<TCHAR>& remove(std::size_t start, std::size_t end);

    /**
     * Appends a C-string to the end of this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param s
     *      The C-string to append to this string.
     *      Must be null-terminated.
     *
     * @return
     *      A reference to this string.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    tstring<TCHAR>& operator<< (const TCHAR* s) {
        return append(s);
    }
    
    /**
     * Appends a string to the end of this string.
     *
     * Strong exception guarantee.
     * If the function throws an exception, the string will be left unchanged.
     *
     * @param s
     *       The string to append to this string.
     *
     * @return
     *       A reference to this string.
     *
     * @throws out_of_memory_error
     *       If there is not enough memory.
     */
    tstring<TCHAR>& operator<< (const tstring<TCHAR>& s) {
        return append(s.c_str(), s.length());
    }

    /**
     * Returns a reference to the character at the input index.
     *
     * @param
     *      The index of the character to return.
     *
     * @return
     *      A reference to the character at the index.
     *
     * @throws index_out_of_bound_exception
     *      If index >= length()
     */
    TCHAR& operator[] (std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, length()));
        return c_str()[idx];
    }
    
    /**
     * Returns a constant reference to the character at the input index.
     *
     * @param
     *      The index of the character to return.
     *
     * @return
     *      A constant reference to the character at the index.
     *
     * @throws index_out_of_bound_exception
     *      If index >= length()
     */
    const TCHAR& operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE(check_index(idx, length()));
        return c_str()[idx];
    }

    /**
     * Returns the length of the string.
     *
     * @return
     *      Length of the string
     */
    std::size_t length() const {
        return size;
    }
    
    /**
     * Sets the string to a new length.
     *
     * If the new length is less than the old length, simply sets the null terminator to the new end of the string.
     *
     * If the new length is greater than the capacity, reallocates memory.
     * The actual reallocated memory will be one more to accommodate the null terminator.
     *
     * If the new length is greater than the old length, sets the input character between the old and new end of the string.
     *
     * Strict exception guarantee.
     * If the function throws an exception, the string will remain unchanged.
     *
     * @param newlen
     *      The new length of the string.
     *
     * @param ch
     *      The character to fill between the old length and the new length.
     *
     * @throws out_of_memory_error
     *      If there is not enough memory.
     */
    void set_length(std::size_t newlen, const TCHAR& ch = 0);

    /**
     * Returns the capacity of the internal buffer.
     *
     * @return
     *       Buffer capacity.
     */
    std::size_t capacity() const {
        return cap;
    }

    /**
     * @return
     *      The allocator that manages this row's memory.
     */
    tca::allocator* get_allocator() const {
        return allocator;
    }

    /**
     * Returns the character at the input index.
     *
     * @param
     *      The index of the character to return.
     *
     * @return
     *      The character at the index.
     *
     * @throws index_out_of_bound_exception
     *      If index >= length()
     */
    TCHAR char_at(std::size_t idx) const {
        JSTD_DEBUG_CODE(check_index(idx, size));
        return c_str()[idx];
    }
    
    /**
     * Returns the index of the first occurrence of the input string.
     *
     * @param s
     *      The substring to search for.
     *
     * @param from_index
     *      The index at which to start the search.
     *
     * @param len
     *      The length of the input string.
     *
     * @return
     *      The index of the input string, or tc::npos() if the substring is not found.
     *
     * @throws illegal_argument_exception
     *      If from_index >= length()
     */
    std::size_t index_of(const TCHAR* s, std::size_t from_index = 0, std::size_t len = npos()) const;
    
    /**
     * Returns the index of the last occurrence of the input string.
     *
     * @param s
     *      The substring to search for.
     *
     * @param from_index
     *      The index at which to start the search.
     *
     * @param len (Optional)
     *      The length of the input string.
     *      If length is not specified, the length of the string will be calculated internally.
     *      In this case, the string must be null-terminated.
     *
     * @return
     *      The index of the input string, or tc::npos() if the substring is not found.
     *
     * @throws illegal_argument_exception
     *      If from_index >= length()
     */
    std::size_t last_index_of(const TCHAR* s, std::size_t len = npos()) const;

    /**
     * Checks whether the input string is within this string.
     *
     * @param s
     *      The string to check.
     *
     * @param len (Optional)
     *      The length of the input string.
     *      If length is not specified, the length of the string will be calculated internally.
     *      In this case, the string must be null-terminated.
     *
     * @return
     *      true if the string contains the substring; otherwise, false.
     */
    bool contains(const TCHAR* s, std::size_t len = npos()) const;

    /**
     * Checks whether the string at the specified index begins with the input substring.
     *
     * @param offset
     *      The offset to search.
     *
     * @param s
     *      The string to search.
     *
     * @param len
     *      The length of the input string.
     *
     * @return
     *      true if the substring begins with the input string; otherwise, false.
     */
    bool starts_with(std::size_t offset, const TCHAR* s, std::size_t len = npos()) const;
    
    /**
     * Checks whether the string starts with the input substring.
     *
     * @param s
     *      The string to search for.
     *
     * @param len
     *      The length of the input string.
     *
     * @return
     *      true if the substring starts with the input string; otherwise, false.
     */
    bool starts_with(const TCHAR* s, std::size_t len = npos()) const {
        return starts_with(0, s, len);
    }
    
    /**
     * Checks whether the string at the specified index begins with the input substring.
     *
     * @param offset
     *      The offset to search.
     *
     * @param s
     *      The string to search.
     *
     * @return
     *      true if the substring begins with the input string; otherwise, false.
     */
    bool starts_with(std::size_t offset, const tstring<TCHAR>& s) const {
        return starts_with(offset, s.c_str());
    }
    
    /**
     * Checks whether the string starts with the input substring.
     *
     * @param s
     *      The string to search for.
     *
     * @param len
     *      The length of the input string.
     *
     * @return
     *      true if the substring starts with the input string; otherwise, false.
     */
    bool starts_with(const tstring<TCHAR>& s) const {
        return starts_with(s.c_str(), s.length());
    }
    
    /**
     * Checks whether the string ends with the input string.
     *
     * @param s
     *      The input string to check.
     *
     * @param len
     *      The length of the input string.
     *
     * @return
     *      Whether this string ends with the input substring.
     */
    bool ends_with(const TCHAR* s, std::size_t len = npos()) const;
    
    /**
     * Checks whether the string ends with the input string.
     *
     * @param s
     *      The input string to check.
     *
     * @return
     *      Whether this string ends with the input substring.
     */
    bool ends_with(const tstring<TCHAR>& s) const {
        return ends_with(s.c_str(), s.length());
    }

    /**
     * Clears this string.
     * Sets the size to 0,
     * and sets the zero character at index 0.
     */
    void clear();

    /**
     * Whether the string is empty.
     * The string is empty if and only if the size() function returns 0.
     */
    bool is_empty() const;

    /**
     * Checks if this string is equal to the input string.
     *
     * @param s
     *      The string to check.
     *
     * @return
     *      true if the strings are equal, false otherwise.
     */
    bool equals(const tstring<TCHAR>& s) const;
    
    /**
     * Checks if this string is equal to the input string.
     *
     * @param s
     *      The string to check.
     *
     * @return
     *      true if the strings are equal, false otherwise.
     */
    bool equals(const TCHAR* s) const;
    
    /**
     * Checks if this string is equal to the input string.
     *
     * @param s
     *      The string to check.
     *
     * @return
     *      true if the strings are equal, false otherwise.
     */
    bool operator==(const tstring<TCHAR>& s) const {
        return equals(s);
    }
    
    /**
     * Checks if this string is equal to the input string.
     *
     * @param s
     *      The string to check.
     *
     * @return
     *      true if the strings are equal, false otherwise.
     */
    bool operator==(const TCHAR* s) const {
        return equals(s);
    }

    /**
     * Returns the hash code of the string.
     *
     * @return
     *       The hash code of this string.
     */
    std::size_t hashcode() const;

    /**
     * Compares this string to the input string.
     *
     * @return
     *      -1 - if this string is less than the input string.
     *       0 - if this string is equal to the input string.
     *       1 - if this string is greater than the input string.
     */
    int compare_to(const tstring<TCHAR>& r);

    /**
     * Removes all leading and trailing characters less than or equal to 0x20
     *
     * @return
     *      A reference to this string.
     */
    tstring<TCHAR>& trim();

    /**
     * 
     */
    tstring<TCHAR> sub_string(std::size_t start, std::size_t end, tca::allocator* allocator = tca::get_default_allocator()) const;

    /**
     * 
     */
    TCHAR* begin();
    
    /**
     * 
     */
    TCHAR* end();
    
    /**
     * 
     */
    const TCHAR* begin() const;
    
    /**
     * 
     */
    const TCHAR* end() const;
};

namespace internal
{
    
    template<typename E>
    tstring<char> to_string0(const E& e, const char* ext, tca::allocator* allocator);

    template<typename E>
    tstring<wchar_t> to_wstring0(const E& e, const wchar_t* ext, tca::allocator* allocator);
}

    template<typename E>
    tstring<char> to_string(const E& e, tca::allocator* allocator = tca::get_default_allocator());

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    inline tstring<char> to_string<char>(const char& e, tca::allocator* allocator) {
        return internal::to_string0<char>(e, "%c", allocator);
    }
    
    template<>
    inline tstring<char> to_string<unsigned char>(const unsigned char& e, tca::allocator* allocator) {
        return internal::to_string0<unsigned char>(e, "%c", allocator);
    }
    
    template<>
    inline tstring<char> to_string<signed char>(const signed char& e, tca::allocator* allocator) {
        return internal::to_string0<signed char>(e, "%c", allocator);
    }

    template<>
    inline tstring<char> to_string<int>(const int& e, tca::allocator* allocator) {
        return internal::to_string0<int>(e, "%i", allocator);
    }
    
    template<>
    inline tstring<char> to_string<unsigned int>(const unsigned int& e, tca::allocator* allocator) {
        return internal::to_string0<unsigned int>(e, "%u", allocator);
    }
    
    template<>
    inline tstring<char> to_string<short>(const short& e, tca::allocator* allocator) {
        return internal::to_string0<int>(e, "%i", allocator);
    }
    
    template<>
    inline tstring<char> to_string<unsigned short>(const unsigned short& e, tca::allocator* allocator) {
        return internal::to_string0<unsigned int>(e, "%u", allocator);
    }

    template<>
    inline tstring<char> to_string<long>(const long& e, tca::allocator* allocator) {
        return internal::to_string0<long>(e, "%li", allocator);
    }
    
    template<>
    inline tstring<char> to_string<unsigned long>(const unsigned long& e, tca::allocator* allocator) {
        return internal::to_string0<unsigned long>(e, "%lu", allocator);
    }

    template<>
    inline tstring<char> to_string<long long>(const long long& e, tca::allocator* allocator) {
        return internal::to_string0<long long>(e, "%lli", allocator);
    }
    
    template<>
    inline tstring<char> to_string<unsigned long long>(const unsigned long long& e, tca::allocator* allocator) {
        return internal::to_string0<unsigned long long>(e, "%llu", allocator);
    }

    template<>
    inline tstring<char> to_string<float>(const float& e, tca::allocator* allocator) {
        return internal::to_string0<float>(e, "%g", allocator);
    }
    
    template<>
    inline tstring<char> to_string<double>(const double& e, tca::allocator* allocator) {
        return internal::to_string0<double>(e, "%g", allocator);
    }
    
    template<typename T>
    inline tstring<char> to_string(T* e, tca::allocator* allocator = tca::get_default_allocator()) {
        return internal::to_string0<const void*>(e, "%p", allocator);
    }
    
    template<>
    inline tstring<char> to_string<char>(char* e, tca::allocator* allocator) {
        return tstring<char>(e, allocator);
    }
    
    template<>
    inline tstring<char> to_string<const char>(const char* e, tca::allocator* allocator) {
        return tstring<char>(e, allocator);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename E>
    inline tstring<wchar_t> to_wstring(const E& e, tca::allocator* allocator = tca::get_default_allocator());

    template<>
    inline tstring<wchar_t> to_wstring<wchar_t>(const wchar_t& e, tca::allocator* allocator) {
        return internal::to_wstring0<wchar_t>(e, L"%lc", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<unsigned char>(const unsigned char& e, tca::allocator* allocator) {
        return internal::to_wstring0<unsigned char>(e, L"%c", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<signed char>(const signed char& e, tca::allocator* allocator) {
        return internal::to_wstring0<signed char>(e, L"%c", allocator);
    }

    template<>
    inline tstring<wchar_t> to_wstring<int>(const int& e, tca::allocator* allocator) {
        return internal::to_wstring0<int>(e, L"%i", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<unsigned int>(const unsigned int& e, tca::allocator* allocator) {
        return internal::to_wstring0<unsigned int>(e, L"%u", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<short>(const short& e, tca::allocator* allocator) {
        return internal::to_wstring0<int>(e, L"%i", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<unsigned short>(const unsigned short& e, tca::allocator* allocator) {
        return internal::to_wstring0<unsigned int>(e, L"%u", allocator);
    }

    template<>
    inline tstring<wchar_t> to_wstring<long>(const long& e, tca::allocator* allocator) {
        return internal::to_wstring0<long>(e, L"%li", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<unsigned long>(const unsigned long& e, tca::allocator* allocator) {
        return internal::to_wstring0<unsigned long>(e, L"%lu", allocator);
    }

    template<>
    inline tstring<wchar_t> to_wstring<long long>(const long long& e, tca::allocator* allocator) {
        return internal::to_wstring0<long long>(e, L"%lli", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<unsigned long long>(const unsigned long long& e, tca::allocator* allocator) {
        return internal::to_wstring0<unsigned long long>(e, L"%llu", allocator);
    }

    template<>
    inline tstring<wchar_t> to_wstring<float>(const float& e, tca::allocator* allocator) {
        return internal::to_wstring0<float>(e, L"%g", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<double>(const double& e, tca::allocator* allocator) {
        return internal::to_wstring0<double>(e, L"%g", allocator);
    }
    
    template<typename T>
    inline tstring<wchar_t> to_wstring(T* e, tca::allocator* allocator = tca::get_default_allocator()) {
        return internal::to_string0<const void*>(e, L"%p", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<wchar_t>(wchar_t* e, tca::allocator* allocator) {
        return internal::to_wstring0<const wchar_t*>(e, L"%s", allocator);
    }
    
    template<>
    inline tstring<wchar_t> to_wstring<const wchar_t>(const wchar_t* e, tca::allocator* allocator) {
        return internal::to_wstring0<const wchar_t*>(e, L"%s", allocator);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
using u8string  = tstring<char>;
using u16string = tstring<unsigned short>;
using u32string = tstring<unsigned long>;
using wstring   = tstring<wchar_t>;
using string    = u8string;

template<typename T>
struct hash_for<tstring<T>> {
    std::size_t operator() (const tstring<T>& s) const {
        return s.hashcode();
    }
};

template<typename T>
struct equal_to<tstring<T>> {
    bool operator() (const tstring<T>& a, const tstring<T>& b) const {
        return a.equals(b);
    }
};

template<typename T>
struct compare_to<tstring<T>> {
    int operator() (const tstring<T>& a, const tstring<T>& b) const {
        return a.compare_to(b);
    }
};

} //namespace tc

#include <iosfwd>

inline std::ostream& operator<<(std::ostream& out, const tc::string& s) {
    out << static_cast<const char*>(s.c_str());
    return out;
}

inline std::wostream& operator<<(std::wostream& out, const tc::wstring& s) {
    out << static_cast<const wchar_t*>(s.c_str());
    return out;
}

#endif /* A6379E4D_4085_4DC3_B5E7_3C4A783981AF */
