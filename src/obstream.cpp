#include <cpp/lang/io/obstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc {

    obstream::obstream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0), _out(nullptr) {

    }

    obstream::obstream(ostream* stream, tca::base_allocator* allocator, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE(
            if (stream == nullptr)      throw_except<null_pointer_exception>("stream is null!");
            if (allocator == nullptr)   throw_except<null_pointer_exception>("allocator is null!");
        )
        char* data  = (char*) allocator->allocate(buf_size);
        if (!data)
            throw_except<out_of_memory_error>("Out of memory!");
        _allocator  = allocator;
        _buffer     = data;
        _capacity   = buf_size;
        _offset     = 0;
        _out        = stream;
    }
    
    obstream::obstream(ostream* stream, char* buffer, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE(
            if (stream == nullptr)  throw_except<null_pointer_exception>("stream is null!");
            if (buffer == nullptr)  throw_except<null_pointer_exception>("buffer is null!");
        )
        _buffer     = buffer;
        _capacity   = buf_size;
        _offset     = 0;
        _out        = stream;
    }

    obstream::obstream(obstream&& stream) : 
    _allocator(stream._allocator), _buffer(stream._buffer), _capacity(stream._capacity), _offset(stream._offset), _out(stream._out) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
        stream._out         = nullptr;
    }

    obstream& obstream::operator= (obstream&& stream) {
        if (&stream != this) {
            if (_out != nullptr)
                close();
            _allocator  = stream._allocator;
            _buffer     = stream._buffer;
            _capacity   = stream._capacity;
            _offset     = stream._offset;
            _out        = stream._out;

            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
            stream._out         = nullptr;
        }
        return *this;
    }

    void obstream::free() {
        if (_allocator != nullptr) {
            _allocator->deallocate(_buffer, _capacity);
            _allocator = nullptr;
        }
    }

    obstream::~obstream() {
		free();
    }

    void obstream::write(char c) {
        ostream::write(c);
    }
    
    void obstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (_out == nullptr)
                throw_except<io_exception>("stream is null!");
        )
        assert(_capacity >= _offset);
        std::size_t rem = _capacity - _offset;
        
        if (rem < sz)
        {
            flush();
        }
        
        if (sz > _capacity)
        {
            _out->write(data, sz);
        } 
        else
        {
            memcpy(_buffer + _offset, data, sz);
            _offset += sz;
        }
    }
    
    void obstream::flush() {
        JSTD_DEBUG_CODE(
            if (_out == nullptr)
                throw_except<io_exception>("stream is null!");
        )
        if (_offset > 0)
        {
            _out->write(_buffer, _offset);
            _offset = 0;
        }
    }
    
    void obstream::close() {
        if (_out == nullptr) 
            return;
        try {
            flush();
        } catch (...) {
            try {
                _out->close();
            } catch (const io_exception& close_except) {}
            _out = nullptr;
            free();
            throw;    
        }
        
        free();

        try {
            _out->close();
            _out = nullptr;
        } catch (const io_exception& e) {
            _out = nullptr;
            throw e;
        }
    }
    
}