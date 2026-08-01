#include <allocators/linear_allocator.hpp>
#include <allocators/Helpers.hpp>
#include <exception>
#include <cstdio>
#include <cstddef>

namespace tca
{

    linear_allocator::linear_allocator() : 
    allocator(nullptr),
    _buffer(nullptr),
    _capacity(0),
    _offset(0) {

    }

    linear_allocator::linear_allocator(void* baseBuffer, std::size_t capacity) : 
    allocator(nullptr),
    _buffer(baseBuffer), 
    _capacity(capacity), 
    _offset(0) {

    }

    linear_allocator::linear_allocator(std::size_t capacity, base_allocator* baseAllocator) : 
    allocator(baseAllocator), 
    _buffer(nullptr), 
    _capacity(capacity),
    _offset(0) {
        _buffer = m_parent->allocate_align(capacity, alignof(std::max_align_t));
    }

    linear_allocator::linear_allocator(linear_allocator&& alloc) : 
    allocator(std::move(alloc)), 
    _buffer(alloc._buffer), 
    _capacity(alloc._capacity), 
    _offset(alloc._offset) {
        alloc._buffer   = nullptr;
        alloc._capacity = 0;
        alloc._offset   = 0;
    }
    
    linear_allocator& linear_allocator::operator= (linear_allocator&& alloc) {
        if (&alloc != this) {
            if (m_parent != nullptr)
                m_parent->deallocate(_buffer, _capacity);
    
            base_allocator::operator=(std::move(alloc));

            _buffer     = alloc._buffer;
            _capacity   = alloc._capacity;
            _offset     = alloc._offset;
            alloc._buffer   = nullptr;
            alloc._capacity = 0;
            alloc._offset   = 0;

        }
        return *this;
    }

    linear_allocator::~linear_allocator() {
        if (m_parent != nullptr)
            m_parent->deallocate(_buffer, _capacity);
    }
    
    void* linear_allocator::allocate(std::size_t sz) {
        if ((_offset + sz > _capacity) || _buffer == nullptr){
            return nullptr;
        }
        char* buf = ((char*) _buffer) + _offset;
        _offset += sz;
        return (void*) buf;
    }

    void* linear_allocator::allocate_align(std::size_t sz, std::size_t align) {
        std::size_t off = align_up(_offset, align);
        if ((_offset + off + sz > _capacity) || _buffer == nullptr)
            return nullptr;
        _offset += off;
        char* buf = ((char*) _buffer) + _offset;
        _offset += sz;
        return (void*) buf;
    }
    
    void linear_allocator::deallocate(void*) {
    
    }

    int linear_allocator::to_string(char buf[], std::size_t buf_size) const {
        return snprintf(buf, buf_size, "[size %zu, offset %zu, free %zu]", _capacity, _offset, _capacity - _offset);
    }

    void linear_allocator::print() const {
        char strbuf[128];
        to_string(strbuf, sizeof(strbuf));
        std::printf("%s\n", strbuf);
    }
}