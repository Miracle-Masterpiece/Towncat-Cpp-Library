#include <cpp/lang/errors.hpp>
#include <cstring>
#include <cpp/lang/utils/cond_compile.hpp>

#if defined(_WIN32)
# include <windows.h>
#endif

#include <cpp/lang/string.hpp>
#include <iostream>

namespace tc
{
    bool error_category::equivalent(int err, const class error_condition& cond) const {
        return default_error_condition(err) == cond;
    }
    
    bool error_category::equivalent(const class error_code& err, int cond) const {
        return (*this == err.get_category()) && (err.get_value() == cond);
    }
    
    error_condition error_category::default_error_condition(int err) const {
        return error_condition(err, *this);
    }
}

namespace tc
{

namespace internal
{
    /**
     * Error category for POSIX errno error codes.
     * 
     * Implements error_category for errno values. Provides the name "system"
     * and error messages via strerror().
     * 
     * @note
     *      On POSIX systems, this is equivalent to system_category.
     *      On Windows, system_category handles GetLastError() separately.
     * 
     * @warning
     *      strerror() may not be thread-safe on all platforms. Consider using
     *      strerror_r() or strerror_s() in thread-safe implementations.
     * 
     * @example
     *      int fd = open("/path", O_RDONLY);
     *      if (fd == -1) {
     *          error_code ec(errno, generic_category());
     *          string msg = ec.get_message();  // "No such file or directory"
     *      }
     * 
     * @see
     *      error_category, system_category(), error_code
     */
    class errno_category : public error_category {
        errno_category(const errno_category&) = delete;
        errno_category(errno_category&&) = delete;
        errno_category& operator= (const errno_category&) = delete;
        errno_category& operator= (errno_category&&) = delete;
    public:    
        
        errno_category() = default;
        
        /**
         * @return The category name "errno".
         */
        const char* get_name() const override {
            return "errno";
        }
        
        /**
         * Converts errno to a human-readable message.
         * 
         * @param err
         *      The errno value.
         * 
         * @param alloc
         *      Allocator for the returned string.
         * 
         * @return Error message string.
         */
        string get_message(int err, tca::allocator* alloc) const override {
            return string(std::strerror(err), alloc);
        }
    };

    /**
     * Error category for Windows system error codes.
     * 
     * Implements error_category for Windows GetLastError() codes.
     * Provides the name "system" and error messages via FormatMessageA().
     * 
     * @note
     *      Uses English (US) locale for consistent error messages across
     *      different system locales.
     * 
     * @warning
     *      The error message buffer is fixed at 128 characters. Very long
     *      error messages may be truncated.
     * 
     * @example
     *      HANDLE h = CreateFileA("nonexistent.txt", ...);
     *      if (h == INVALID_HANDLE_VALUE) {
     *          error_code ec(GetLastError(), windows_category());
     *          string msg = ec.get_message();  // "The system cannot find the file specified."
     *      }
     * 
     * @see
     *      error_category, system_category(), error_code
     */
    class windows_category : public error_category {
        windows_category(const windows_category&) = delete;
        windows_category(windows_category&&) = delete;
        windows_category& operator= (const windows_category&) = delete;
        windows_category& operator= (windows_category&&) = delete;
    public:    
        
        windows_category() = default;
    
        /**
         * @return The category name "system".
         */
        const char* get_name() const override {
            return "system";
        }
        
        /**
         * Converts Windows error code to a human-readable message.
         * 
         * @param err
         *      The Windows error code (from GetLastError()).
         * 
         * @param alloc
         *      Allocator for the returned string.
         * 
         * @return
         *      Error message string.
         * 
         * @note
         *      Uses FORMAT_MESSAGE_FROM_SYSTEM to retrieve system error strings.
         *      Messages are returned in English (US).
         */
        string get_message(int err, tca::allocator* alloc) const override {
            const std::size_t MSG_BUF_SIZE = 128;
            char msg_buffer[MSG_BUF_SIZE];
            msg_buffer[0] = '\0';

            DWORD result = FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                NULL, 
                static_cast<DWORD>(err),
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
                msg_buffer, 
                MSG_BUF_SIZE, 
                NULL
            );

            if (result == 0)
                return "N/A";

            return string(msg_buffer, alloc);
        }

        // 
        error_condition default_error_condition(int err) const override;

    };

} //namespace internal
} //namespace tc

#if defined(_WIN32)
#  include <windows.h>
#endif
#ifndef EROFS
#  define EROFS EINVAL
#endif
#ifndef ENAMETOOLONG
#  define ENAMETOOLONG EINVAL
#endif
#ifndef EMLINK
#  define EMLINK EINVAL
#endif
#ifndef ENOTEMPTY
#  define ENOTEMPTY EINVAL
#endif
namespace tc
{
namespace internal
{

inline int win32_to_posix_error(int err) noexcept {
#if defined(_WIN32)
    switch (err) {
        case 0: return 0;
        // --- Доступ и права ---
        case ERROR_ACCESS_DENIED:       return EACCES;
        case ERROR_WRITE_PROTECT:        return EROFS;
        case ERROR_PRIVILEGE_NOT_HELD:
        case ERROR_CANNOT_MAKE:         return EPERM;
        case ERROR_NOACCESS:            return EFAULT;

        // --- Файлы, каталоги и пути ---
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_NET_NAME:
        case ERROR_BAD_PATHNAME:
        case ERROR_DEV_NOT_EXIST:
        case ERROR_MOD_NOT_FOUND:
        case ERROR_NETNAME_DELETED:
        case ERROR_INVALID_NAME:        return ENOENT;

        case ERROR_ALREADY_EXISTS:
        case ERROR_FILE_EXISTS:
        case ERROR_FILE_CORRUPT:        return EEXIST;

        case ERROR_DIRECTORY:           return ENOTDIR;
        case ERROR_DIR_NOT_EMPTY:       return ENOTEMPTY;
        case ERROR_FILENAME_EXCED_RANGE: return ENAMETOOLONG;
        case ERROR_NOT_SAME_DEVICE:     return EXDEV;
        case ERROR_TOO_MANY_LINKS:      return EMLINK;

        // --- Дескрипторы и память ---
        case ERROR_INVALID_HANDLE:      return EBADF;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:         return ENOMEM;
        case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
        case ERROR_NO_MORE_SEARCH_HANDLES: return ENFILE;

        // --- Аргументы и параметры ---
        case ERROR_INVALID_PARAMETER:
        case ERROR_INVALID_DATA:
        case ERROR_INVALID_ADDRESS:
        case ERROR_BAD_PIPE:
        case ERROR_BAD_USERNAME:
        case ERROR_INVALID_SIGNAL_NUMBER:
        case ERROR_META_EXPANSION_TOO_LONG:
        case ERROR_NEGATIVE_SEEK:
        case ERROR_NONE_MAPPED:
        case ERROR_NO_TOKEN:
        case ERROR_SECTOR_NOT_FOUND:
        case ERROR_SEEK:
        case ERROR_THREAD_1_INACTIVE:   return EINVAL;

        // --- Блокировки и пайпы ---
        case ERROR_BUSY:
        case ERROR_DEVICE_IN_USE:
        case ERROR_LOCK_VIOLATION:
        case ERROR_SHARING_VIOLATION:
        case ERROR_PIPE_BUSY:
        case ERROR_PIPE_CONNECTED:
        case ERROR_CHILD_NOT_COMPLETE:
        case ERROR_SERVICE_REQUEST_TIMEOUT:
        case ERROR_SIGNAL_PENDING:
        case ERROR_TIMEOUT:             return EBUSY;

        case ERROR_BROKEN_PIPE:
        case ERROR_NO_DATA:             return EPIPE;

        // --- Ввод-Вывод (I/O) и диски ---
        case ERROR_IO_DEVICE:
        case ERROR_CRC:
        case ERROR_DISK_CORRUPT:
        case ERROR_OPEN_FAILED:
        case ERROR_BEGINNING_OF_MEDIA:
        case ERROR_BUS_RESET:
        case ERROR_DEVICE_DOOR_OPEN:
        case ERROR_DEVICE_REQUIRES_CLEANING:
        case ERROR_DS_GENERIC_ERROR:
        case ERROR_EOM_OVERFLOW:
        case ERROR_FILEMARK_DETECTED:
        case ERROR_INVALID_BLOCK_LENGTH:
        case ERROR_NO_DATA_DETECTED:
        case ERROR_NO_SIGNAL_SENT:
        case ERROR_SETMARK_DETECTED:
        case ERROR_SIGNAL_REFUSED:
        case ERROR_UNEXP_NET_ERR:       return EIO;

        // --- Место на диске и квоты ---
#ifdef ENOSPC
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
        case ERROR_END_OF_MEDIA:        return ENOSPC;
#else
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
        case ERROR_END_OF_MEDIA:        return EINVAL;
#endif
        case ERROR_NO_SYSTEM_RESOURCES: return EFBIG;

        // --- Исполняемые файлы (EXE) ---
        case ERROR_BAD_EXE_FORMAT:
        case ERROR_EXE_MACHINE_TYPE_MISMATCH:
        case ERROR_EXE_MARKED_INVALID:
        case ERROR_INVALID_EXE_SIGNATURE:
        case ERROR_IOPL_NOT_ENABLED:   return ENOEXEC;

        // --- Не поддерживается ---
        case ERROR_BAD_NET_RESP:
        case ERROR_CALL_NOT_IMPLEMENTED:
        case ERROR_NOT_SUPPORTED:       return ENOSYS;

        // --- Ошибки оборудования ---
        case ERROR_BAD_DEVICE:
        case ERROR_BAD_UNIT:
        case ERROR_INVALID_DRIVE:       return ENODEV;

        case ERROR_FILE_INVALID:        return ENXIO;

        // --- Асинхронность и ресурсы ---
        case ERROR_ACTIVE_CONNECTIONS:
        case ERROR_COMMITMENT_LIMIT:
        case ERROR_IO_INCOMPLETE:
        case ERROR_IO_PENDING:
        case ERROR_MAX_THRDS_REACHED:
        case ERROR_NONPAGED_SYSTEM_RESOURCES:
        case ERROR_NO_PROC_SLOTS:
        case ERROR_OPEN_FILES:
        case ERROR_PAGED_SYSTEM_RESOURCES:
        case ERROR_PAGEFILE_QUOTA:
        case ERROR_WORKING_SET_QUOTA:   return EAGAIN;

        // --- Разное ---
        case ERROR_CANCELLED:
        case ERROR_INVALID_AT_INTERRUPT_TIME: return EINTR;

        case ERROR_MORE_DATA:           return EMSGSIZE;
        case ERROR_POSSIBLE_DEADLOCK:   return EDEADLK;
        case ERROR_PROCESS_ABORTED:     return EFAULT;
        case ERROR_PROC_NOT_FOUND:      return ESRCH;
        case ERROR_SHARING_BUFFER_EXCEEDED: return ENOLCK;

        default: return 0;
    }
#else
    return 0;
#endif
}

    error_condition windows_category::default_error_condition(int err) const {
        if (err == 0) {
            return error_condition(0, generic_category());
        }

    #if defined(_WIN32) && !defined(__CYGWIN__)
        int posix_val = win32_to_posix_error(err);
        if (posix_val != 0) {
            return error_condition(posix_val, generic_category());
        }
    #else
        // Список базовых кодов POSIX с защитными #ifdef
        switch (err) {
    #ifdef E2BIG
            case E2BIG:
    #endif
    #ifdef EACCES
            case EACCES:
    #endif
    #ifdef EADDRINUSE
            case EADDRINUSE:
    #endif
    #ifdef EADDRNOTAVAIL
            case EADDRNOTAVAIL:
    #endif
    #ifdef EAFNOSUPPORT
            case EAFNOSUPPORT:
    #endif
    #ifdef EAGAIN
            case EAGAIN:
    #endif
    #ifdef EALREADY
            case EALREADY:
    #endif
    #ifdef EBADF
            case EBADF:
    #endif
    #ifdef EBADMSG
            case EBADMSG:
    #endif
    #ifdef EBUSY
            case EBUSY:
    #endif
    #ifdef ECANCELED
            case ECANCELED:
    #endif
    #ifdef ECHILD
            case ECHILD:
    #endif
    #ifdef ECONNABORTED
            case ECONNABORTED:
    #endif
    #ifdef ECONNREFUSED
            case ECONNREFUSED:
    #endif
    #ifdef ECONNRESET
            case ECONNRESET:
    #endif
    #ifdef EDEADLK
            case EDEADLK:
    #endif
    #ifdef EDESTADDRREQ
            case EDESTADDRREQ:
    #endif
            case EDOM:
    #ifdef EEXIST
            case EEXIST:
    #endif
    #ifdef EFAULT
            case EFAULT:
    #endif
    #ifdef EFBIG
            case EFBIG:
    #endif
    #ifdef EHOSTUNREACH
            case EHOSTUNREACH:
    #endif
    #ifdef EIDRM
            case EIDRM:
    #endif
            case EILSEQ:
    #ifdef EINPROGRESS
            case EINPROGRESS:
    #endif
    #ifdef EINTR
            case EINTR:
    #endif
    #ifdef EINVAL
            case EINVAL:
    #endif
    #ifdef EIO
            case EIO:
    #endif
    #ifdef EISCONN
            case EISCONN:
    #endif
    #ifdef EISDIR
            case EISDIR:
    #endif
    #ifdef ELOOP
            case ELOOP:
    #endif
    #ifdef EMFILE
            case EMFILE:
    #endif
    #ifdef EMLINK
            case EMLINK:
    #endif
    #ifdef EMSGSIZE
            case EMSGSIZE:
    #endif
    #ifdef ENAMETOOLONG
            case ENAMETOOLONG:
    #endif
    #ifdef ENETDOWN
            case ENETDOWN:
    #endif
    #ifdef ENETRESET
            case ENETRESET:
    #endif
    #ifdef ENETUNREACH
            case ENETUNREACH:
    #endif
    #ifdef ENFILE
            case ENFILE:
    #endif
    #ifdef ENOBUFS
            case ENOBUFS:
    #endif
    #ifdef ENODATA
            case ENODATA:
    #endif
    #ifdef ENODEV
            case ENODEV:
    #endif
    #ifdef ENOENT
            case ENOENT:
    #endif
    #ifdef ENOEXEC
            case ENOEXEC:
    #endif
    #ifdef ENOLCK
            case ENOLCK:
    #endif
    #ifdef ENOLINK
            case ENOLINK:
    #endif
    #ifdef ENOMEM
            case ENOMEM:
    #endif
    #ifdef ENOMSG
            case ENOMSG:
    #endif
    #ifdef ENOPROTOOPT
            case ENOPROTOOPT:
    #endif
    #ifdef ENOSPC
            case ENOSPC:
    #endif
    #ifdef ENOSR
            case ENOSR:
    #endif
    #ifdef ENOSTR
            case ENOSTR:
    #endif
    #ifdef ENOSYS
            case ENOSYS:
    #endif
    #ifdef ENOTCONN
            case ENOTCONN:
    #endif
    #ifdef ENOTDIR
            case ENOTDIR:
    #endif
    #if defined ENOTEMPTY && (!defined EEXIST || ENOTEMPTY != EEXIST)
            case ENOTEMPTY:
    #endif
    #ifdef ENOTRECOVERABLE
            case ENOTRECOVERABLE:
    #endif
    #ifdef ENOTSOCK
            case ENOTSOCK:
    #endif
    #if defined ENOTSUP && (!defined ENOSYS || ENOTSUP != ENOSYS)
            case ENOTSUP:
    #endif
    #ifdef ENOTTY
            case ENOTTY:
    #endif
    #ifdef ENXIO
            case ENXIO:
    #endif
    #if defined EOPNOTSUPP && (!defined ENOTSUP || EOPNOTSUPP != ENOTSUP)
            case EOPNOTSUPP:
    #endif
    #ifdef EOVERFLOW
            case EOVERFLOW:
    #endif
    #ifdef EOWNERDEAD
            case EOWNERDEAD:
    #endif
    #ifdef EPERM
            case EPERM:
    #endif
    #ifdef EPIPE
            case EPIPE:
    #endif
    #ifdef EPROTO
            case EPROTO:
    #endif
    #ifdef EPROTONOSUPPORT
            case EPROTONOSUPPORT:
    #endif
    #ifdef EPROTOTYPE
            case EPROTOTYPE:
    #endif
            case ERANGE:
    #ifdef EROFS
            case EROFS:
    #endif
    #ifdef ESPIPE
            case ESPIPE:
    #endif
    #ifdef ESRCH
            case ESRCH:
    #endif
    #ifdef ETIME
            case ETIME:
    #endif
    #ifdef ETIMEDOUT
            case ETIMEDOUT:
    #endif
    #ifdef ETXTBSY
            case ETXTBSY:
    #endif
    #if defined EWOULDBLOCK && (!defined EAGAIN || EWOULDBLOCK != EAGAIN)
            case EWOULDBLOCK:
    #endif
    #ifdef EXDEV
            case EXDEV:
    #endif
            return error_condition(err, generic_category());
        default:
            break;
        }
    #endif

        // Если прямого соответствия с POSIX нет
        return error_condition(err, *this);
    }

} //namespace internal
} //namespace tc

namespace tc
{
    const error_category& generic_category() {
        static const internal::errno_category errno_cat;
        return errno_cat;
    }

    const error_category& system_category() {
        JSTD_WIN_CODE (
            static const internal::windows_category system_cat;
            return system_cat;
        )
        return generic_category();   
    }
}

namespace tc
{
    error_code::error_code() : val(0), category(&system_category()) {

    }
    
    error_code::error_code(int val, const error_category& ecat) : val(val), category(&ecat) {

    }

    error_category::~error_category() {

    }
} //namespace tc

namespace tc
{
    error_condition::error_condition() : error_condition(0, generic_category()) {

    }
    
    error_condition::error_condition(int err, const error_category& ecat) : val(err), category(&ecat) {

    }

} //namespace tc

#include <iostream>
#include <cstring>
#include <cpp/lang/expected.hpp>

struct test {

    int var;

    test() {
        std::cout << "test()\n";
    }
    test(test&&) {
        std::cout << "test(test&&)\n";
    }
    test(const test&) {
        std::cout << "test(const test&)\n";
    }
    test& operator=(test&&) {
        std::cout << "test& operator= (test&&)\n";
        return *this;
    }
    test& operator=(const test&) {
        std::cout << "test& operator= (const test&)\n";
        return *this;
    }
    ~test() {
        std::cout << "~test()\n";
    }
};

#include <expected>
#include <cerrno>
#include <cpp/lang/errcode.hpp>

int main() {
    
}