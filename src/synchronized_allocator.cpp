#include <allocators/synchronized_allocator.hpp>

namespace tca
{

    synchronized_allocator::synchronized_allocator(allocator* alloc) : m_alloc(alloc), m_mutex() {
    
    }
    
    synchronized_allocator::synchronized_allocator(synchronized_allocator&& a) : m_alloc(a.m_alloc), m_mutex() {
        
    }
    
    synchronized_allocator& synchronized_allocator::operator= (synchronized_allocator&& a) {
        m_alloc = a.m_alloc;
        return *this;
    }
    
    void* synchronized_allocator::allocate(std::size_t sz) {
        tc::unique_lock lock(m_mutex);
        return m_alloc->allocate(sz);
    }
    
    void* synchronized_allocator::allocate_align(std::size_t sz, std::size_t align) {
        tc::unique_lock lock(m_mutex);
        return m_alloc->allocate_align(sz, align);
    }
    
    void synchronized_allocator::deallocate(void* p) {
        tc::unique_lock lock(m_mutex);
        m_alloc->deallocate(p);
    }
    
    void synchronized_allocator::deallocate(void* p, std::size_t) {
        tc::unique_lock lock(m_mutex);
        m_alloc->deallocate(p);
    }

}