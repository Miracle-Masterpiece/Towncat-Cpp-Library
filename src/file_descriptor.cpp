#include <internal/io/file_descriptor.hpp>

#include <allocators/inline_linear_allocator.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/utils/coder.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/io/constants.hpp>
#include <errno.h>
#include <cstring>
#include <internal/io/filesystem.hpp>

#if defined(JSTD_OS_WINDOWS)
# include <windows.h>
# include <io.h>
# include <sys/fcntl.h>
# include <sys/types.h>
# define JSTD_WIN_CODE(code) code
# define JSTD_POSIX_CODE(code)
#elif defined(JSTD_OS_UNIX) || defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
# define JSTD_WIN_CODE(code)    
# define JSTD_POSIX_CODE(code) code
#endif

namespace tc
{
namespace internal
{
    const file_descriptor file_descriptor::NULL_FD(-1);

    file_descriptor::file_descriptor(int fd) : fd(fd) {

    }

    file_descriptor::file_descriptor() : file_descriptor(NULL_FD.fd) {

    }

    file_descriptor::file_descriptor(file_descriptor&& fdesc) : fd(fdesc.fd) {
        fdesc.fd = NULL_FD.fd;
    }
    
    file_descriptor::operator bool() const {
        return *this != NULL_FD;
    }

    file_descriptor& file_descriptor::operator=(file_descriptor&& fdesc) {
        if (&fdesc != this) 
        {
            close();
            fd          = fdesc.fd;
            fdesc.fd    = NULL_FD.fd;
        }
        return *this;
    }
    
    file_descriptor::file_descriptor(const char* u8path, int oflags, int pmode) {
        fd = filesystem::open_fd(u8path, oflags, pmode);
    }
    
    void file_descriptor::close() {
        if (*this != NULL_FD)
        {
            filesystem::close_fd(fd);
        }
    }

    file_descriptor::~file_descriptor() {
        
    }
    
    int file_descriptor::get_fd() const {
        return fd;
    }

    bool file_descriptor::operator==(const file_descriptor& fdesc) const {
        return fd == fdesc.fd;
    }

    bool file_descriptor::operator!=(const file_descriptor& fdesc) const {
        return fd != fdesc.fd;
    }
}

}