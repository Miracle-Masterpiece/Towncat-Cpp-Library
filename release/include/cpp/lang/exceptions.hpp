#ifndef _JSTDLIB_CPP_LANG_EXCEPTIONS_THROWABLE_H_
#define _JSTDLIB_CPP_LANG_EXCEPTIONS_THROWABLE_H_

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <utility>
#include <cstddef>
#include <cpp/lang/stacktrace/stacktrace.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>

#define JSTD_THROWABLE_CAUSE_SIZE 256

namespace tc 
{

class throwable {
    calltrace m_calltrace;
    char _cause[JSTD_THROWABLE_CAUSE_SIZE];
public:
    // 
    // 
    // 
    throwable() noexcept;    
    
    // 
    // 
    // 
    throwable(const char* cause) noexcept;
    
    // 
    // 
    // 
    throwable(const throwable& t) noexcept;
    
    // 
    // 
    // 
    throwable(throwable&& t) noexcept;
    
    // 
    // 
    // 
    throwable& operator= (const throwable& t) noexcept;
    
    // 
    // 
    // 
    throwable& operator= (throwable&& t) noexcept;
    
    // 
    // 
    // 
    virtual ~throwable() noexcept;
    
    // 
    // 
    // 
    const char* cause() const noexcept;
    
    // 
    // 
    // 
    void print_stack_trace() const;
    
    // 
    // 
    // 
    const calltrace& get_calltrace() const;
    
    // 
    // 
    // 
    void set_calltrace(calltrace&& calltrace);
};

#define MAKE_EXCEPT_CLASS__(clazz_name, _extends)\
                                                class clazz_name : public _extends {\
                                                public:\
                                                    clazz_name() noexcept;\
                                                    clazz_name(const char* cause) noexcept;\
                                                };\


MAKE_EXCEPT_CLASS__(error,                               throwable)
MAKE_EXCEPT_CLASS__(out_of_memory_error,                 error)
MAKE_EXCEPT_CLASS__(exception,                           throwable)
MAKE_EXCEPT_CLASS__(runtime_exception,                   exception)
MAKE_EXCEPT_CLASS__(null_pointer_exception,              runtime_exception)
MAKE_EXCEPT_CLASS__(security_exception,                  runtime_exception)
MAKE_EXCEPT_CLASS__(interrupted_exception,               exception)


/**
 * #################################################################
 *                  R A N G E  E X C E P T I O N S
 * #################################################################
 */
MAKE_EXCEPT_CLASS__(index_out_of_bound_exception,        runtime_exception)
MAKE_EXCEPT_CLASS__(overflow_exception,                  runtime_exception)
MAKE_EXCEPT_CLASS__(underflow_exception,                 runtime_exception)
MAKE_EXCEPT_CLASS__(no_such_element_exception,           runtime_exception)
MAKE_EXCEPT_CLASS__(stack_overflow_error,                error)

/**
 * #################################################################
 *                  S T A T E  E X C E P T I O N S
 * #################################################################
 */
MAKE_EXCEPT_CLASS__(illegal_argument_exception,          runtime_exception)
MAKE_EXCEPT_CLASS__(illegal_state_exception,             runtime_exception)
MAKE_EXCEPT_CLASS__(invalid_mark_exception,              illegal_state_exception)

MAKE_EXCEPT_CLASS__(unsupported_operation_exception,             exception)
MAKE_EXCEPT_CLASS__(utf_format_exception,                runtime_exception)
MAKE_EXCEPT_CLASS__(invalid_data_format_exception,       runtime_exception)
MAKE_EXCEPT_CLASS__(number_format_exception,             runtime_exception)
MAKE_EXCEPT_CLASS__(class_cast_exception,                runtime_exception)

                
/**
 * #################################################################
 *                  I O E X C E P T I O N S
 * #################################################################
 */
MAKE_EXCEPT_CLASS__(io_exception,                        exception)
MAKE_EXCEPT_CLASS__(file_not_found_exception,            io_exception)
MAKE_EXCEPT_CLASS__(eof_exception,                       io_exception)
MAKE_EXCEPT_CLASS__(closed_exception,                    io_exception)
MAKE_EXCEPT_CLASS__(readonly_exception,                  io_exception)

//socket exceptions
MAKE_EXCEPT_CLASS__(socket_exception,                    io_exception)
MAKE_EXCEPT_CLASS__(unknow_host_exception,               io_exception)
MAKE_EXCEPT_CLASS__(connect_exception,                   socket_exception)
MAKE_EXCEPT_CLASS__(bind_exception,                      socket_exception)
MAKE_EXCEPT_CLASS__(socket_timeout_exception,            socket_exception)

#undef MAKE_EXCEPT_CLASS__


    template<typename T>
    T make_except(const char* format, ...) {
        va_list args;
        char buf[JSTD_THROWABLE_CAUSE_SIZE];
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        T except;
        except.set_calltrace(calltrace::current());

        return T(std::move(except));
    }

    template<typename T>
    void throw_except(const char* format, ...) {
        va_list args;
        char buf[JSTD_THROWABLE_CAUSE_SIZE];
        
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        
        T except(buf);
        except.set_calltrace(calltrace::current());
        throw T(std::move(except));
    }

    template<typename T>
    void throw_except() {
        T except;
        except.set_calltrace(calltrace::current());
        throw except;
    }
    
    inline void check_non_null(const void* p, const char* msg) {
        if (p == nullptr)
            throw_except<null_pointer_exception>(msg);
    }
    
    inline void check_non_null(const void* p) {
        if (p == nullptr)
            throw_except<null_pointer_exception>();
    }

    template<typename T>
    void check_index(T idx, T len) {
        JSTD_DEBUG_CODE(
            if (is_unsigned<T>::value)
            {
                if (idx >= len)
                    throw_except<index_out_of_bound_exception>("Index %zu out of bound for length %zu!", (std::size_t) idx, (std::size_t) len);
            }
            else
            {
                if (idx < 0 || idx >= len)
                    throw_except<index_out_of_bound_exception>("Index %d out of bound for length %d!", (std::ptrdiff_t) idx, (std::ptrdiff_t) len);
            }
        );
    }

}
#endif