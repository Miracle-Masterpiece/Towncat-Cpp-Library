#ifndef JSTD_CPP_LANG_CONCURRENCY_MUTEX_H
#define JSTD_CPP_LANG_CONCURRENCY_MUTEX_H

#include <mutex>

namespace tc
{

class mutex {
    friend class cond_var;
    friend class unique_lock;
    mutex(const mutex&)             = delete;
    mutex& operator= (const mutex&) = delete;
    mutex(mutex&&)                  = delete;
    mutex& operator= (mutex&&)      = delete;
    std::mutex m_mutex_impl;
public:
    mutex();
    ~mutex();
    void lock();
    void unlock();
    bool try_lock();
};

class unique_lock {
    friend class cond_var;
    std::unique_lock<std::mutex> m_mutex;
    unique_lock(const unique_lock&)             = delete;
    unique_lock& operator= (const unique_lock&) = delete;
    unique_lock(unique_lock&&)                  = delete;
    unique_lock& operator= (unique_lock&&)      = delete;
public:
    unique_lock(mutex& mutex)  : m_mutex(mutex.m_mutex_impl) {
        
    }
    ~unique_lock() {
        
    }
};

}// namespace tc

#endif//JSTD_CPP_LANG_CONCURRENCY_MUTEX_H