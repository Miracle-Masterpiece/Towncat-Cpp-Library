#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace tc {

    odstream::odstream() : _out(nullptr) {

    }

    odstream::odstream(ostream* out) : _out(out) {
        if (out == nullptr)
            throw_except<null_pointer_exception>("stream is null!");
    }

    odstream::odstream(odstream&& stream) : _out(stream._out) {
        stream._out = nullptr;
    }
    
    odstream& odstream::operator= (odstream&& stream) {
        if (&stream != this) {
            if (_out != nullptr)
                close();
            _out        = stream._out;
            stream._out = nullptr;
        }
        return *this;
    }
    
    odstream::~odstream() {

    }

    void odstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (_out == nullptr)
                throw_except<io_exception>("Stream is null!");
        )
        _out->write(data, sz);
    }
    
    void odstream::flush() {
        JSTD_DEBUG_CODE(
            if (_out == nullptr)
                throw_except<io_exception>("Stream is null!");
        )
        _out->flush();
    }
    
    void odstream::close() {
        JSTD_DEBUG_CODE(
            if (_out == nullptr)
                throw_except<io_exception>("Stream is null!");
        )
        try {
            _out->close();
            _out = nullptr;
        } catch (...) {
            _out = nullptr;
            throw;
        }
    }
}