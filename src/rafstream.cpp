#include <cpp/lang/io/rafstream.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>
#include <cpp/lang/io/basebuf.hpp>
#include <cstdio>
#include <cerrno>

#if defined(JSTD_OS_WINDOWS)
# include <io.h>
# include <sys/stat.h>
# define STAT_STRUCT struct _stat
# define STAT_FUNC(fd, ptr) _fstat(fd, ptr)
# define FTRUNCATE(fd, size) _chsize_s(fd, size)
#elif defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
# include <sys/stat.h>
# include <unistd.h>
# define STAT_STRUCT struct stat
# define STAT_FUNC(fd, ptr) fstat(fd, ptr)
# define FTRUNCATE(fd, size) ftruncate(fd, size)
#endif

namespace tc
{

    typedef typename int_of<32>::utype len_type;

    static void fill_stat(int fd, STAT_STRUCT* s) {
        if (STAT_FUNC(fd, s) != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }

    static int FILE_to_fd(FILE* f) {
        int fd = fileno(f);
        if (fd == -1)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
        return fd;  
    } 
}

namespace tc
{

    rafstream::rafstream() : m_handle(nullptr) {
    
    }
    
    rafstream::rafstream(const char* f, const char* m) : rafstream(file(f), m) {
        
    }
    
    rafstream::rafstream(const file& file, const char* mode) {
        
        if (strcmp(mode, "rw") == 0)
        {
            mode = "w+b";
        }
        else if (strcmp(mode, "r") == 0)
        {
            mode = "rb";
        }
        else if (strcmp(mode, "w") == 0)
        {
            mode = "wb";
        }
        
        expected<FILE*, error_code> fhandle = filesystem::open(file.c_str(), mode);
        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }

        m_handle = static_cast<void*>(fhandle.value());
    }
    
    rafstream::rafstream(const file& file, const string& mode) : rafstream(file, mode.c_str()) {
    
    }
    
    rafstream::rafstream(const string& file, const string& mode) : rafstream(file.c_str(), mode.c_str()) {
    
    }
    
    rafstream::rafstream(rafstream&& s) : m_handle(s.m_handle) {
        s.m_handle = nullptr;
    } 

    rafstream& rafstream::operator=(rafstream&& s) {
        if (&s != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle    = s.m_handle;
            s.m_handle  = nullptr;
        }
        return *this;
    }
    
    rafstream::~rafstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    void rafstream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }
    
    void rafstream::close(error_code& err) {
        if (m_handle)
        {
            err = filesystem::close(static_cast<FILE*>(m_handle));
            m_handle = nullptr;
        }
    }
    
    void rafstream::set_length(std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int fd = FILE_to_fd(static_cast<FILE*>(m_handle));
        if (FTRUNCATE(fd, len) != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    std::size_t rafstream::length() {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int fd = FILE_to_fd(static_cast<FILE*>(m_handle));
        STAT_STRUCT stat;
        fill_stat(fd, &stat);
        return static_cast<std::size_t>(stat.st_size);
    }
    
    void rafstream::seek(std::size_t pos) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int err = std::fseek(static_cast<FILE*>(m_handle), static_cast<long>(pos), SEEK_SET);
        if (err != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    std::size_t rafstream::get_file_pointer() {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        long pos = std::ftell(static_cast<FILE*>(m_handle));
        if (pos == -1L)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
        return static_cast<std::size_t>(static_cast<unsigned long>(pos));
    }
    
    void rafstream::skip_bytes(std::size_t n) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        const std::size_t BUF_SIZE = 128;
        char buf[BUF_SIZE];
        while (n)
        {
            std::size_t need_skip   = math::min(BUF_SIZE, n);
            std::size_t readed      = read(buf, need_skip);
            if (readed < need_skip)
                break;
            n -= readed;
        }
    }
    
    void rafstream::write(const char* arr, std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        std::size_t writed = fwrite(arr, sizeof(char), len, static_cast<FILE*>(m_handle));
        if (writed != len)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    void rafstream::write(char ch) {
        write(&ch, 1);
    }
    
    std::size_t rafstream::read(char buf[], std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        FILE* handle = static_cast<FILE*>(m_handle);
        
        std::size_t readed = std::fread(buf, sizeof(char), len, handle);
        if (readed == 0)
        {
            if (std::feof(handle))
            {
                return eof_value();
            }
            else if (std::ferror(handle))
            {
                internal::io::throw_error_code(error_code(errno, generic_category()));
            }
        }

        return readed;
    }
    
    int rafstream::read() {
        char c;
        std::size_t r = read(&c, 1);
        if (r == 0)
            return -1;
        return c & 0xFF;
    }

    void rafstream::write_string(const string& x) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        len_type len = static_cast<len_type>(x.length());
        write<len_type>(len);
        write(x.c_str(), len);
    }

    string rafstream::read_string(tca::allocator* alloc) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        string result(alloc);

        len_type len = read<len_type>();
        
        result.set_length(len);
        
        std::size_t readed = read(result.c_str(), len);
    
        if (readed != len)
        {
            throw_except<eof_exception>("cannot read type");
        }

        result.c_str()[len] = 0;

        return result;
    }

    void rafstream::force(bool metadata) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )

        if (std::fflush(static_cast<FILE*>(m_handle)) != 0)
        {
            internal::io::throw_error_code( error_code(errno, generic_category()) );
        }

        #if 1
        JSTD_WIN_CODE
        (
            HANDLE fhandle = reinterpret_cast<HANDLE>(
                _get_osfhandle(fileno( static_cast<FILE*>(m_handle) ))
            );
            
            if (!FlushFileBuffers(fhandle))
            {
                internal::io::throw_error_code(error_code(GetLastError(), system_category()));
            }
        )

        JSTD_UNIX_CODE
        (
            int fhandle = fileno(static_cast<FILE*>(m_handle));
            int err  = 0;
            
            if (metadata) err = fsync(fhandle);
            else          err = fdatasync(fhandle);
            
            if (err)
            {
                internal::io::throw_error_code(errno, generic_category());
            }
        )
        #endif
    }
}