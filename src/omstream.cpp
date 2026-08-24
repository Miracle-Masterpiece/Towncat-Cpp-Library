#include <cpp/lang/io/omstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc
{
    
    omstream::omstream(std::size_t init_buf_size, tca::allocator* allocator) : 
    m_allocator(allocator),
    m_buffer(nullptr),
    m_capacity(init_buf_size),
    m_offset(0) {

    }

    omstream::omstream(char* buf, std::size_t capacity) :
    m_allocator(nullptr),
    m_buffer(buf),
    m_capacity(capacity),
    m_offset(0) {

    }

    omstream::omstream(omstream&& stream) : 
    m_allocator(stream.m_allocator), 
    m_buffer(stream.m_buffer), 
    m_capacity(stream.m_capacity), 
    m_offset(stream.m_offset) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
    }

    omstream& omstream::operator= (omstream&& stream) {
        if (&stream != this)
        {
            if (m_buffer)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator   = stream.m_allocator;
            m_buffer      = stream.m_buffer;
            m_capacity    = stream.m_capacity;
            m_offset      = stream.m_offset;
            
            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
        }
        return *this;
    }

    void omstream::resize(std::size_t sz) {
        std::size_t s = m_capacity == 0 ? INIT_BUF_SIZE : m_capacity;
        
        if (s <= 1) s = 2;
        while (sz + m_offset > s)
            s = s + (s >> 1);

        char* new_buffer = (char*) m_allocator->allocate_align(s, alignof(char));
        if (!new_buffer)
            throw_except<out_of_memory_error>("out of memory");
        
        std::memcpy(new_buffer, m_buffer, m_offset);
        
        m_allocator->deallocate(m_buffer);
        m_buffer     = new_buffer;
        m_capacity   = s;
    }

    void omstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_allocator && !m_buffer)
                throw_except<io_exception>("Stream is null");
        )

        if ((m_capacity - m_offset < sz) || (!m_buffer))
        {
            if (!m_allocator)
                throw_except<overflow_exception>("omstream buffer is owerflow!");
            resize(sz);
        }
        
        std::memcpy(m_buffer + m_offset, data, sz);
        m_offset += sz;
    }

    void omstream::flush() {
        JSTD_DEBUG_CODE
        (
            if (!m_allocator && !m_buffer)
                throw_except<io_exception>("Stream is null");
        );
    }

    void omstream::close(error_code& err) {
        if (!m_buffer)
            return;
        if (m_allocator)
        {
            m_allocator->deallocate(m_buffer);
            m_allocator = nullptr;
        }
        m_buffer = nullptr;
    }

    omstream::~omstream() {
        error_code dontcare;
        close(dontcare);
    }

    const char* omstream::data() const {
        return m_buffer;
    }
    
    std::size_t omstream::offset() const {
        return m_offset;
    }
}