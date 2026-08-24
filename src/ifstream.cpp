#include <cpp/lang/io/ifstream.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace tc
{

    ifstream::ifstream() : istream(), m_handle(nullptr) {

    }

    ifstream::ifstream(const char* path) : ifstream(file(path)) {

    }
    
    ifstream::ifstream(const string& path) : ifstream(path.c_str()) {
    
    }

    ifstream::ifstream(const file& file) : m_handle(nullptr) {
        expected<FILE*, error_code> fhandle = filesystem::open(file.c_str(), "rb");

        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }

        m_handle = fhandle.value();
    }
    
    ifstream::ifstream(ifstream&& stream) : istream(), m_handle(stream.m_handle) {
        stream.m_handle      = nullptr;
    }
    
    ifstream& ifstream::operator= (ifstream&& stream) {
        if (&stream != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle        = stream.m_handle;
            stream.m_handle = nullptr;
        }
        return *this;
    }
    
    void ifstream::close(error_code& err) {
        if (m_handle == nullptr)
            return;
        err = filesystem::close(m_handle);
        m_handle     = nullptr;
    }
    
    ifstream::~ifstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    std::size_t ifstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (m_handle == nullptr)
                throw_except<io_exception>("file stream not open")
        );
        std::size_t readed = fread(buf, 1, sz, m_handle);   
        if (readed == 0)
        {
            if (feof(m_handle))
            {
                return istream::eof_value();
            }
            if (ferror(m_handle))
            {
                throw_except<io_exception>(strerror(errno));
            }
        }
        return readed;
    }
}