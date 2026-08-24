#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc
{

    ibstream::ibstream() : m_allocator(nullptr), m_buffer(nullptr), m_capacity(0), m_offset(0), m_limit(0), m_in(nullptr) {

    }

    ibstream::ibstream(istream* stream, std::size_t buf_size, tca::allocator* allocator) : ibstream() {
        char* data = (char*) allocator->allocate_align(buf_size, alignof(char));
        m_allocator  = allocator;
        m_buffer     = data;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_limit      = 0;
        m_in         = stream;
    }
    
    ibstream::ibstream(istream* stream, char* buf, std::size_t buf_size) : ibstream() {
        m_buffer     = buf;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_limit      = 0;
        m_in         = stream;
    }
    
    ibstream::ibstream(ibstream&& stream) : 
    m_allocator(stream.m_allocator), 
    m_buffer(stream.m_buffer),
    m_capacity(stream.m_capacity),
    m_offset(stream.m_offset),
    m_limit(stream.m_limit),
    m_in(stream.m_in) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
        stream.m_limit       = 0;
        stream.m_in          = nullptr;
    }
    
    ibstream& ibstream::operator= (ibstream&& stream) {
        if (&stream != this) {
            if (m_in != nullptr)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator   = stream.m_allocator;
            m_buffer      = stream.m_buffer;
            m_capacity    = stream.m_capacity;
            m_offset      = stream.m_offset;
            m_limit       = stream.m_limit;
            m_in          = stream.m_in;
            
            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
            stream.m_limit       = 0;
            stream.m_in          = nullptr;
        }
        return *this;
    }
    
    void ibstream::fill_buffer() {
        std::size_t readed = m_in->read(m_buffer, m_capacity);
        m_limit  = (readed == istream::eof_value()) ? 0 : readed;
        m_offset = 0;
    }

    std::size_t ibstream::read(char* buf, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_in)
                throw_except<io_exception>("stream is null");
        );
        
        std::size_t total_readed = 0;
        
        while (total_readed < sz)
        {
            if (m_offset >= m_limit)
            {
                fill_buffer();
                if (m_limit == 0)
                    break; // EOF
            }
            
            std::size_t available = m_limit - m_offset;
            std::size_t to_read = std::min(available, sz - total_readed);
            
            if (to_read > 0)
            {
                memcpy(buf + total_readed, m_buffer + m_offset, to_read);
                m_offset += to_read;
                total_readed += to_read;
            }
        }
        
        return total_readed > 0 ? 
                                    total_readed : istream::eof_value();
    }
    
    void ibstream::free() {
        if (m_allocator)
        {
            m_allocator->deallocate(m_buffer, m_capacity);
            m_allocator = nullptr;
        }
    }

    ibstream::~ibstream() {
		error_code dontcare;
        close(dontcare);
    }

    void ibstream::close(error_code& err) {
        if (m_in == nullptr)
            return;
        m_in->close(err);
        m_in = nullptr;
        free();
    }
}