#include <cpp/lang/system.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cstdarg>
#include <cstdio>
#include <cpp/lang/utils/cond_compile.hpp>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <cstring>
    #include<sys/time.h>
	#include<time.h>
#endif

namespace tc
{   
namespace system
{

namespace internal
{

    byte_order init_native_byte_order() {
        const unsigned long c   = 1;
        const unsigned char* ip = reinterpret_cast<const unsigned char*>(&c);
        return *ip != 0 ? byte_order::LE : byte_order::BE;
    }
    
    const byte_order system_order = init_native_byte_order();

}

    
    timepoint current_time_millis() {
    #if defined(__linux__) || defined(__APPLE__)
        timeval time;
        gettimeofday(&time, NULL);
        return (timepoint) ((time.tv_sec * 1000) + (time.tv_usec / 1000));
    #elif _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER time;
        time.LowPart    = ft.dwLowDateTime;
        time.HighPart   = ft.dwHighDateTime;
        ULONGLONG millesec = time.QuadPart / (timepoint) 10000;
        return (timepoint) (millesec - (timepoint) 11644473600000);
    #else
        #error Undefined platform
    #endif
    }
    
    timepoint current_time_seconds() {
        return current_time_millis() / 1000;
    }

    timepoint nano_time() {
        const long NS_SECOND = 1000000000L;
    #if defined(JSTD_OS_WINDOWS)
		
		LARGE_INTEGER freeq, counter;
        
        QueryPerformanceFrequency(&freeq);
        QueryPerformanceCounter(&counter);
        
        return (timepoint) ((counter.QuadPart * NS_SECOND) / freeq.QuadPart);
    #elif defined(JSTD_OS_MAC) || defined(JSTD_OS_LINUX)
		timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return (timepoint) ((time.tv_sec * NS_SECOND) + time.tv_nsec);
    #else
        #error Undefined OS
    #endif
    }


#if defined(_WIN32)
    const char* error_string(int err) {
        thread_local char no_specified_error_buffer[48];
        switch(err){
			case ERROR_SUCCESS              : return "No error";
			case ERROR_INSUFFICIENT_BUFFER  : return "The buffer size is insufficient to store the full path to the file";
			case ERROR_INVALID_HANDLE       : return "Invalid handle";
			case ERROR_MOD_NOT_FOUND        : return "Module not found";
			case ERROR_ACCESS_DENIED        : return "Access denied";
			case ERROR_INVALID_PARAMETER    : return "Invalid parameter";
			case ERROR_NOT_ENOUGH_MEMORY    : return "There is not enough memory to perform the operation";
			case ERROR_INVALID_FUNCTION     : return "The wrong was called to perform the operation";
			case ERROR_BAD_PATHNAME         : return "Bad path name";
			case ERROR_FILE_NOT_FOUND       : return "File not found";
			case ERROR_INVALID_NAME         : return "Invalid name";
			case ERROR_BAD_FORMAT           : return "Bad format";
            case ERROR_NOACCESS             : return "Invalid access to memory location";
            case ERROR_MAPPED_ALIGNMENT     : return "The base address or the file offset specified does not have the proper alignment";
            case ERROR_USER_MAPPED_FILE     : return "The requested operation cannot be performed on a file with a user-mapped section open";
			default:
                    std::snprintf(no_specified_error_buffer, sizeof(no_specified_error_buffer), "Windows error: %i", err);
                    return no_specified_error_buffer;
		}
    }
#elif defined(__linux__)
    const char* error_string(int err) {
        return strerror(err);
    }
#endif

    int tsprintf(const char* format, ...) {
        static tc::mutex ls_mutex;
        ls_mutex.lock();
            std::va_list args;
            va_start(args, format);
            int result = std::vprintf(format, args);
            va_end(args);
        ls_mutex.unlock();
        return result;
    }

}//namespace system

}//namespace jstd