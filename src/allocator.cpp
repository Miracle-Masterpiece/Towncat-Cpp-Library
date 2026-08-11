#include <allocators/allocator.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <utility>

namespace tca
{    
    allocator::allocator() : base_allocator() {

    }

    allocator::allocator(base_allocator* parent) : base_allocator(parent) {

    }

    allocator::allocator(allocator&& a) : base_allocator(std::move(a)) {
        
    }

    allocator& allocator::operator= (allocator&& a) {
        if (&a != this) {
            base_allocator::operator=(std::move(a));
        }
        return *this;
    }

    void allocator::deallocate(void* p, std::size_t) {
        deallocate(p);
    }

    allocator::~allocator() {

    }
}


namespace tca
{
    
namespace internal 
{
    thread_local allocator* scoped_allocator;
}
    
    static malloc_free_allocator malloc_allocator;
    
    allocator* default_alloc = &malloc_allocator;

    allocator* get_default_allocator() {
        return default_alloc;
    }

    void set_default_allocator(allocator* allocator) {
        default_alloc = 
                        (allocator != nullptr) ? allocator : &malloc_allocator;
    }

    allocator* get_scoped_or_default() {
        return internal::scoped_allocator ? internal::scoped_allocator : get_default_allocator();
    }

    allocator* get_exception_allocator() {
        return get_default_allocator();
    }
}

namespace tca
{
    scope_allocator::scope_allocator(allocator* allocator) : m_prev(internal::scoped_allocator) {
        internal::scoped_allocator = allocator;
    }
    
    scope_allocator::~scope_allocator() {
        if (internal::scoped_allocator != nullptr) {
            internal::scoped_allocator = m_prev;
        }
    }
    
    allocator* scope_allocator::get_prev() const {
        return m_prev;
    }
}