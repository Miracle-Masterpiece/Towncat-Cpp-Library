#include <cpp/lang/io/obstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <iostream>
#include <cassert>

namespace tc
{

    obstream::obstream() : m_allocator(nullptr), m_buffer(nullptr), m_capacity(0), m_offset(0), m_out(nullptr) {

    }

    obstream::obstream(ostream* stream, tca::allocator* allocator, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE
        (
            if (!stream)      throw_except<null_pointer_exception>("stream is null");
            if (!allocator)   throw_except<null_pointer_exception>("allocator is null");
        )
        char* data  = (char*) allocator->allocate_align(buf_size, alignof(char));
        if (!data)
            throw_except<out_of_memory_error>("out of memory");
        m_allocator  = allocator;
        m_buffer     = data;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_out        = stream;
    }
    
    obstream::obstream(ostream* stream, char* buffer, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE
        (
            if (stream == nullptr)  throw_except<null_pointer_exception>("stream is null");
            if (buffer == nullptr)  throw_except<null_pointer_exception>("buffer is null");
        )
        m_buffer     = buffer;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_out        = stream;
    }

    obstream::obstream(obstream&& stream) : 
    m_allocator(stream.m_allocator), m_buffer(stream.m_buffer), m_capacity(stream.m_capacity), m_offset(stream.m_offset), m_out(stream.m_out) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
        stream.m_out         = nullptr;
    }

    obstream& obstream::operator= (obstream&& stream) {
        if (&stream != this)
        {
            if (!m_out)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator  = stream.m_allocator;
            m_buffer     = stream.m_buffer;
            m_capacity   = stream.m_capacity;
            m_offset     = stream.m_offset;
            m_out        = stream.m_out;

            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
            stream.m_out         = nullptr;
        }
        return *this;
    }

    void obstream::free() {
        if (!m_allocator)
        {
            m_allocator->deallocate(m_buffer);
            m_allocator = nullptr;
        }
    }

    obstream::~obstream() {
        error_code dontcare;
        close(dontcare);
    }

    void obstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_out)
                throw_except<io_exception>("stream is null");
        )
        
        assert(m_capacity >= m_offset);
        std::size_t rem = m_capacity - m_offset;
        
        if (rem < sz)
        {
            flush();
        }
        
        if (sz > m_capacity)
        {
            m_out->write(data, sz);
        } 
        else
        {
            memcpy(m_buffer + m_offset, data, sz);
            m_offset += sz;
        }
    }
    
    void obstream::flush() {
        if (m_out)
        {
            if (m_offset > 0)
            {
                m_out->write(m_buffer, m_offset);
                m_offset = 0;
            }
        }
    }
    
    void obstream::close(error_code& err) {
        if (m_out)
        {
            
            error_code flush_err;
            error_code close_err;

            try {
                flush();
            } catch (...) {
                flush_err = error_code(errcode::io_error, generic_category());
            }
        
            m_out->close(close_err);
            
            free();
            m_out = nullptr;
            
            err = flush_err ? flush_err : close_err;
        }
    }
    
}