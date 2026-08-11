#include <cpp/lang/cstr.hpp>

namespace tc
{

    cstr::cstr(const char* s) : m_cstr(s), m_length(~(std::size_t) 0) {
        
    }

    cstr::operator const char* () const {
        return m_cstr;
    }
    
    std::size_t cstr::length() const {
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

    const char& cstr::operator[] (std::size_t idx) const {
#ifndef NDEBUG
        check_index(idx, length());
#endif//NDEBUG
        return m_cstr[idx];
    }

    bool cstr::equals(const cstr& s) const {
        std::size_t len1 = length();
        std::size_t len2 = s.length();
        if (len1 != len2)
            return false;
        for (std::size_t i = 0, len = len1; i < len; ++i)
            if ((*this)[i] != s[i])
                return false;
        return true;
    }

    bool cstr::operator==(const cstr& s) const {
        return equals(s);
    }
    
    bool cstr::operator!=(const cstr& s) const {
        return !equals(s);
    }

    std::size_t cstr::hashcode() const {
        return objects::hashcode(m_cstr,  m_cstr + m_length, hash_for<char>());
    }

    bool cstr::is_empty() const {
        return length() == 0;
    }

}