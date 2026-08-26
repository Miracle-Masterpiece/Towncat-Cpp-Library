#include <cpp/lang/io/imstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace tc
{


    imstream::imstream() : m_buffer(nullptr), m_capacity(0), m_offset(0) {

    }

    imstream::imstream(const char* buffer, std::size_t capacity) : m_buffer(buffer), m_capacity(capacity), m_offset(0) {
        if (buffer == nullptr)
            throw_except<null_pointer_exception>("buffer is null");
    }
    
    imstream::imstream(imstream&& stream) : m_buffer(stream.m_buffer), m_capacity(stream.m_capacity), m_offset(stream.m_offset) {
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
    }
    
    imstream& imstream::operator= (imstream&& stream) {
        if (&stream != this) {
            if (m_buffer != nullptr)
                close();
            m_buffer     = stream.m_buffer;
            m_capacity   = stream.m_capacity;
            m_offset     = stream.m_offset;

            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
        }
        return *this;
    }
    
    imstream::~imstream() {

    }
    
    std::size_t imstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (m_buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        );
        sz = std::min(m_capacity - m_offset, sz);
        if (sz == 0)
            return istream::eof_value();

        std::memcpy(buf, m_buffer + m_offset, sz);
        m_offset += sz;
        
        return sz;
    }

    void imstream::close(error_code& err) {
        m_buffer = nullptr;
    }
    
    void imstream::reset() {
        m_offset = 0;
    }
}