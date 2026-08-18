#include <cpp/lang/io/basebuf.hpp>

namespace tc
{

    string basebuf::to_string(tca::allocator* alloc) const {
        string result(alloc);

        result
        .append("[position=").append(tc::to_string(m_position, alloc))
        .append(", limit=").append(tc::to_string(m_limit, alloc))
        .append(", capacity=").append(tc::to_string(m_capacity, alloc))
        .append(']');

        return result;
    }

} //namespace tc