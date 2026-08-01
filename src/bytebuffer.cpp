#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cassert>
#include <exception>
#include <cstdio>

#define BYTE_BITS 8

namespace tc {

    static std::size_t null_value() {
        return (std::size_t) -1;
    }

    byte_buffer::byte_buffer() :
    m_allocator(nullptr), 
    m_data(nullptr), 
    m_capacity(0), 
    m_position(0), 
    m_limit(0), 
    m_mark(null_value()),
    m_order(system::native_byte_order()),
    m_readonly(false) {

    }

    byte_buffer::byte_buffer(char* buf, std::size_t bufsize, bool readonly) : 
    m_allocator(nullptr), 
    m_data(buf), 
    m_capacity(bufsize), 
    m_position(0), 
    m_limit(bufsize), 
    m_mark(null_value()),
    m_order(system::native_byte_order()),
    m_readonly(readonly) {

    }
    
    byte_buffer::byte_buffer(std::size_t capacity, tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_data(nullptr), 
    m_capacity(0), 
    m_position(0), 
    m_limit(0), 
    m_mark(null_value()),
    m_order(system::native_byte_order())  {
        if (capacity > 0) {
            char* data = (char*) allocator->allocate_align(capacity, alignof(char));
            if (data == nullptr)
                throw_except<out_of_memory_error>("Out of memory");
            m_allocator  = allocator;
            m_data       = data;
            m_capacity   = capacity;
            m_limit      = m_capacity;
            m_position   = 0;
        }
    }
    
    byte_buffer::byte_buffer(byte_buffer&& buf) : 
    m_allocator(buf.m_allocator), 
    m_data(buf.m_data), 
    m_capacity(buf.m_capacity), 
    m_position(buf.m_position),
    m_limit(buf.m_limit), 
    m_mark(buf.m_mark),
    m_order(buf.m_order),
    m_readonly(buf.m_readonly) {
        buf.m_allocator  = nullptr;
        buf.m_data       = nullptr;
        buf.m_capacity   = 0;
        buf.m_position   = 0;
        buf.m_limit      = 0;
        buf.m_mark       = null_value();
        buf.m_readonly   = false;
    }
    
    byte_buffer& byte_buffer::operator= (byte_buffer&& buf) {
        if (&buf != this)
        {
            std::swap(m_allocator, buf.m_allocator);
            std::swap(m_data, buf.m_data);
            std::swap(m_position, buf.m_position);
            std::swap(m_limit, buf.m_limit);
            std::swap(m_mark, buf.m_mark);
            std::swap(m_order, buf.m_order);
            std::swap(m_readonly, buf.m_readonly);
        }
        return *this;
    }
    
    void byte_buffer::dispose() {
        if (m_allocator != nullptr) {
            if (m_data != nullptr)
                m_allocator->deallocate(m_data, m_capacity);
        }
    }

    byte_buffer::~byte_buffer() {
        dispose();
    }

    /*static*/ byte_buffer byte_buffer::allocate(std::size_t capacity, tca::allocator* allocator) {
        return byte_buffer(capacity, allocator);
    }

    std::size_t byte_buffer::position() const {
        return m_position;
    }
    
    byte_buffer& byte_buffer::position(std::size_t newpos) {
        assert(newpos <= m_limit);
        if (m_mark > newpos)
            m_mark = null_value();
        m_position = newpos;
        return *this;
    }

    std::size_t byte_buffer::limit() const {
        return m_limit;
    }
    
    byte_buffer& byte_buffer::limit(std::size_t newlimit){
        assert(newlimit <= m_capacity);
        m_limit = newlimit;
        if (m_position > newlimit) m_position   = newlimit;
        if (m_mark > newlimit)     m_mark       = null_value();
        return *this;
    }

    std::size_t byte_buffer::remaining() const {
        if (m_position > m_limit)
            return 0;
        return m_limit - m_position;
    }

    std::size_t byte_buffer::capacity() const {
        return m_capacity;
    }

    byte_buffer& byte_buffer::clear() {
        m_position  = 0;
        m_limit     = m_capacity;
        m_mark      = null_value();
        return *this;
    }
    
    byte_buffer& byte_buffer::compact() {
        char* src = m_data + m_position;
        char* dst = m_data;
        if (m_limit >= m_position)
        {
            std::size_t len = m_limit - m_position;
            if (len > 0)
                std::memmove(dst, src, len);
            m_position    = len;
            m_limit  = m_capacity;
            m_mark   = null_value();
        }
        return *this;
    }
    
    byte_buffer& byte_buffer::flip() {
        m_limit  = m_position;
        m_position    = 0;
        m_mark   = null_value();
        return *this;
    }

    byte_buffer& byte_buffer::mark() {
        m_mark = m_position;
        return *this;
    }

    byte_order byte_buffer::order() const {
        return m_order;
    }

    char* byte_buffer::data() {
        return m_data;
    }
    
    const char* byte_buffer::data() const {
        return m_data;
    }
    
    byte_buffer& byte_buffer::order(byte_order order) {
        m_order = order;
        return *this;
    }

#define MAKE_SPECIALIZATION__ALLOCATORS_LIB_BYTE_BUFFER_IMPL(type_name__jstd_io_bytebuffer)\
    template<>\
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer v) {\
        check_write_or_except();\
        put<type_name__jstd_io_bytebuffer>(m_position, v);\
        m_position += sizeof(v);\
        return *this;\
    }\
    template<>\
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(std::size_t idx, type_name__jstd_io_bytebuffer v) {\
        checkIndex(idx, sizeof(v));\
        check_write_or_except();\
        m_data[idx] = static_cast<char>(v);\
        return *this;\
    }\
    template<>\
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(const type_name__jstd_io_bytebuffer* arr, std::size_t sz) {\
        check_write_or_except();\
        puts<type_name__jstd_io_bytebuffer>(m_position, arr, sz);\
        m_position += sizeof(type_name__jstd_io_bytebuffer) * sz;\
        return *this;\
    }\
    template<>\
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(std::size_t idx, const type_name__jstd_io_bytebuffer* arr, std::size_t sz) {\
        check_write_or_except();\
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer) * sz);\
        std::memcpy(m_data + idx, reinterpret_cast<const char*>(arr), sizeof(type_name__jstd_io_bytebuffer) * sz);\
        return *this;\
    }\
    template<>\
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>() {\
        type_name__jstd_io_bytebuffer tmp = get<type_name__jstd_io_bytebuffer>(m_position);\
        m_position += sizeof(tmp);\
        return tmp;\
    }\
    template<>\
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>(std::size_t idx) const {\
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer));\
        return static_cast<type_name__jstd_io_bytebuffer>(m_data[idx]);\
    }\
    template<>\
    byte_buffer& byte_buffer::gets(type_name__jstd_io_bytebuffer* arr, std::size_t sz) {\
        gets<type_name__jstd_io_bytebuffer>(m_position, arr, sz);\
        m_position += sizeof(type_name__jstd_io_bytebuffer) * sz;\
        return *this;\
    }\
    template<>\
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(std::size_t idx, type_name__jstd_io_bytebuffer* arr, std::size_t sz) {\
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer) * sz);\
        std::memcpy(reinterpret_cast<char*>(arr), m_data + idx, sizeof(type_name__jstd_io_bytebuffer) * sz);\
        return *this;\
    }\

    MAKE_SPECIALIZATION__ALLOCATORS_LIB_BYTE_BUFFER_IMPL(char)
    MAKE_SPECIALIZATION__ALLOCATORS_LIB_BYTE_BUFFER_IMPL(signed char)
    MAKE_SPECIALIZATION__ALLOCATORS_LIB_BYTE_BUFFER_IMPL(unsigned char)

#undef MAKE_SPECIALIZATION__ALLOCATORS_LIB_BYTE_BUFFER_IMPL

    void byte_buffer::print() const {
        std::printf("[position=%zu, limit=%zu, capacity=%zu]\n", m_position, m_limit, m_capacity);
    }

    int byte_buffer::to_string(char buf[], std::size_t bufsize) const {
        return std::snprintf(buf, bufsize, "[position=%zu, limit=%zu, capacity=%zu]\n", m_position, m_limit, m_capacity);
    }
}