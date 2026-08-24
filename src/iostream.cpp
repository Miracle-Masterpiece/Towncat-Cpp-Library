#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <internal/io/io_helpers.hpp>

namespace tc
{
    std::size_t istream::skip(std::size_t n) {
        std::size_t skipped = 0;
        while (n > 0)
        {
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

    void istream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }

    void ostream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }
} 