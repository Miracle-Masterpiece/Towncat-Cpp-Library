#include <cpp/lang/exceptions.hpp>
#include <utility>

namespace tc 
{

    throwable::throwable() noexcept : m_calltrace() {
        _cause[0] = 0;
    }

    throwable::throwable(const char* cause) noexcept : m_calltrace() {
        std::size_t len         = std::strlen(cause);
        std::size_t max_size    = sizeof(_cause);
        std::size_t max         = len >= max_size ? max_size - 1 : len;
        std::memcpy(_cause, cause, max);
        _cause[max] = 0; 
    }
    
    throwable::throwable(const throwable& t) noexcept : m_calltrace(t.m_calltrace) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable::throwable(throwable&& t) noexcept : m_calltrace(std::move(t.m_calltrace)) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable& throwable::operator= (const throwable& t) noexcept {
        if (&t != this)
        {
            m_calltrace = t.m_calltrace;
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable& throwable::operator= (throwable&& t) noexcept {
        if (&t != this)
        {
            m_calltrace = std::move(t.m_calltrace);
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable::~throwable() noexcept {

    }
    
    const char* throwable::cause() const noexcept {
        return _cause;
    }

    void throwable::print_stack_trace() const {
        m_calltrace.print();
    }
    
    const calltrace& throwable::get_calltrace() const {
        return m_calltrace;
    }
    
    void throwable::set_calltrace(calltrace&& calltrace) {
        m_calltrace = std::move(calltrace);
    }

#define TEMPLATE__EXCEPT_CLASS_IMPL(clazz_name, super_clazz)\
    clazz_name::clazz_name() noexcept : super_clazz() {}\
    clazz_name::clazz_name(const char* cause) noexcept : super_clazz(cause) {}\
                   
TEMPLATE__EXCEPT_CLASS_IMPL(error,                               throwable)
TEMPLATE__EXCEPT_CLASS_IMPL(out_of_memory_error,                 error)
TEMPLATE__EXCEPT_CLASS_IMPL(exception,                           throwable)
TEMPLATE__EXCEPT_CLASS_IMPL(runtime_exception,                   exception)
TEMPLATE__EXCEPT_CLASS_IMPL(null_pointer_exception,              runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(security_exception,                  runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(interrupted_exception,               exception)

/**
 * #################################################################
 *                  R A N G E  E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(index_out_of_bound_exception,        runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(overflow_exception,                  runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(underflow_exception,                 runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(no_such_element_exception,           runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(stack_overflow_error,                error)

/**
 * #################################################################
 *                  S T A T E  E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(illegal_argument_exception,          runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(illegal_state_exception,             runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(invalid_mark_exception,              illegal_state_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(unsupported_operation_exception,             exception)
TEMPLATE__EXCEPT_CLASS_IMPL(utf_format_exception,                runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(invalid_data_format_exception,       runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(number_format_exception,             runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(class_cast_exception,                runtime_exception)

/**
 * #################################################################
 *                  I O E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(io_exception,                        exception)
TEMPLATE__EXCEPT_CLASS_IMPL(file_not_found_exception,            io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(eof_exception,                       io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(closed_exception,                    io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(readonly_exception,                  io_exception)

//socket exceptions
TEMPLATE__EXCEPT_CLASS_IMPL(socket_exception,                    io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(unknow_host_exception,               io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(connect_exception,                   socket_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(bind_exception,                      socket_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(socket_timeout_exception,            socket_exception)


    

}