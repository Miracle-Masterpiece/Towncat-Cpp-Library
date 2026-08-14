#include <cpp/lang/string.hpp>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cwchar>

namespace tc
{
    template<typename TCHAR>
    tstring<TCHAR>::tstring(tca::allocator* allocator) : data(nullptr), allocator(allocator), cap(INLINE_BUFFER_SIZE), size(0) {

    }

    template<typename TCHAR>
    tstring<TCHAR>::tstring(const TCHAR* s, tca::allocator* alloc) : data(nullptr), allocator(alloc), cap(INLINE_BUFFER_SIZE), size(0) {
        append(0, s);
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(const tstring<TCHAR>& s) : tstring<TCHAR>(s.cstr(), s.get_allocator()) {
        
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(tstring<TCHAR>&& s) : allocator(s.allocator), cap(s.cap), size(s.size) {
        if (s.is_inline_string())
        {
            std::memcpy(inline_data, s.inline_data, (s.size + 1) * sizeof(TCHAR));
        }
        else
        {
            data = s.data;
            s.data = nullptr;
        }

        s.cap   = INLINE_BUFFER_SIZE;
        s.size  = 0;
        s.inline_data[0] = TCHAR(0);
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
        if (this == &s) return *this;

        if (get_allocator() == s.get_allocator())
        {
            if (!is_inline_string())
            {
                allocator->deallocate(data);
            }
            
            if (s.is_inline_string())
            {
                std::memcpy(cstr(), s.cstr(), s.length());
                cap  = s.cap;
                size = s.length();
            }
            else
            {
                data   = s.data;
                s.data = nullptr;

                size   = s.size;
                s.size = 0;

                cap    = s.cap;
                
                s.cap            = INLINE_BUFFER_SIZE;
                s.inline_data[0] = 0;
            }

        }
        else
        {
            *this = s;
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
                std::memcpy(inline_data, data, math::min(static_cast<std::size_t>(INLINE_BUFFER_SIZE), size) * sizeof(TCHAR));
                cap = static_cast<std::size_t>(INLINE_BUFFER_SIZE);
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
		if (length() != s.length()) return false;
        return objects::equals(begin(), end(), s.begin(), s.end());
    }
    
    template<typename TCHAR>
    bool tstring<TCHAR>::equals(const TCHAR* s) const {
		if (length() != str_len(s)) return false;
        return objects::equals(begin(), end(), s, s + length());
    }

    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::begin() {
        return cstr();
    }
    
    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::end() {
        return cstr() + length();
    }

    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::begin() const {
        return cstr();
    }
    
    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::end() const {
        return cstr() + length();
    }

    template<typename TCHAR>
    std::size_t tstring<TCHAR>::hashcode() const {
        return objects::hashcode(cstr(), cstr() + length(), hash_for<TCHAR>());
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
} //namespace tc

namespace tc
{
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

    // explicit instancing to_string
    template tstring<char> to_string0<char>                 (const char&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned char>        (const unsigned char&, const char*, tca::allocator*);
    template tstring<char> to_string0<signed char>          (const signed char&, const char*, tca::allocator*);
    template tstring<char> to_string0<short>                (const short&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned short>       (const unsigned short&, const char*, tca::allocator*);
    template tstring<char> to_string0<int>                  (const int&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned int>         (const unsigned int&, const char*, tca::allocator*);
    template tstring<char> to_string0<long>                 (const long&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned long>        (const unsigned long&, const char*, tca::allocator*);
    template tstring<char> to_string0<long long>            (const long long&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned long long>   (const unsigned long long&, const char*, tca::allocator*);
    template tstring<char> to_string0<float>                (const float&, const char*, tca::allocator*);
    template tstring<char> to_string0<double>               (const double&, const char*, tca::allocator*);
    template tstring<char> to_string0<long double>          (const long double&, const char*, tca::allocator*);
    template tstring<char> to_string0<void*>                (void* const&, const char*, tca::allocator*);
    template tstring<char> to_string0<const void*>          (const void* const&, const char*, tca::allocator*);
    
    // explicit instancing to_wstring
    template tstring<wchar_t> to_wstring0<wchar_t>              (const wchar_t&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned char>        (const unsigned char&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<signed char>          (const signed char&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<short>                (const short&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned short>       (const unsigned short&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<int>                  (const int&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned int>         (const unsigned int&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long>                 (const long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned long>        (const unsigned long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long long>            (const long long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned long long>   (const unsigned long long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<float>                (const float&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<double>               (const double&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long double>          (const long double&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<void*>                (void* const&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<const void*>          (const void* const&, const wchar_t*, tca::allocator*);

} //namespace internal
} //namespace tc

// Explicit instaincing strings
namespace tc
{
    template class tstring<char>;
    template class tstring<unsigned char>;
    template class tstring<signed char>;
    
    template class tstring<wchar_t>;
    
    template class tstring<short>;
    template class tstring<unsigned short>;
    
    template class tstring<int>;
    template class tstring<unsigned int>;
    
    template class tstring<long>;
    template class tstring<unsigned long>;
    
    template class tstring<long long>;
    template class tstring<unsigned long long>;
} //namespace tc