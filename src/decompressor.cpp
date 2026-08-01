#include <cpp/lang/compress/decompressor.hpp>


namespace tc
{

    decompressor::decompressor() : input(nullptr), input_size(0) {

    }

    decompressor::decompressor(decompressor&& other) : input(other.input), input_size(other.input_size) {
        other.input         = nullptr;
        other.input_size    = 0;
    }
        
    decompressor& decompressor::operator=(decompressor&& other) {
        if (&other != this)
        {
            input       = other.input;
            input_size  = other.input_size;
            other.input         = nullptr;
            other.input_size    = 0;
        }
        return *this;
    }
        
    void decompressor::set_input(const char* in, std::size_t length) {
        input       = in;
        input_size  = length;
    }
    
    decompressor::~decompressor() {

    }
}
