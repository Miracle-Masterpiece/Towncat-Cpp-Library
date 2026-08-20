#include <cpp/lang/io/ifstream.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace tc {

    ifstream::ifstream() : istream(), m_handle(nullptr), m_available(0) {

    }

    ifstream::ifstream(const char* path) : ifstream(file(path)) {

    }
    
    ifstream::ifstream(const file& file) : m_handle(nullptr), m_available(0) {
        //функция filesystem::open уже кидает нужные исключения.
        m_handle = filesystem::open(file.c_str(), "rb");
        try {
            m_available = filesystem::length(file.c_str());
        } catch (const io_exception& except) {
            try {
                filesystem::close(m_handle);
            } catch (const throwable& dontCare) {

            }
            throw except;
        }
    }
    
    ifstream::ifstream(ifstream&& stream) : istream(), m_handle(stream.m_handle), m_available(stream.m_available) {
        stream.m_handle      = nullptr;
        stream.m_available   = 0;
    }
    
    ifstream& ifstream::operator= (ifstream&& stream) {
        if (&stream != this) {
            if (m_handle != nullptr)
                close();
            m_handle     = stream.m_handle;
            m_available  = stream.m_available;
            stream.m_handle      = nullptr;
            stream.m_available   = 0;
        }
        return *this;
    }
    
    void ifstream::close() {
        if (m_handle == nullptr) return;
        try {
            filesystem::close(m_handle);
        } catch (const io_exception& e) {
            m_handle     = nullptr;
            m_available  = 0;
            throw e;
        }
        m_handle     = nullptr;
        m_available  = 0;
    }
    
    ifstream::~ifstream() {
        
    }
    
    int ifstream::read() {
        return istream::read();
    }
    
    std::size_t ifstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (m_handle == nullptr)
                throw_except<io_exception>("File stream is null!")
        );
        std::size_t readed = fread(buf, 1, sz, m_handle);   
        if (readed == 0) {
            if (feof(m_handle))
                return istream::eof_value();
            if (ferror(m_handle))
                throw_except<io_exception>(strerror(errno));
        }
        m_available -= readed;
        return readed;
    }
    
    std::size_t ifstream::skip(std::size_t n) {
        JSTD_DEBUG_CODE(
            if (m_handle == nullptr)
                throw_except<io_exception>("File stream is null!");
        );
        return istream::skip(n);
    }
    
    std::uintmax_t ifstream::available() const {
        JSTD_DEBUG_CODE(
            if (m_handle == nullptr)
                throw_except<io_exception>("File stream is null!");
        );
        return m_available;
    }

}