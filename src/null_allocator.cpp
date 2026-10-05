#include <tc/internal/allocators/null_allocator.hpp>
#include <utility>
#include <cassert>

namespace tca
{
    null_allocator::null_allocator() : base_allocator() {

    }

    null_allocator::null_allocator(null_allocator&& alloc) : base_allocator(std::move(alloc)) {

    }
    
    null_allocator& null_allocator::operator= (null_allocator&& alloc) {
        if (&alloc != this)
            base_allocator::operator=(std::move(alloc));
        return *this;
    }
    
    null_allocator::~null_allocator() {

    }
    
    void* null_allocator::allocate(std::size_t){
        return nullptr;
    }
    
    void* null_allocator::allocate_align(std::size_t, std::size_t) {
        return nullptr;
    }
    
    void null_allocator::deallocate(void* p, std::size_t) {
        assert(p == nullptr);
    }
}