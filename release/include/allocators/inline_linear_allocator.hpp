#ifndef ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H
#define ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H

#include <allocators/linear_allocator.hpp>
#include <cstddef>

namespace tca {

template<std::size_t SIZE, typename ALIGN_T = std::max_align_t>
class inline_linear_allocator : public linear_allocator {
    
    /**
     * 
     */
    inline_linear_allocator(const inline_linear_allocator<SIZE, ALIGN_T>&) = delete;
    
    /**
     * 
     */
    inline_linear_allocator<SIZE, ALIGN_T>& operator=(const inline_linear_allocator<SIZE, ALIGN_T>&) = delete;
    
    /**
     * 
     */
    inline_linear_allocator(inline_linear_allocator<SIZE, ALIGN_T>&&) = delete;
    
    /**
     * 
     */
    inline_linear_allocator<SIZE, ALIGN_T>& operator=(inline_linear_allocator<SIZE, ALIGN_T>&&) = delete;
public:
    /**
     * 
     */    
    union {
        /**
         * 
         */
        char m_buf[SIZE];
        
        /**
         * 
         */
        ALIGN_T m_dummy;
    } m_data;
    
    /**
     * 
     */
    inline_linear_allocator();
    
    /**
     * 
     */
    ~inline_linear_allocator();
};
    
    template<std::size_t SIZE, typename ALIGN_T>
    inline_linear_allocator<SIZE, ALIGN_T>::inline_linear_allocator() : linear_allocator(m_data.m_buf, SIZE) {

    }
    
    template<std::size_t SIZE, typename ALIGN_T>
    inline_linear_allocator<SIZE, ALIGN_T>::~inline_linear_allocator() {

    }
}
#endif//ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H