#include <cpp/lang/io/bytebuf.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cassert>

namespace tc
{

    
    bytebuf::bytebuf(tca::allocator* alloc) : bytebuf(0, alloc) {
        
    }
    
    bytebuf::bytebuf(std::size_t capacity, tca::allocator* alloc) : basebuf(), m_alloc(alloc) {
        if (capacity > 0)
        {
            char* data = (char*) m_alloc->allocate_align(capacity, alignof(char));
            if(!data)
                throw_except<out_of_memory_error>("Out of memory");
            m_data      = data;
            m_capacity  = capacity;
            m_limit     = capacity;
        }
    }

    bytebuf& bytebuf::operator= (const bytebuf& buf) {
        assert(m_alloc != nullptr);
        
        char* newdata       = nullptr;
        std::size_t newcap  = buf.m_capacity;
        if (newcap > 0)
        {
            newdata = (char*) m_alloc->allocate_align(newcap, alignof(char));
            if (!newdata)
                throw_except<out_of_memory_error>("Out of memory");
            std::memcpy(newdata, buf.m_data, newcap);
        }

        if (m_data)
            m_alloc->deallocate(m_data);
        
        m_data      = newdata;
        m_capacity  = newcap;
        
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_position  = buf.m_position;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;

        return *this;
    }

    bytebuf::bytebuf(const bytebuf& buf) : basebuf(), m_alloc(buf.m_alloc) {
        char* newdata       = nullptr;
        std::size_t newcap  = buf.m_capacity;
        if (newcap > 0)
        {
            newdata = (char*) m_alloc->allocate_align(newcap, alignof(char));
            if (!newdata)
                throw_except<out_of_memory_error>("Out of memory");
            std::memcpy(newdata, buf.m_data, newcap);
        }

        if (m_data)
            m_alloc->deallocate(m_data);
        
        m_data      = newdata;
        m_capacity  = newcap;
        
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_position  = buf.m_position;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;
    }

    bytebuf::bytebuf(bytebuf&& buf) : basebuf(), m_alloc(buf.m_alloc) {
        m_data      = buf.m_data;
        m_capacity  = buf.m_capacity;
        m_position  = buf.m_position;
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;
        
        m_data      = nullptr;
        m_capacity  = 0;
    }

    bytebuf& bytebuf::operator= (bytebuf&& buf) {
        if (&buf == this)
            return *this;

        if (get_allocator() == buf.get_allocator())
        {
            std::swap(m_data,       buf.m_data);
            std::swap(m_capacity,   buf.m_capacity);
            std::swap(m_position,   buf.m_position);
            std::swap(m_limit,      buf.m_limit);
            std::swap(m_mark,       buf.m_mark);
            std::swap(m_order,      buf.m_order);
            std::swap(m_readonly,   buf.m_readonly);
        }
        else
        {
            *this = buf;
        }

        return *this;
    }

    bytebuf::~bytebuf() {
        if(m_data)
            m_alloc->deallocate(m_data);
    }
}