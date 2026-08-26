#ifndef _ALLOCATORS_NULL_ALLOCATOR_H
#define _ALLOCATORS_NULL_ALLOCATOR_H

#include <allocators/base_allocator.hpp>

namespace tca {

/**
 * Null-распределитель
 * Выделяет только указатели с null значением.
 */
class null_allocator : public base_allocator {
public:
    
    /**
     * Вызывает стандартный конструтор по-умолчанию.
     */
    null_allocator();
    
    /**
     * 
     */
    null_allocator(null_allocator&&);
    
    /**
     * 
     */
    null_allocator& operator= (null_allocator&&);
    
    /**
     */
    ~null_allocator();
    
    /**
     * Аллоцирует null-указатель.
     */
    void* allocate(std::size_t) override;
    
    /**
     * Аллоцирует null-указатель.
     */
    void* allocate_align(std::size_t, std::size_t) override;
    
    /**
     * Освобождает null-указатель.
     */
    void deallocate(void*, std::size_t) override;
};

}

#endif//_ALLOCATORS_NULL_ALLOCATOR_H