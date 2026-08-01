#ifndef JSTD_CPP_LANG_STACKTRACE_H
#define JSTD_CPP_LANG_STACKTRACE_H

#include <cpp/lang/utils/cond_compile.hpp>



#if __cplusplus >= 202302L
#   include <stacktrace>
#   define JSTD_ENABLE_CALLTRACE
#   define JSTD_CALLTRACE_CODE(code) code
#else 
#   define JSTD_CALLTRACE_CODE(code)
#endif


namespace tc
{

class calltrace {
    JSTD_CALLTRACE_CODE (
        std::stacktrace m_stacktrace;
    )
public:
    // 
    // 
    // 
    calltrace();
    
    // 
    // 
    // 
    calltrace(const calltrace&);
    
    // 
    // 
    // 
    calltrace(calltrace&&);
    
    // 
    // 
    // 
    calltrace& operator=(const calltrace&);
    
    // 
    // 
    // 
    calltrace& operator=(calltrace&&);
    
    // 
    // 
    // 
    ~calltrace();
    
    // 
    // 
    // 
    void print() const;
    
    // 
    // 
    // 
    void write_log(class ostream* out) const;
    
    // 
    // 
    // 
    static calltrace current();
};

}// namespace jstd

#endif//JSTD_CPP_LANG_STACKTRACE_H