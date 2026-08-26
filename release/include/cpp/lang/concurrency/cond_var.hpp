#ifndef JSTD_CPP_LANG_CONCURRENCY_COND_VAR_H
#define JSTD_CPP_LANG_CONCURRENCY_COND_VAR_H
#include <condition_variable>
#include <cpp/lang/concurrency/mutex.hpp>

namespace tc
{

class cond_var {
    std::condition_variable m_cond;
    cond_var(const cond_var&) = delete;
    cond_var(cond_var&&) = delete;
    cond_var& operator=(const cond_var&) = delete;
    cond_var& operator=(cond_var&&) = delete;
public:
    cond_var() : m_cond() {}
    ~cond_var() {}
    void wait(unique_lock& lck) {
        m_cond.wait(lck.m_mutex);
    }
    void notify() {
        m_cond.notify_one();
    }
    void notify_all() {
        m_cond.notify_all();
    }
};

}

#endif//JSTD_CPP_LANG_CONCURRENCY_COND_VAR_H