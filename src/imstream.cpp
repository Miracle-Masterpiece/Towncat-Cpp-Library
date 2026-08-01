#include <cpp/lang/io/imstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace tc {


    imstream::imstream() : _buffer(nullptr), _capacity(0), _offset(0) {

    }

    imstream::imstream(const char* buffer, std::size_t capacity) : _buffer(buffer), _capacity(capacity), _offset(0) {
        if (buffer == nullptr)
            throw_except<null_pointer_exception>("buffer is null");
    }
    
    imstream::imstream(imstream&& stream) : _buffer(stream._buffer), _capacity(stream._capacity), _offset(stream._offset) {
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
    }
    
    imstream& imstream::operator= (imstream&& stream) {
        if (&stream != this) {
            if (_buffer != nullptr)
                close();
            _buffer     = stream._buffer;
            _capacity   = stream._capacity;
            _offset     = stream._offset;

            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
        }
        return *this;
    }
    
    imstream::~imstream() {

    }
    
    int imstream::read() {
        return istream::read();
    }
    
    std::size_t imstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (_buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        );
        sz = std::min(_capacity - _offset, sz);
        if (sz == 0)
            return istream::eof_value();

        std::memcpy(buf, _buffer + _offset, sz);
        _offset += sz;
        
        return sz;
    }
    
    std::uintmax_t imstream::available() const {
        JSTD_DEBUG_CODE(
            if (_buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        )
        return _capacity - _offset;
    }
    
    void imstream::close() {
        _buffer = nullptr;
    }
    
    void imstream::reset() {
        _offset = 0;
    }
}