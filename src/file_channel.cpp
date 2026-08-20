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
        JSTD_DEBUG_CODE (
            if ((oflags & open_option::CREATE) && (oflags & open_option::APPEND))
                throw_except<io_exception>("CREATE + APPEND not allowed!");
            if (is_open())
                throw_except<io_exception>("File is already open!");
        );
        
        if (path.is_dir())
            throw_except<illegal_state_exception>("path is dir");

        JSTD_WIN_CODE(
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

        JSTD_POSIX_CODE(
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
    }
    
    std::uintmax_t file_channel::size() const {
        require_fd();
        JSTD_WIN_CODE(
            struct _stat64 stat;
            if (_fstat64(m_fd.get_fd(), &stat) != 0) {
                if (errno == EACCES)
                    throw_except<sequrity_exception>(std::strerror(errno));
                throw_except<io_exception>(std::strerror(errno));
            }
            return (uintmax_t) stat.st_size;
        );

        JSTD_POSIX_CODE(
            struct stat64 stat;
            if (fstat64(m_fd.get_fd(), &stat) != 0) {
                if (errno == EACCES)
                    throw_except<sequrity_exception>(std::strerror(errno));
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
                    throw_except<sequrity_exception>(system::error_string(GetLastError()));
                );
                JSTD_POSIX_CODE(
                    throw_except<sequrity_exception>(system::error_string(errno));
                );
            }
            throw_except<io_exception>(std::strerror(errno));
        }

        return *this;
    }
    
    mappedbuf file_channel::map(fmap_mode mode, std::size_t offset, std::size_t length) {
        require_fd();
        
        if (mode == fmap_mode::READ_WRITE && !(m_oflags & open_option::WRITE))
            throw_except<io_exception>("File not writable");
        
        if (mode == fmap_mode::READ_ONLY && !(m_oflags & open_option::READ))
            throw_except<io_exception>("File not readable");

        if (!(m_oflags & open_option::WRITE))
        {
            std::uintmax_t fsize = size();
            if ((offset >= fsize) || fsize - offset < length)
                throw_except<io_exception>("The file is not open for writing - it is not possible to enlarge the file to the required size");
        }
        
        void* page_base = nullptr;
        void* file_view = nullptr;

        //WINDOWS
        {JSTD_WIN_CODE(
            DWORD page_prot = PAGE_NOACCESS;
            if (mode == fmap_mode::READ_ONLY) {
                if (m_oflags & open_option::WRITE) {
                    page_prot = PAGE_READWRITE;
                }
                else {
                    page_prot = PAGE_READONLY;
                }
            }
            else if (mode == fmap_mode::READ_WRITE) {
                page_prot = PAGE_READWRITE;
            }

            HANDLE h = (HANDLE) _get_osfhandle(m_fd.get_fd());

            HANDLE map;
            {
                ULARGE_INTEGER sz;
                sz.QuadPart = offset + length;
                map = CreateFileMapping(h, NULL, page_prot, sz.HighPart, sz.LowPart, NULL);
            }

            if (map == NULL)
                throw_except<io_exception>(system::error_string(GetLastError()));

            DWORD map_prot = 0;
            if (mode == fmap_mode::READ_ONLY)
            {
                if (m_oflags & open_option::WRITE)
                {
                    map_prot = FILE_MAP_READ | FILE_MAP_WRITE;
                } 
                else
                {
                    map_prot = FILE_MAP_READ;
                }
            }
            else if (mode == fmap_mode::READ_WRITE)
            {
                map_prot = FILE_MAP_READ | FILE_MAP_WRITE;
            }

            {
                const std::size_t PAGE_SIZE             = internal::get_page_size();
                const std::size_t offset_for_view       = offset & ~PAGE_SIZE;
                const std::size_t page_offset           = offset - offset_for_view;
                
                ULARGE_INTEGER large_offset;
                large_offset.QuadPart = page_offset;
                page_base = MapViewOfFile(map, map_prot, large_offset.HighPart, large_offset.LowPart, length);
                
                if (page_base == NULL) {
                    CloseHandle(map);
                    throw_except<io_exception>(system::error_string(GetLastError()));
                }
                
                file_view = reinterpret_cast<char*>(page_base) + offset_for_view;

                mappedbuf buf = mappedbuf(page_base, file_view, length, mode);
                buf.m_mapped_hanle = map;

                return mappedbuf(std::move(buf));
            }
        );}

        //POSIX
        {JSTD_POSIX_CODE(
            if (size() < offset + length)
                truncate(offset + length);
            int prot = 0;
            if (mode == fmap_mode::READ_ONLY)       prot = PROT_READ;
            else if (mode == fmap_mode::READ_WRITE) prot = PROT_READ | PROT_WRITE;
            
            const std::size_t PAGE_SIZE         = internal::get_page_size();
            const std::size_t offset_for_view   = offset & ~PAGE_SIZE;
            const std::size_t page_offset       = offset - offset_for_view;

            page_base = mmap(NULL, length, prot, MAP_SHARED, m_fd.get_fd(), (off_t) page_offset);
            if (page_base == MAP_FAILED)
                throw_except<io_exception>(std::strerror(errno));

            file_view = reinterpret_cast<char*>(page_base) + offset_for_view;

            return mappedbuf(page_base, file_view, length, mode);
        );}

        return mappedbuf(NULL, NULL, 0, fmap_mode::READ_ONLY);        
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
        JSTD_WIN_CODE
        (
            m_mapped_hanle      = buf.m_mapped_hanle;
            buf.m_mapped_hanle  = NULL;
        );
    }

    mappedbuf& mappedbuf::operator= (mappedbuf&& buf) {
        if (&buf != this)
        {
            cleanup();
            m_base      = buf.m_base;
            buf.m_base  = nullptr;
            JSTD_WIN_CODE
            (
                m_mapped_hanle      = buf.m_mapped_hanle;
                buf.m_mapped_hanle  = NULL;
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
        JSTD_DEBUG_CODE(
            if (m_data == nullptr)
                throw_except<illegal_state_exception>("buffer is not mapped");
        );
        JSTD_WIN_CODE(
            if (!FlushViewOfFile(m_base, m_capacity))
                throw_except<io_exception>(system::error_string(GetLastError()));
        );
        JSTD_POSIX_CODE(
            //int msync(void *start, size_t length, int flags);
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