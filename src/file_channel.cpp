#include <cpp/lang/io/file_channel.hpp>
#include <cpp/lang/exceptions.hpp>

#include <cassert>
#include <unistd.h>

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
# include <sys/mman.h>
# include <unistd.h>
# include <cerrno>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# define JSTD_POSIX_CODE(code) code
#else
# define JSTD_POSIX_CODE(code)
#endif

#if defined(JSTD_OS_WINDOWS)
# include <io.h>
# include <windows.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <cerrno>
# define ftruncate(fd, sz) _chsize_s(fd, sz)
#endif


namespace tc
{
    namespace internal
    {
        /**
         * Возвращает выравнивание страниц памяти отображённой памяти.
         */
        std::size_t get_page_size();
    }
}

/**
 * ##########################################################################################
 *                               M A P P E D _ F I L E
 * ##########################################################################################
 */
namespace tc 
{

    file_channel::file_channel() : m_fd(), m_oflags(ZERO_FLAG) {

    }

    file_channel::file_channel(const tc::file& path, int oflags) : m_fd(), m_oflags(oflags) {
        open(path, oflags);
    }

    file_channel::file_channel(file_channel&& f) : m_fd(std::move(f.m_fd)), m_oflags(f.m_oflags) {
        f.m_oflags = ZERO_FLAG;
    }
    
    file_channel& file_channel::operator= (file_channel&& f) {
        if (&f != this)
            m_fd = std::move(f.m_fd);
        return *this;
    }

    file_channel::~file_channel() {

    }

    void file_channel::close() {
        if (m_fd)
            m_fd.close();
    }

    void file_channel::require_fd() const {
        JSTD_DEBUG_CODE(
            if (!is_open())
                throw_except<io_exception>("File not open!");
        );
    }

    void file_channel::open(const tc::file& path, int oflags) {
        JSTD_DEBUG_CODE
        (
            if ((oflags & open_option::CREATE) && (oflags & open_option::APPEND))
                throw_except<io_exception>("CREATE + APPEND not allowed!");
            if (is_open())
                throw_except<io_exception>("file is already open!");
        );
        
        if (path.is_dir())
            throw_except<illegal_state_exception>("path is dir");

        JSTD_WIN_CODE
        (
            int o_prot = _O_BINARY;
            if (oflags & open_option::CREATE) o_prot |= _O_CREAT;
            if (oflags & open_option::APPEND) o_prot |= _O_APPEND;
            if (oflags & open_option::READ_WRITE)
            {
                if ((oflags & open_option::READ) && !(oflags & open_option::WRITE))
                {
                    o_prot |= _O_RDONLY;
                }
                else if ((oflags & open_option::WRITE) && !(oflags & open_option::READ))
                {
                    o_prot |= _O_WRONLY;
                } 
                else
                {
                    o_prot |= _O_RDWR;
                }
            };
            
            m_fd = internal::file_descriptor(path.c_str(), o_prot, _S_IRUSR | _S_IWUSR);
        );

        JSTD_POSIX_CODE
        (
            int o_prot = 0;
            if (oflags & open_option::CREATE) o_prot |= O_CREAT;
            if (oflags & open_option::APPEND) o_prot |= O_APPEND;
            if (oflags & open_option::READ_WRITE)
            {
                if ((oflags & open_option::READ) && !(oflags & open_option::WRITE))
                    o_prot |= O_RDONLY;
                else if ((oflags & open_option::WRITE) && !(oflags & open_option::READ))
                    o_prot |= O_WRONLY;
                else o_prot |= O_RDWR;
            };
            m_fd = internal::file_descriptor(path.c_str(), o_prot, S_IRUSR | S_IWUSR);
        );

        m_oflags = oflags;
    }
    
    std::uintmax_t file_channel::size() const {
        require_fd();
        JSTD_WIN_CODE
        (
            struct _stat64 stat;
            if (_fstat64(m_fd.get_fd(), &stat) != 0) {
                if (errno == EACCES)
                    throw_except<security_exception>(std::strerror(errno));
                throw_except<io_exception>(std::strerror(errno));
            }
            return (uintmax_t) stat.st_size;
        );

        JSTD_POSIX_CODE
        (
            struct stat64 stat;
            if (fstat64(m_fd.get_fd(), &stat) != 0) {
                if (errno == EACCES)
                    throw_except<security_exception>(std::strerror(errno));
                throw_except<io_exception>(std::strerror(errno));
            }
            return (uintmax_t) stat.st_size;
        );

        return 0;
    }
    
    bool file_channel::is_open() const {
        return m_fd != internal::file_descriptor::NULL_FD;
    }

    file_channel& file_channel::truncate(std::uintmax_t length) {
        require_fd();
        
        if (!(m_oflags & open_option::WRITE))
            throw_except<io_exception>("It is not possible to change the size of a read-only file");

        if (
            JSTD_WIN_CODE (ftruncate(m_fd.get_fd(), (long long) length) != 0)
            JSTD_UNIX_CODE(ftruncate(m_fd.get_fd(), (off_t)     length) != 0)
        ) {
            if (errno == EACCES) {
                JSTD_WIN_CODE(
                    throw_except<security_exception>(system::error_string(GetLastError()));
                );
                JSTD_POSIX_CODE(
                    throw_except<security_exception>(system::error_string(errno));
                );
            }
            throw_except<io_exception>(std::strerror(errno));
        }

        return *this;
    }
    
    mappedbuf file_channel::map(fmap_mode mode, std::size_t offset, std::size_t length) {
        require_fd();

        bool is_writable = (m_oflags & open_option::WRITE) != 0;
        bool is_readable = (m_oflags & open_option::READ) != 0;

        if (mode == fmap_mode::READ_WRITE && !is_writable)
            throw_except<io_exception>("file is not writable for READ_WRITE mapping");
        
        if (mode == fmap_mode::READ_ONLY && !is_readable)
            throw_except<io_exception>("file is not readable for READ_ONLY mapping");

        const std::size_t required_size = offset + length;
        if (is_writable)
        {
            if (size() < required_size)
            {
                truncate(required_size);
            }
        } 
        else if (size() < required_size)
        {
            throw_except<io_exception>("requested region exceeds file boundary and file is read-only");
        }

        const std::size_t page_size         = internal::get_page_size();
        const std::size_t aligned_offset    = offset & ~(page_size - 1);
        const std::size_t page_delta        = offset - aligned_offset;   
        const std::size_t map_length        = length + page_delta;

        void* page_base = nullptr;

        JSTD_WIN_CODE
        (
            const DWORD page_prot = (mode == fmap_mode::READ_WRITE) ? PAGE_READWRITE : PAGE_READONLY;
            const DWORD map_prot  = (mode == fmap_mode::READ_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;

            HANDLE h_file = reinterpret_cast<HANDLE>(_get_osfhandle(m_fd.get_fd()));
            
            ULARGE_INTEGER map_size;
            map_size.QuadPart = required_size;

            HANDLE h_map = CreateFileMapping(h_file, NULL, page_prot, map_size.HighPart, map_size.LowPart, NULL);
            if (!h_map) {
                throw_except<io_exception>(system::error_string(GetLastError()));
            }

            ULARGE_INTEGER map_offset;
            map_offset.QuadPart = aligned_offset;

            page_base = MapViewOfFile(h_map, map_prot, map_offset.HighPart, map_offset.LowPart, map_length);
            if (!page_base)
            {
                CloseHandle(h_map);
                throw_except<io_exception>(system::error_string(GetLastError()));
            }

            void* file_view = static_cast<char*>(page_base) + page_delta;
            mappedbuf buf(page_base, file_view, length, mode);
            buf.m_mapped_hanle = h_map;

            return buf;
        );

        JSTD_POSIX_CODE
        (
            const int prot = (mode == fmap_mode::READ_WRITE) ? (PROT_READ | PROT_WRITE) : PROT_READ;

            page_base = mmap(NULL, map_length, prot, MAP_SHARED, m_fd.get_fd(), static_cast<off_t>(aligned_offset));
            if (page_base == MAP_FAILED) {
                throw_except<io_exception>(std::strerror(errno));
            }

            void* file_view = static_cast<char*>(page_base) + page_delta;
            return mappedbuf(page_base, file_view, length, mode);
        );

        return mappedbuf();
    }
}

/**
 * ##########################################################################################
 *                          M A P P E D _ B Y T E _ B U F F E R
 * ##########################################################################################
 */
namespace tc
{
    mappedbuf::mappedbuf() : basebuf(), m_base(nullptr) {
        JSTD_WIN_CODE(
            m_mapped_hanle = NULL;
        );
    }

    mappedbuf::mappedbuf(mappedbuf&& buf) : basebuf(),
    m_base(buf.m_base) {
        
        m_base              = buf.m_base;
        m_capacity          = buf.m_capacity;
        m_limit             = buf.m_limit;
        m_mark              = buf.m_mark;
        m_order             = buf.m_order;
        m_readonly          = buf.m_readonly;
        m_position          = buf.m_position;
        m_data              = buf.m_data;

        buf.m_base      = nullptr;
        buf.m_data      = nullptr;
        buf.m_capacity  = 0;
        buf.m_limit     = 0;
        buf.m_mark      = npos();
        buf.m_position  = 0;

        JSTD_WIN_CODE
        (
            
            m_mapped_hanle      = buf.m_mapped_hanle;
            buf.m_mapped_hanle  = NULL;
        );
    }

    mappedbuf& mappedbuf::operator= (mappedbuf&& buf) {
        if (&buf != this)
        {
            
            std::swap(m_base,       buf.m_base);
            std::swap(m_capacity,   buf.m_capacity);
            std::swap(m_data,       buf.m_data);
            std::swap(m_limit,      buf.m_limit);
            std::swap(m_mark,       buf.m_mark);
            std::swap(m_order,      buf.m_order);
            std::swap(m_position,   buf.m_position);
            std::swap(m_readonly,   buf.m_readonly);
        
            JSTD_WIN_CODE
            (
                std::swap(m_mapped_hanle, buf.m_mapped_hanle);
            );
        }
        return *this;
    }

    mappedbuf::mappedbuf(void* page_base, void* base, std::size_t capacity, fmap_mode mode) : basebuf(), m_base(base) {
        m_readonly  = mode == fmap_mode::READ_ONLY;
        m_data      = static_cast<char*>(base);
        m_base      = page_base;
        m_capacity  = capacity;
        m_limit     = capacity;
    }

    void mappedbuf::cleanup() {
        if (m_base == nullptr)
            return;
        JSTD_WIN_CODE
        (
            UnmapViewOfFile(m_base);
            CloseHandle(m_mapped_hanle);
        );
        JSTD_POSIX_CODE
        (
            munmap(m_base, m_capacity);
        );
    }

    mappedbuf::~mappedbuf() {
        cleanup();
    }

    mappedbuf& mappedbuf::force() {
        JSTD_DEBUG_CODE
        (
            if (!m_data)
                throw_except<illegal_state_exception>("buffer is not mapped");
        );
        JSTD_WIN_CODE
        (
            if (!FlushViewOfFile(m_base, m_capacity))
                throw_except<io_exception>(system::error_string(GetLastError()));
        );
        JSTD_POSIX_CODE
        (
            msync(m_base, m_capacity, MS_SYNC);
        );
        return *this;
    }

}

namespace tc
{
    namespace internal
    {
        #if defined(JSTD_OS_WINDOWS)
            std::size_t get_page_size() {
                SYSTEM_INFO sys_info;
                GetSystemInfo(&sys_info);
                return (std::size_t) sys_info.dwAllocationGranularity;
            }
        #endif
        
        #if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
            std::size_t get_page_size() {
                #if defined(_SC_PAGESIZE)
                    return (std::size_t) sysconf(_SC_PAGESIZE);
                #elif defined(_SC_PAGE_SIZE)
                    return (std::size_t) sysconf(_SC_PAGE_SIZE);
                #else
                    #error Current platform not defined _SC_PAGESIZE or _SC_PAGE_SIZE
                #endif
            }
        #endif
    }
}