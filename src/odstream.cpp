#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/exceptions.hpp>

namespace tc
{
    odstream::odstream() : m_out(nullptr) {

    }

    odstream::odstream(ostream* out) : m_out(out) {
        JSTD_DEBUG_CODE
        (
            if (!out)
                throw_except<io_exception>("stream is null");
        )
    }

    odstream::odstream(odstream&& stream) : m_out(stream.m_out) {
        stream.m_out = nullptr;
    }
    
    odstream& odstream::operator= (odstream&& out) {
        if (&out != this)
        {
            if (m_out)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_out     = out.m_out;
            out.m_out = nullptr;
        }
        return *this;
    }
    
    odstream::~odstream() {
        error_code dontcare;
        close(dontcare);
    }

    void odstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_out)
                throw_except<io_exception>("stream is null");
        )
        m_out->write(data, sz);
    }
    
    void odstream::flush() {
        if (m_out)
        {
            m_out->flush();
        }
    }
    
    void odstream::close(error_code& err) {
        if (m_out)
        {
            m_out->close(err);
            m_out = nullptr;
        }
    }
}