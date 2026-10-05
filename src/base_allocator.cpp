#include <tc/internal/allocators/malloc_free_allocator.hpp>
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace tca{

    base_allocator::base_allocator() : base_allocator(nullptr) {

    }

    base_allocator::base_allocator(base_allocator* parent) : m_parent(parent) {
        
    }

    base_allocator::base_allocator(base_allocator&& base) : m_parent(base.m_parent) {
        base.m_parent = nullptr;
    }

    base_allocator& base_allocator::operator= (base_allocator&& base) {
        if (&base != this)
        {
            m_parent      = base.m_parent;
            base.m_parent = nullptr;
        }
        return *this;
    }

    base_allocator::~base_allocator() {

    }

    void* base_allocator::allocate(std::size_t sz) {
        assert(m_parent != nullptr);
        return m_parent->allocate(sz);
    }

    void* base_allocator::allocate_align(std::size_t sz, std::size_t align) {
        assert(m_parent != nullptr);
        return m_parent->allocate_align(sz, align);
    }

    void base_allocator::deallocate(void* p, std::size_t sz) {
        assert(m_parent != nullptr);
        m_parent->deallocate(p, sz);
    }
}