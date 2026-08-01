#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/exceptions.hpp>

namespace tc {
    
    void close_stream_and_suppress_except(class istream* in) {
        try {
            in->close();
        } catch (const io_exception& e) {
            //ignore
        }
    }

    void close_stream_and_suppress_except(class ostream* out) {
        try {
            out->close();
        } catch (const io_exception& e) {
            //ignore
        }
    }

    std::size_t istream::skip(std::size_t n) {
        std::size_t skipped = 0;
        while (n > 0) {
            int readed = read();
            if (readed == -1)
                break;
            ++skipped;
            --n;
        }
        return skipped;
    }

    istream::~istream() {

    }

    ostream::~ostream() {
        
    }

    void ostream::write(char c) {
        write(&c, 1);
    }

    int istream::read() {
        char c;
        std::size_t readed = read(&c, 1);
        if (readed == istream::eof_value())
            return -1;
        return c & 0xFF;
    }
} 