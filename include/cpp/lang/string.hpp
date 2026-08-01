#ifndef A6379E4D_4085_4DC3_B5E7_3C4A783981AF
#define A6379E4D_4085_4DC3_B5E7_3C4A783981AF

#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/math.hpp>
#include <utility>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cwchar>

namespace tc
{

template<typename TCHAR>
class tstring {
    /**
     * 
     */
    static const std::size_t INLINE_BUFFER_SIZE = 16;
    
    /**
     * 
     */
    union {
        TCHAR*  data;
        TCHAR   inline_data[INLINE_BUFFER_SIZE];
    };
    
    tca::allocator* allocator;
    std::size_t     cap;
    std::size_t     size;

    /**
     * 
     */
    std::size_t rem() const {
        assert(cap >= size);
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
    tstring(const tstring<TCHAR>& s, tca::allocator* allocator = tca::get_default_allocator());
    
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
    bool operator==(const tstring<TCHAR>& s) const {
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
};

    template<typename TCHAR>
    /*static*/ const std::size_t tstring<TCHAR>::INLINE_BUFFER_SIZE;

    template<typename TCHAR>
    tstring<TCHAR>::tstring(tca::allocator* allocator) : data(nullptr), allocator(allocator), cap(INLINE_BUFFER_SIZE), size(0) {

    }

    template<typename TCHAR>
    tstring<TCHAR>::tstring(const TCHAR* s, tca::allocator* alloc) : data(nullptr), allocator(alloc), cap(INLINE_BUFFER_SIZE), size(0) {
        append(0, s);
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(const tstring<TCHAR>& s, tca::allocator* allocator) : tstring<TCHAR>(s.cstr(), allocator) {
        
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(tstring<TCHAR>&& s) : tstring<TCHAR>(tca::get_default_allocator()) {
        std::swap(allocator,    s.allocator);
        std::swap(cap,          s.cap);
        std::swap(size,         s.size);
        if (!s.is_inline_string())  std::swap(data, s.data);
        else                        std::memcpy(inline_data, s.inline_data, INLINE_BUFFER_SIZE * sizeof(TCHAR));
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::operator=(const tstring<TCHAR>& s) {
        if (&s != this)
        {
            if (s.length() < INLINE_BUFFER_SIZE)
            {
                if (!is_inline_string())
                    allocator->deallocate(data);
                
                assert(s.length() < INLINE_BUFFER_SIZE);
                
                std::memcpy(cstr(), s.cstr(), (s.length() + 1) * sizeof(TCHAR));
                cap  = INLINE_BUFFER_SIZE;
                size = s.length();
            }
            else
            {
                assert(allocator != nullptr);
                
                std::size_t new_cap = (s.length() + 1);
                TCHAR* new_data = static_cast<TCHAR*>(
                                                        allocator->allocate_align(new_cap * sizeof(TCHAR), alignof(TCHAR))
                                                    );
                if (!new_data)
                    throw_except<out_of_memory_error>("Out of memory");
                
                std::memcpy(new_data, s.cstr(), (s.length() + 1) * sizeof(TCHAR));

                if (!is_inline_string())
                    allocator->deallocate(data);
                
                data    = new_data;
                cap     = new_cap;
                size    = s.length();
            }
        }
        return *this;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::operator=(tstring<TCHAR>&& s) {
        if (&s != this)
        {
            std::swap(allocator,    s.allocator);
            std::swap(cap,          s.cap);
            std::swap(size,         s.size);
            if (!s.is_inline_string())  std::swap(data, s.data);
            else                        std::memcpy(inline_data, s.inline_data, INLINE_BUFFER_SIZE * sizeof(TCHAR)); 
        }
        return *this;
    }

    template<typename TCHAR>
    void tstring<TCHAR>::ensure_cap(std::size_t new_size) {
        std::size_t new_cap = new_size + 1;
        TCHAR* new_data     = static_cast<TCHAR*>( allocator->allocate_align(sizeof(TCHAR) * new_cap, alignof(TCHAR)) );
        if (!new_data)
            throw_except<out_of_memory_error>("Out of memory");

        std::memcpy(new_data, cstr(), (size + 1) * sizeof(TCHAR));

        //Если предыдущий буфер строки не встроен в строку, освобождаем
        if (!is_inline_string())
            allocator->deallocate(data);
        
        data = new_data;
        cap  = new_cap;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::~tstring() {
        if (!is_inline_string() && data)
        {
            allocator->deallocate(data);
        }
    }

    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::cstr() {
        return is_inline_string() ? inline_data : data;
    }

    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::cstr() const {
        return is_inline_string() ? inline_data : data;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::reserve(std::size_t sz) {
        if (sz >= INLINE_BUFFER_SIZE)
        {
            std::size_t new_cap = sz + 1;
            std::size_t len     = math::min(length(), sz);

            TCHAR* new_data = static_cast<TCHAR*>( allocator->allocate_align(new_cap * sizeof(TCHAR), alignof(TCHAR)) );
            if (!new_data)
                throw_except<out_of_memory_error>("Out of memory");
            
            std::memcpy(new_data, cstr(), len * sizeof(TCHAR));
            new_data[len] = 0;
            
            if (!is_inline_string())
                allocator->deallocate(data);
            
            data    = new_data;
            cap     = new_cap;
            size    = len;
        }
        else
        {
            if (!is_inline_string())
            {
                TCHAR* data = cstr();
                std::memcpy(inline_data, data, math::min(INLINE_BUFFER_SIZE, size) * sizeof(TCHAR));
                cap = INLINE_BUFFER_SIZE;
                allocator->deallocate(data);
            }
            size    = math::min(size, sz);
            inline_data[sz] = 0;
        }
        return *this;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::append(std::size_t idx, const TCHAR* s, std::size_t len) {
        JSTD_DEBUG_CODE(
            if (idx > size)
                throw_except<index_out_of_bound_exception>("Index %zu out of bound for length %zu!", idx, size);
        )
        
        len = normalize_length(s, len);

        if (rem() <= len)
            ensure_cap(size + len);
        
        TCHAR* str = cstr();
        
        std::memmove(
                    str + (idx + len), 
                    str + idx, 
                    (size - idx) * sizeof(TCHAR)
        );
        
        std::memcpy(
            str + idx, 
            s, 
            len * sizeof(TCHAR)
        );
        
        size += len;
        str[size] = 0;

        return *this;
    }


    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::append(const tstring<TCHAR>& s) {
        append(s.cstr(), s.length());
        return *this;
    }

    template<typename TCHAR>
    TCHAR tstring<TCHAR>::char_at(std::size_t idx) const {
        check_index(idx, size);
        return cstr()[idx];
    }
    
    template<typename TCHAR>
    std::size_t tstring<TCHAR>::index_of(const TCHAR* s, std::size_t from_index, std::size_t len) const {
        JSTD_DEBUG_CODE(
            if (from_index >= length()) throw_except<illegal_argument_exception>("from_index can't be greater length");
        )
        
        len = normalize_length(s, len);
        if (len == 0 || len > (length() - from_index)) return npos();

        for (std::size_t i = from_index; i <= length() - len; ++i)
        {
            bool match = true;
            for (std::size_t j = 0; j < len; ++j)
            {
                assert(j < len);
                assert(i + j < size);
                if (cstr()[i + j] != s[j])
                {
                    match = false;
                    break;
                }
            }   
            if (match) return i;
        }

        return npos();
    }
    
    template<typename TCHAR>
    std::size_t tstring<TCHAR>::last_index_of(const TCHAR* s, std::size_t len) const {
        len = normalize_length(s, len);
    
        if (len == 0 || len > length()) return npos();
        
        for (std::size_t i = length() - len + 1; i > 0;)
        {
            --i;
            bool match = true;
            for (std::size_t j = 0; j < len; ++j)
            {
                if (cstr()[i + j] != s[j])
                {
                    match = false;
                    break;
                }
            }
            
            if (match) return i;
        }
        
        return npos();
    }

    template<typename TCHAR>
    void tstring<TCHAR>::clear() {
        cstr()[0]   = '\0';
        size        = 0;
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::is_empty() const {
        return length() == 0;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::remove(std::size_t start, std::size_t end) {
       JSTD_DEBUG_CODE(
            if (end < start)    throw_except<illegal_argument_exception>("'start' can't less 'end' where [start: %zu, end: %zu]", start, end);
            if (end > length()) throw_except<illegal_argument_exception>("'end' must be less or equal 'length' where [start: %zu, length: %zu]", start, length());
        )
        std::size_t len = end - start;
        TCHAR* str      = cstr();
        
        std::size_t s = start;                          //start
        std::size_t e = end;                            //end
        std::size_t l = (size - end) * sizeof(TCHAR);   //length

        if (l > 0)
        {
            std::memmove(str + s, str + e, l);
        }

        size -= len;    
        str[size] = 0;

        return *this;
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::contains(const TCHAR* s, std::size_t len) const {
        return index_of(s, len) != npos();
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::starts_with(std::size_t offset, const TCHAR* s, std::size_t len) const {

        len = normalize_length(s, len);
        
        if (len == 0 || len > length()) return false;
        if (offset >= length())         return false;
        if (length() - offset < len)    return false;

        for (std::size_t i = 0; i < len; ++i)
        {
            assert( 
                (offset < length()) && 
                (i < length() - offset) 
            );
            if (cstr()[i + offset] != s[i])
                return false;
        }

        return true;
    }
    
    template<typename TCHAR>
    bool tstring<TCHAR>::ends_with(const TCHAR* s, std::size_t len) const {
        len = normalize_length(s, len);
        if (len == 0 || len > length())
            return false;
        
        std::size_t start   = length() - len;
    
        for (std::size_t i = start, j = 0; j < len; ++i, ++j)
            if (cstr()[i] != s[j])
                return false;

        return true;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::replace_all(const TCHAR* matcher, const TCHAR* replacement) {
        std::size_t matcher_len     = str_len(matcher);
        std::size_t replacement_len = str_len(replacement);
        
        if (matcher_len == 0 || matcher_len > length())
            return *this;

        for (std::size_t i = 0; i <= length() - matcher_len; )
        {
            bool matched = match(cstr() + i, matcher, matcher_len);
            if (matched)
            {
                remove(i, i + matcher_len);
                append(i, replacement, replacement_len);
                i += replacement_len;
            }
            else
            {
                ++i;
            }
        }

        return *this;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::replace(std::size_t start, std::size_t end, const TCHAR* replacement) {
        JSTD_DEBUG_CODE(
            if (start >= length())  throw_except<illegal_argument_exception>("start can't be equal or greater length()");
            if (end   >  length())  throw_except<illegal_argument_exception>("end can't be greater length()");
            if (end   <  start)     throw_except<illegal_argument_exception>("end can't be less start");
        );
        remove(start, end);
        append(start, replacement);
        return *this;
    }

    template<typename TCHAR>
    tstring<TCHAR> tstring<TCHAR>::sub_string(std::size_t start, std::size_t end, tca::allocator* allocator) const {
        JSTD_DEBUG_CODE(
            if (end < start)    throw_except<illegal_argument_exception>("'start' can't less 'end' where [start: %zu, end: %zu]", start, end);
            if (end > length()) throw_except<illegal_argument_exception>("'end' must be less or equal 'length' where [start: %zu, length: %zu]", start, length());
        )
        
        std::size_t len = end - start;

        tstring<TCHAR> sub(allocator);
        sub.reserve(len);
        sub.append(cstr() + start, len);
        
        return tstring<TCHAR>(std::move(sub));
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::equals(const tstring<TCHAR>& s) const {
		if (length() != s.length())
            return false;
        return objects::equals(cstr(), s.cstr(), length());
    }

    template<typename TCHAR>
    std::size_t tstring<TCHAR>::hashcode() const {
        return objects::hashcode(cstr(), length());
    }
    
    template<typename TCHAR>
    int tstring<TCHAR>::compare_to(const tstring<TCHAR>& r) {
        std::size_t len = math::min(length(), r.length());
        for (std::size_t i = 0; i < len; ++i)
            if (char_at(i) < r.char_at(i))
                return -1;
            else if (char_at(i) > r.char_at(i))
                return 1;
        if (length() < r.length()) return -1;
        if (length() > r.length()) return 1;
        return 0;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::trim() {
        std::size_t start = 0;
        std::size_t end = length();

        while (start < end && char_at(start) <= 0x20)
            ++start;

        while (end > start && char_at(end - 1) <= 0x20)
            --end;

        const std::size_t len = end - start;

        if (start != 0 && len != 0)
            std::memmove(cstr(), cstr() + start, len * sizeof(TCHAR));

        size = len;
        cstr()[size] = 0;

        return *this;
    }

    template<typename TCHAR>
    void tstring<TCHAR>::set_length(std::size_t newlen, const TCHAR& ch) {
        if (newlen >= capacity() - 1)
        {
            reserve(newlen);
        }

        if (newlen > length())
        {
            while (size < newlen)
                cstr()[size++] = ch;
            cstr()[size] = 0;
        }
        else
        {
            cstr()[newlen] = 0;
            size = newlen;
        }
    }

namespace internal
{
    
    static const std::size_t CHAR_BUF = 32;
    template<typename E>
    tstring<char> to_string0(const E& e, const char* ext, tca::allocator* allocator) {
        tstring<char> str(allocator);
        char buf[CHAR_BUF];    
        int len = std::snprintf(buf, CHAR_BUF, ext, e);
        if (len > 0)
            str.append(buf, static_cast<std::size_t>(len));
        return tstring<char>( std::move(str) );
    }

    template<typename E>
    tstring<wchar_t> to_wstring0(const E& e, const wchar_t* ext, tca::allocator* allocator) {
        tstring<wchar_t> str(allocator);
        wchar_t buf[CHAR_BUF];    
        int len = std::swprintf(buf, CHAR_BUF, ext, e);
        if (len > 0)
            str.append(buf, static_cast<std::size_t>(len));
        return tstring<wchar_t>( std::move(str) );
    }
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
        return internal::to_string0<const char*>(e, "%s", allocator);
    }
    
    template<>
    inline tstring<char> to_string<const char>(const char* e, tca::allocator* allocator) {
        return internal::to_string0<const char*>(e, "%s", allocator);
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

}//namespace jstd

#endif /* A6379E4D_4085_4DC3_B5E7_3C4A783981AF */
