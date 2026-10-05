#include <tc/internal/cpp/lang/compress/compressor.hpp>

    const char* compressor::get_input() const {
        return input;
    }

    std::size_t compressor::get_size() const {
        return input_size;
    }

    void compressor::finish() {
        finished = true;
    }

    compressor::compressor() : input(nullptr), input_size(0) {

    }
    
    compressor::compressor(compressor&& other) {

    }
    
    compressor& compressor::operator= (compressor&& other) {
        if (&other != this)
        {
            input       = other.input;
            input_size  = other.input_size; 
        }
        return *this;
    }
    
    void compressor::set_input(const char* data, std::size_t len) {
        input       = data;
        input_size  = len;
    }
    
    bool compressor::is_finished() const {
        return finished;
    }

    compressor::~compressor() {

    }