#include <tc/internal/allocators/malloc_free_allocator.hpp>
#include <cstdlib>
#include <cstddef>
#include <malloc.h>

namespace tca {
    
    malloc_free_allocator::malloc_free_allocator() : allocator(nullptr) {

    }

    void* malloc_free_allocator::allocate(std::size_t sz) {
        return malloc(sz);
    }
    
    void* malloc_free_allocator::reallocate(void* p, std::size_t new_size) {
        return realloc(p, new_size);
    }

    void* malloc_free_allocator::allocate_align(std::size_t sz, std::size_t) {
        return malloc(sz);
    }
    
    void malloc_free_allocator::deallocate(void* p) {
        free(p);
    }
}