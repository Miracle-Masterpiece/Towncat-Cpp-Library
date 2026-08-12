#ifndef ALLOCATORS_LANG_IO_BYTEBUFFER_H_
#define ALLOCATORS_LANG_IO_BYTEBUFFER_H_

#include <allocators/allocator.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstdint>
#include <cstring>
#include <climits>

namespace tc {


class byte_buffer {
protected:
    
    /**
     * 
     */
    tca::allocator* m_allocator;
    
    /**
     * 
     */
    char* m_data;
    
    /**
     * 
     */
    std::size_t m_capacity;
    
    /**
     * 
     */
    std::size_t m_position;
    
    /**
     * 
     */
    std::size_t m_limit;
    
    /**
     * 
     */
    std::size_t m_mark;
    
    /**
     * 
     */
    byte_order m_order;
    
    /**
     * 
     */
    bool m_readonly;


    /**
     * deleted
     */
    byte_buffer(const byte_buffer&) = delete;

    /**
     * deleted
     */
    byte_buffer& operator= (const byte_buffer&) = delete;

    /**
     * 
     */
    void dispose();
    
    /**
     * 
     */
    void checkIndex(std::size_t idx, std::size_t sz) const {
        JSTD_DEBUG_CODE(
            if (idx > m_limit) {
                throw_except<index_out_of_bound_exception>("Index %zu > _limit %zu", idx, m_limit);
            }
            else if (m_limit - idx < sz) {
                throw_except<index_out_of_bound_exception>("Remaining %zu < tsize %zu", m_limit - idx, sz);
            }
        );
    }
    
    /**
     * 
     */
    void check_write_or_except() const {
        JSTD_DEBUG_CODE(
            if (m_readonly)
                throw_except<readonly_exception>("Buffer is read only");
        );
    }

public:
    /**
     * 
     */
    byte_buffer();
    
    /**
     * 
     */
    byte_buffer(char* buf, std::size_t bufsize, bool readonly = false);
    
    /**
     * 
     */
    explicit byte_buffer(std::size_t capacity, tca::allocator* allocator = tca::get_scoped_or_default());
    
    /**
     * 
     */
    byte_buffer(byte_buffer&&);
    
    /**
     * 
     */
    byte_buffer& operator= (byte_buffer&&);
    
    /**
     * 
     */
    ~byte_buffer();
    
    /**
     * 
     */
    static byte_buffer allocate(std::size_t capacity, tca::allocator* allocator = tca::get_scoped_or_default());
    
    /**
     * 
     */
    std::size_t position() const;
    
    /**
     * 
     */
    byte_buffer& position(std::size_t newpos);

    /**
     * 
     */
    std::size_t limit() const;
    
    /**
     * 
     */
    byte_buffer& limit(std::size_t newlimit);

    /**
     * 
     */
    std::size_t remaining() const;

    /**
     * 
     */
    std::size_t capacity() const;

    /**
     * 
     */
    byte_buffer& clear();
    
    /**
     * 
     */
    byte_buffer& compact();
        
    /**
     * 
     */
    byte_buffer& flip();
        
    /**
     * 
     */
    byte_buffer& mark();
    
    /**
     * 
     */
    byte_order order() const;
        
    /**
     * 
     */
    byte_buffer& order(byte_order order);
    
    /**
     * 
     */
    char* data();
        
    /**
     * 
     */
    const char* data() const;
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& put(T v);
        
    /**
     * 
     */
    template<typename T>
    byte_buffer& put(std::size_t idx, T v);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& puts(const T* arr, std::size_t sz);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& puts(std::size_t idx, const T* arr, std::size_t sz);
        
    /**
     * 
     */
    template<typename T>
    T get();
    
    /**
     * 
     */
    template<typename T>
    T get(std::size_t idx) const;
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& gets(T* arr, std::size_t sz);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& gets(std::size_t idx, T* arr, std::size_t sz);
    
    /**
     * 
     */
    void print() const;
    
    /**
     * 
     */
    int to_string(char buf[], std::size_t bufsize) const;
};

    template<typename T>
    byte_buffer& byte_buffer::put(T v) {
        check_write_or_except();                                                                                                                            \
        put<T>(m_position, v);
        m_position += sizeof(v);
        return *this;
    }
    
    template<typename T>
    byte_buffer& byte_buffer::put(std::size_t idx, T v) {
        check_write_or_except();                                                                                                                            \
        checkIndex(idx, sizeof(v));
        utils::write_with_order<T>(m_data + idx, v, m_order);
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(const T* arr, std::size_t sz) {
        check_write_or_except();                                                                                                                            \
        puts<T>(m_position, arr, sz);
        m_position += sizeof(T) * sz;
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(std::size_t idx, const T* arr, std::size_t sz) {
        check_write_or_except();                                                                                                                            \
        checkIndex(idx, sizeof(T) * sz);
        if (m_order != system::native_byte_order()) {
            utils::copy_swap_memory<T>(m_data + idx, reinterpret_cast<const char*>(arr), sz);
        } 
        else {
            std::memcpy(m_data + idx, reinterpret_cast<const char*>(arr), sizeof(T) * sz);
        }
        return *this;
    }
    
    template<typename T>
    T byte_buffer::get() {
        T tmp = get<T>(m_position);
        m_position += sizeof(tmp);
        return tmp;
    }

    template<typename T>
    T byte_buffer::get(std::size_t idx) const {
        checkIndex(idx, sizeof(T));
        return utils::read_with_order<T>(m_data + idx, m_order);
    }

    template<typename T>
    byte_buffer& byte_buffer::gets(T* arr, std::size_t sz) {
        gets<T>(m_position, arr, sz);
        m_position += sizeof(T) * sz;
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::gets(std::size_t idx, T* arr, std::size_t sz) {
        checkIndex(idx, sizeof(T) * sz);
        if (m_order != system::native_byte_order()) {
            utils::copy_swap_memory<T>(reinterpret_cast<char*>(arr), m_data + idx, sz);
        } 
        else {
            std::memcpy(reinterpret_cast<char*>(arr), m_data + idx, sizeof(T) * sz);
        }
        return *this;
    }

    /**
     * ################################################################
     *              C H A R  S P E C I A L I Z A T I O N
     * ###############################################################
     */

#define MAKE_SPECIALIZATION_ALLOCATORS_LIB_BYTE_BUFFER(type_name__jstd_io_bytebuffer)\
    template<>\
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer v);\
    template<>\
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(std::size_t idx, type_name__jstd_io_bytebuffer v);\
    template<>\
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(const type_name__jstd_io_bytebuffer* arr, std::size_t sz);\
    template<>\
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(std::size_t idx, const type_name__jstd_io_bytebuffer* arr, std::size_t sz);\
    template<>\
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>();\
    template<>\
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>(std::size_t idx) const;\
    template<>\
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer* arr, std::size_t sz);\
    template<>\
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(std::size_t idx, type_name__jstd_io_bytebuffer* arr, std::size_t sz);\

    MAKE_SPECIALIZATION_ALLOCATORS_LIB_BYTE_BUFFER(char)
    MAKE_SPECIALIZATION_ALLOCATORS_LIB_BYTE_BUFFER(signed char)
    MAKE_SPECIALIZATION_ALLOCATORS_LIB_BYTE_BUFFER(unsigned char)

#undef MAKE_SPECIALIZATION_ALLOCATORS_LIB_BYTE_BUFFER
}                                                                                                                                                   

#endif//ALLOCATORS_LANG_IO_BYTEBUFFER_H_