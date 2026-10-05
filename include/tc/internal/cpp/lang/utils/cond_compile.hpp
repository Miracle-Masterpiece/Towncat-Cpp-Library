#ifndef JSTD_CPP_LANG_UTILS_COND_COMPILATION_H
#define JSTD_CPP_LANG_UTILS_COND_COMPILATION_H

#if !defined(NDEBUG)
#   define JSTD_DEBUG_CODE(code) code
#else
#   define JSTD_DEBUG_CODE(code)
#endif

#if defined(__linux__) || defined(linux)
#   define JSTD_OS_LINUX
#elif defined(_unix_)
#   define JSTD_OS_UNIX
#elif defined(_WIN32) || defined(WIN32)
#   define JSTD_OS_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#   define JSTD_OS_MAC
#endif

#if defined(JSTD_OS_LINUX)
#   define JSTD_LINUX_CODE(code) code
#   define JSTD_UNIX_CODE(code)  code
#   define JSTD_MAC_CODE(code)
#   define JSTD_WIN_CODE(code)
#endif

#if defined(JSTD_OS_MAC)
#   define JSTD_MAC_CODE(code)  code
#   define JSTD_UNIX_CODE(code) code
#   define JSTD_LINUX_CODE(code)
#   define JSTD_WIN_CODE(code)
#endif

#if defined(JSTD_OS_WINDOWS)
#   define JSTD_WIN_CODE(code) code
#   define JSTD_MAC_CODE(code)
#   define JSTD_UNIX_CODE(code)
#   define JSTD_LINUX_CODE(code)
#endif

#if defined(JSTD_OS_UNIX) && !defined(JSTD_OS_LINUX)
#   define JSTD_WIN_CODE(code) 
#   define JSTD_MAC_CODE(code)   code
#   define JSTD_UNIX_CODE(code)  code
#   define JSTD_LINUX_CODE(code) code
#endif

#endif//JSTD_CPP_LANG_UTILS_COND_COMPILATION_H