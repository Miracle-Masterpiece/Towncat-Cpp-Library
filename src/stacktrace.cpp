#include <cpp/lang/stacktrace/stacktrace.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <utility>
#include <iostream>
#include <string>

namespace tc
{

    calltrace::calltrace() JSTD_CALLTRACE_CODE( : m_stacktrace() ) {

    }

    calltrace::calltrace(const calltrace& ct) JSTD_CALLTRACE_CODE( : m_stacktrace(ct.m_stacktrace) )   {

    }
    
    calltrace::calltrace(calltrace&& ct) JSTD_CALLTRACE_CODE( : m_stacktrace(std::move(ct.m_stacktrace)) )   {

    }
    
    calltrace& calltrace::operator=(const calltrace& ct) {
        if (&ct != this) {
            JSTD_CALLTRACE_CODE(
                m_stacktrace = ct.m_stacktrace;
            );
        }
        return *this;
    }
    
    calltrace& calltrace::operator=(calltrace&& ct) {
        if (&ct != this) {
            JSTD_CALLTRACE_CODE(
                m_stacktrace = std::move(ct.m_stacktrace);
            );
        }
        return *this;
    }
    
    calltrace::~calltrace() {

    }
    
    /*static*/ calltrace calltrace::current() {
        JSTD_CALLTRACE_CODE (
            calltrace current;
            current.m_stacktrace = std::stacktrace::current();
            return current;
        );
        return calltrace();
    }

    void calltrace::print() const {
        JSTD_CALLTRACE_CODE (
            for (int i = 0; i < m_stacktrace.size() - 1; ++i) {
                std::string description     = m_stacktrace.at(i).description();
                std::string callable_file   = m_stacktrace.at(i + 1).source_file();
                int64_t callable_line       = m_stacktrace.at(i + 1).source_line();
                system::tsprintf("      at %s(\033[36m%s:%lli\033[0m)\n", description.c_str(), callable_file.c_str(), (long long) callable_line);
            }
        );
    }
    
    void calltrace::write_log(class ostream* out) const {
        JSTD_CALLTRACE_CODE(
            try {
                for (const std::stacktrace_entry& entry : m_stacktrace) {
                    const std::string& desc = entry.description();
                    out->write(desc.c_str(), desc.length());
                }
            } catch (const throwable& t) {
                std::cout << t.cause() << "\n";
            }
        );
    }

}

