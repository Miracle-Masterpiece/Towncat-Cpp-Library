#ifndef A6379E4D_4085_4DC3_B5E7_3C4A783981AF
#define A6379E4D_4085_4DC3_B5E7_3C4A783981AF

#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/math.hpp>
#include <utility>

namespace tc
{

template<typename TCHAR>
class tstring {
    /**
     * 
     */
    enum {INLINE_BUFFER_SIZE = 16};

    /**
     * 
     */
    union {
        TCHAR*  data;
        TCHAR   inline_data[INLINE_BUFFER_SIZE];
    };
    
    tca::allocator* const allocator;
    std::size_t     cap;
    std::size_t     size;

    /**
     * 
     */
    std::size_t rem() const {
        return cap - size;
    }

    /**
     * 
     */
    bool is_inline_string() const {
        return cap  <= INLINE_BUFFER_SIZE;
    }

    /**
     * 
     */
    void ensure_cap(std::size_t new_size);

    /**
     * 
     */
    static bool match(const TCHAR* a, const TCHAR* b, std::size_t blen) {
        for (std::size_t i = 0; i < blen; ++i)
            if (a[i] != b[i]) return false;
        return true;
    }

    /**
     * 
     */
    static std::size_t normalize_length(const TCHAR* s, std::size_t len) {
        return len == npos() ? str_len(s) : len;
    }

public:

    /**
     * 
     */
    static std::size_t str_len(const TCHAR* s) {
        std::size_t len = 0;
        while(*s++) ++len;
        return len;
    }

    /**
     * 
     */
    static std::size_t npos() {
        return ~(std::size_t) 0;
    }

    /**
     * 
     */
    explicit tstring(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * 
     */
    tstring(const TCHAR* s, tca::allocator* alloc = tca::get_default_allocator());
    
    /**
     * 
     */
    tstring(const tstring<TCHAR>& s);
    
    /**
     * 
     */
    tstring(tstring<TCHAR>&& s);
    
    /**
     * 
     */
    tstring<TCHAR>& operator=(const tstring<TCHAR>& s);
    
    /**
     * 
     */
    tstring<TCHAR>& operator=(tstring<TCHAR>&& s);
    
    /**
     * 
     */
    ~tstring();

    /**
     * 
     */
    TCHAR* cstr();
    
    /**
     * 
     */
    const TCHAR* cstr() const;

    /**
     * 
     */
    tstring<TCHAR>& reserve(std::size_t newcap); 

    /**
     * 
     */
    tstring<TCHAR>& append(std::size_t idx, const TCHAR* s, std::size_t len = npos());
    
    /**
     * 
     */
    tstring<TCHAR>& replace_all(const TCHAR* matcher, const TCHAR* replacement);
    
    /**
     * 
     */
    tstring<TCHAR>& replace(std::size_t start, std::size_t end, const TCHAR* replacement);
    
    /**
     * 
     */
    tstring<TCHAR>& replace(std::size_t start, std::size_t end, const tstring<TCHAR>& replacement) {
        return replace(start, end, replacement.cstr());
    }
    
    /**
     * 
     */
    tstring<TCHAR>& replace_all(const tstring<TCHAR>& matcher, const tstring<TCHAR>& replacement) {
        return replace_all(matcher.cstr(), replacement.cstr());
    }

    /**
     * 
     */
    tstring<TCHAR>& append(const TCHAR* s, std::size_t len = npos()) {
        return append(size, s, len);
    }
    
    /**
     * 
     */
    tstring<TCHAR>& append(const TCHAR s) {
        return append(&s, 1);
    }
    
    /**
     * 
     */
    tstring<TCHAR>& append(std::size_t idx, const TCHAR s) {
        return append(idx, &s, 1);
    }

    /**
     * 
     */
    tstring<TCHAR>& remove(std::size_t start, std::size_t end);

    /**
     * 
     */
    tstring<TCHAR>& operator<< (const TCHAR* s) {
        return append(s);
    }
    
    /**
     * 
     */
    tstring<TCHAR>& operator<< (const tstring<TCHAR>& s) {
        return append(s.cstr());
    }

    /**
     * 
     */
    TCHAR& operator[] (std::size_t idx) {
        check_index(idx, length());
        return cstr()[idx];
    }
    
    /**
     * 
     */
    const TCHAR& operator[] (std::size_t idx) const {
        check_index(idx, length());
        return cstr()[idx];
    }

    /**
     * 
     */
    std::size_t length() const {
        return size;
    }
    
    /**
     * 
     */
    void set_length(std::size_t newlen, const TCHAR& ch = 0);

    /**
     * 
     */
    std::size_t capacity() const {
        return cap;
    }

    /**
     * 
     */
    tca::allocator* get_allocator() const {
        return allocator;
    }

    /**
     * 
     */
    TCHAR char_at(std::size_t idx) const;
    
    /**
     * 
     */
    std::size_t index_of(const TCHAR* s, std::size_t from_index = 0, std::size_t = npos()) const;
    
    /**
     * 
     */
    std::size_t last_index_of(const TCHAR* s, std::size_t = npos()) const;

    /**
     * 
     */
    bool contains(const TCHAR* s, std::size_t = npos()) const;

    /**
     * 
     */
    bool starts_with(std::size_t offset, const TCHAR* s, std::size_t = npos()) const;
    
    /**
     * 
     */
    bool starts_with(const TCHAR* s, std::size_t len = npos()) const {
        return starts_with(0, s, len);
    }
    
    /**
     * 
     */
    bool starts_with(std::size_t offset, const tstring<TCHAR>& s) const {
        return starts_with(offset, s.cstr());
    }
    
    /**
     * 
     */
    bool starts_with(const tstring<TCHAR>& s) const {
        return starts_with(s.cstr(), s.length());
    }
    
    /**
     * 
     */
    bool ends_with(const TCHAR* s, std::size_t = npos()) const;
    
    /**
     * 
     */
    bool ends_with(const tstring<TCHAR>& s) const {
        return ends_with(s.cstr(), s.length());
    }

    /**
     * 
     */
    tstring<TCHAR>& append(const tstring<TCHAR>& s);

    /**
     * 
     */
    void clear();

    /**
     * 
     */
    bool is_empty() const;

    /**
     * 
     */
    bool equals(const tstring<TCHAR>& s) const;
    
    /**
     * 
     */
    bool equals(const TCHAR* s) const;
    
    /**
     * 
     */
    bool operator==(const tstring<TCHAR>& s) const {
        return equals(s);
    }
    
    /**
     * 
     */
    bool operator==(const TCHAR* s) const {
        return equals(s);
    }

    /**
     * 
     */
    std::size_t hashcode() const;

    /**
     * 
     */
    int compare_to(const tstring<TCHAR>& r);

    /**
     * 
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
    out << s.cstr();
    return out;
}

inline std::wostream& operator<<(std::wostream& out, const tc::wstring& s) {
    out << s.cstr();
    return out;
}

#endif /* A6379E4D_4085_4DC3_B5E7_3C4A783981AF */
