#include <internal/io/io_helpers.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <allocators/inline_linear_allocator.hpp>

namespace tc
{
namespace internal
{
namespace io
{
    
    void throw_error_code(const error_code& err) {
        tca::inline_linear_allocator<1024> alloc;
        tc::string msg = err.get_message(&alloc);
        if (err == error_condition(errcode::permission_denied, generic_category()))
        {
            throw_except<security_exception>("%s", msg.c_str());    
        }
        throw_except<io_exception>("%s", msg.c_str());
    }

} //namespace io
} //namespace internal
} //namespace tc