#include <cpp/lang/cstr.hpp>

namespace tc
{

    c_str::c_str(const char* s) : m_cstr(s), m_length(~(std::size_t) 0) {
        
    }

    c_str::operator const char* () const {
        return m_cstr;
    }
    
    std::size_t c_str::length() const {
        if (m_length == ~(std::size_t) 0)
        {
            if (m_cstr != nullptr)
            {
                m_length = std::strlen(m_cstr);
            }
            else
            {
                m_length = 0;
            }
        }
        return m_length;
    }

    const char& c_str::operator[] (std::size_t idx) const {
#ifndef NDEBUG
        check_index(idx, length());
#endif//NDEBUG
        return m_cstr[idx];
    }

    bool c_str::equals(const c_str& s) const {
        std::size_t len1 = length();
        std::size_t len2 = s.length();
        if (len1 != len2)
            return false;
        for (std::size_t i = 0, len = len1; i < len; ++i)
            if ((*this)[i] != s[i])
                return false;
        return true;
    }

    bool c_str::operator==(const c_str& s) const {
        return equals(s);
    }
    
    bool c_str::operator!=(const c_str& s) const {
        return !equals(s);
    }

    std::size_t c_str::hashcode() const {
        return objects::hashcode(m_cstr,  m_cstr + m_length, hash_for<char>());
    }

    bool c_str::is_empty() const {
        return length() == 0;
    }

}