#include <cpp/lang/io/omstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc {
    
    omstream::omstream(std::size_t init_buf_size, tca::allocator* allocator) : 
    _allocator(allocator),
    _buffer(nullptr),
    _capacity(init_buf_size),
    _offset(0) {

    }

    omstream::omstream(char* buf, std::size_t capacity) :
    _allocator(nullptr),
    _buffer(buf),
    _capacity(capacity),
    _offset(0) {

    }

    omstream::omstream(omstream&& stream) : 
    _allocator(stream._allocator), 
    _buffer(stream._buffer), 
    _capacity(stream._capacity), 
    _offset(stream._offset) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
    }

    omstream& omstream::operator= (omstream&& stream) {
        if (&stream != this) {
            close();
            _allocator   = stream._allocator;
            _buffer      = stream._buffer;
            _capacity    = stream._capacity;
            _offset      = stream._offset;
            
            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
        }
        return *this;
    }


    void omstream::write(char c) {
        write(&c, 1);
    }

    void omstream::resize(std::size_t sz) {
        std::size_t s = _capacity == 0 ? INIT_BUF_SIZE : _capacity;
        
        if (s <= 1) s = 2;
        while (sz + _offset > s)
            s = s + (s >> 1);

        char* new_buffer = (char*) _allocator->allocate(s);
        if (!new_buffer)
            throw_except<out_of_memory_error>("Out of memory!");
        
        std::memcpy(new_buffer, _buffer, _offset);
        
        _allocator->deallocate(_buffer);
        _buffer     = new_buffer;
        _capacity   = s;
    }

    void omstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (_allocator == nullptr && _buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        )

        if ((_capacity - _offset < sz) || (_buffer == nullptr))
        {
            if (_allocator == nullptr)
                throw_except<overflow_exception>("omstream buffer is owerflow!");
            resize(sz);
        }
        
        std::memcpy(_buffer + _offset, data, sz);
        _offset += sz;
    }

    void omstream::flush() {
        JSTD_DEBUG_CODE(
            if (_allocator == nullptr && _buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        );
    }

    void omstream::close() {
        if (_buffer == nullptr)
            return;
        if (_allocator != nullptr)
        {
            _allocator->deallocate(_buffer, _capacity);
            _allocator = nullptr;
        }
        _buffer = nullptr;
    }

    omstream::~omstream() {
        
    }

    const char* omstream::data() const {
        return _buffer;
    }
    
    std::size_t omstream::offset() const {
        return _offset;
    }
}