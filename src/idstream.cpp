#include <cpp/lang/io/idstream.hpp>
#include <iostream>

namespace tc {

    idstream::idstream() : m_in(nullptr) {
        
    }

    idstream::idstream(istream* in) : m_in(in) {
        if (in == nullptr)
            throw_except<null_pointer_exception>("in is null");
    }
    
    idstream::idstream(idstream&& stream) : m_in(stream.m_in) {
        stream.m_in = nullptr;
    }

    idstream& idstream::operator= (idstream&& stream) {
        if (&stream != this)
        {
            if (m_in)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_in         = stream.m_in;
            stream.m_in  = nullptr;
        }
        return *this;
    }

    idstream::~idstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    std::size_t idstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (!m_in)
                throw_except<io_exception>("stream is null");
        )
        return m_in->read(buf, sz);
    }

    void idstream::close(error_code& err) {
        if (m_in == nullptr)
            return;
        m_in->close(err);
        m_in = nullptr;
    }
}