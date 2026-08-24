#include <cpp/lang/io/ofstream.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <internal/io/io_helpers.hpp>

namespace tc {

    ofstream::ofstream() : m_handle(nullptr) {

    }

    
    ofstream::ofstream(const string& path, bool append) : ofstream(path.c_str(), append) {
    
    }

    ofstream::ofstream(const char* path, bool append) : ofstream(file(path), append) {
        
    }
    
    ofstream::ofstream(const file& f, bool append) : ofstream() {    
        expected<FILE*, error_code> fhandle = filesystem::open(f.c_str(), append ? "ab" : "wb");
        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }
        m_handle = fhandle.value();
    }

    ofstream::ofstream(ofstream&& stream) : m_handle(stream.m_handle) {
        stream.m_handle = nullptr;
    }
    
    ofstream& ofstream::operator= (ofstream&& out) {
        if (&out != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle     = out.m_handle;
            out.m_handle = nullptr;
        }
        return *this;
    }
    
    ofstream::~ofstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    void ofstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("stream is null");
        )
        std::size_t writed = fwrite(data, 1, sz, m_handle);
        if (writed != sz)
        {
            throw_except<io_exception>(strerror(errno));
        }
    }
    
    void ofstream::flush() {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("stream is null");
        )
        error_code err = filesystem::flush(m_handle);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }
    
    void ofstream::close(error_code& err) {
        if (m_handle)
		{
            err      = filesystem::close(m_handle);
            m_handle = nullptr;
        }
    }    
}