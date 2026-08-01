#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc {

    ibstream::ibstream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0), _limit(0), _in(nullptr) {

    }

    ibstream::ibstream(istream* stream, std::size_t buf_size, tca::allocator* allocator) : ibstream() {
        char* data = (char*) allocator->allocate(buf_size);
        _allocator  = allocator;
        _buffer     = data;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(istream* stream, char* buf, std::size_t buf_size) : ibstream() {
        _buffer     = buf;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(ibstream&& stream) : 
    _allocator(stream._allocator), 
    _buffer(stream._buffer),
    _capacity(stream._capacity),
    _offset(stream._offset),
    _limit(stream._limit),
    _in(stream._in) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
        stream._limit       = 0;
        stream._in          = nullptr;
    }
    
    ibstream& ibstream::operator= (ibstream&& stream) {
        if (&stream != this) {
            if (_in != nullptr)
                close();
            _allocator   = stream._allocator;
            _buffer      = stream._buffer;
            _capacity    = stream._capacity;
            _offset      = stream._offset;
            _limit       = stream._limit;
            _in          = stream._in;
            
            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
            stream._limit       = 0;
            stream._in          = nullptr;
        }
        return *this;
    }
    
    int ibstream::read() {
        return istream::read();
    }
    
    void ibstream::fill_buffer() {
        std::size_t readed = _in->read(_buffer, _capacity);
        _limit  = (readed == istream::eof_value()) ? 0 : readed;
        _offset = 0;
    }

    std::size_t ibstream::read(char* buf, std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (_in == nullptr) throw_except<io_exception>("input is null")
        );
        
        std::size_t total_readed = 0;
        
        while (total_readed < sz)
        {
            if (_offset >= _limit)
            {
                fill_buffer();
                if (_limit == 0)
                    break; // EOF
            }
            
            std::size_t available = _limit - _offset;
            std::size_t to_read = std::min(available, sz - total_readed);
            
            if (to_read > 0)
            {
                memcpy(buf + total_readed, _buffer + _offset, to_read);
                _offset += to_read;
                total_readed += to_read;
            }
        }
        
        return total_readed > 0 ? 
                                    total_readed : istream::eof_value();
    }
   
    std::size_t ibstream::skip(std::size_t n) {
        JSTD_DEBUG_CODE(
            if (_in == nullptr)
                throw_except<io_exception>("Stream is null");
        );
        return istream::skip(n);
    }
    
    std::uintmax_t ibstream::available() const {
        assert(_limit >= _offset);
        std::size_t available_in_buffer = _limit - _offset;
        return available_in_buffer + _in->available();
    }
    
    void ibstream::free() {
        if (_allocator != nullptr)
        {
            _allocator->deallocate(_buffer, _capacity);
            _allocator  = nullptr;
        }
    }

    ibstream::~ibstream() {
		free();
    }

    void ibstream::close() {
        if (_in == nullptr)
            return;
        try
        {
            _in->close();
        } catch (const io_exception& e) {
            _in = nullptr;
            free();
            throw e;    
        }
        _in = nullptr;
        free();
    }    
}