#ifndef JSTD_CPP_LANG_CONCURRENCY_MUTEX_H
#define JSTD_CPP_LANG_CONCURRENCY_MUTEX_H

#include <mutex>

namespace tc
{

/**
 * Mutual exclusion lock for thread synchronization.
 * 
 * Provides mutual exclusion (mutex) functionality for protecting shared
 * resources from concurrent access. Supports lock, unlock, and try_lock
 * operations. Intended to be used with unique_lock for RAII-style locking.
 * 
 * Recursive locking is not supported (undefined behavior).
 */
class mutex {
    friend class cond_var;
    friend class unique_lock;
    /**
     * Mutex implementation handle.
     * 
     * Internal implementation detail. Not intended for direct use.
     */
    std::mutex m_mutex_impl;
public:
    /**
     * Constructs a mutex in the unlocked state.
     */
    mutex();
    
    /**
     * Destroys the mutex.
     * 
     * The mutex must not be locked when destroyed.
     * Destroying a locked mutex results in undefined behavior.
     */
    ~mutex();

    /**
     * Locks the mutex.
     * 
     * Blocks the calling thread until the mutex is acquired.
     * If the mutex is already locked by another thread, the calling thread
     * is blocked until the mutex becomes available.
     * 
     * The mutex must be unlocked by the same thread that locked it.
     * Locking a mutex that is already locked by the same thread
     *       results in undefined behavior (deadlock).
     * 
     * @throws interrupted_exception
     *      If an error occurs (e.g., deadlock detected).
     */
    void lock();

    /**
     * Unlocks the mutex.
     * 
     * Releases the mutex, allowing other threads to acquire it.
     * The mutex must be locked by the calling thread.
     * 
     * @note Unlocking a mutex that is not locked by the calling thread
     *       results in undefined behavior.
     * 
     * @throws interrupted_exception
     *      If an error occurs.
     */
    void unlock();

    /**
     * Tries to lock the mutex without blocking.
     * 
     * Attempts to acquire the mutex. If the mutex is not currently locked,
     * it is locked and true is returned. If the mutex is locked by another
     * thread, false is returned immediately without blocking.
     * 
     * @return
     *      true if the mutex was acquired, false otherwise.
     * 
     * @throws interrupted_exception
     *      If an error occurs.
     */
    bool try_lock();
private:
    mutex(const mutex&)             = delete;
    mutex& operator= (const mutex&) = delete;
    mutex(mutex&&)                  = delete;
    mutex& operator= (mutex&&)      = delete;
};

/**
 * RAII-style lock guard for mutex.
 * 
 * Provides RAII management for
 * mutex locks. The mutex is locked upon construction and automatically
 * unlocked upon destruction. This ensures proper lock release even when
 * exceptions are thrown.
 */
class unique_lock {
    friend class cond_var;
    /**
     * Lock implementation handle.
     * Internal implementation detail. Not intended for direct use.
     */
    std::unique_lock<std::mutex> m_mutex;
public:
    
    /**
     * Constructs a unique_lock and locks the mutex.
     * 
     * Acquires the mutex upon construction. The mutex is locked before
     * the constructor returns.
     * 
     * @param mutex
     *      Reference to the mutex to lock.
     * 
     * @throws interrupted_exception
     *      If an error occurs while locking the mutex.
     */
    unique_lock(mutex& mutex)  : m_mutex(mutex.m_mutex_impl) {
        
    }

    /**
     * Destroys the unique_lock and unlocks the mutex.
     * 
     * Automatically releases the mutex if it is locked. The mutex is
     * guaranteed to be unlocked when the destructor completes.
     */
    ~unique_lock() {
        
    }
private:
    unique_lock(const unique_lock&)             = delete;
    unique_lock& operator= (const unique_lock&) = delete;
    unique_lock(unique_lock&&)                  = delete;
    unique_lock& operator= (unique_lock&&)      = delete;
};

}// namespace tc

#endif//JSTD_CPP_LANG_CONCURRENCY_MUTEX_H