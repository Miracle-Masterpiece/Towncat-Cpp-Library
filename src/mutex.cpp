#include <cpp/lang/concurrency/mutex.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>

namespace tc
{

    mutex::mutex() {

    }

    mutex::~mutex() {
        
    }
    
    void mutex::lock() {
        try {
            m_mutex_impl.lock();
        } catch (...) {
            throw_except<interrupted_exception>();
        }
    }
    
    void mutex::unlock() {
        try {
            m_mutex_impl.unlock();
        } catch (...) {
            throw_except<interrupted_exception>();
        }
    }
    
    bool mutex::try_lock() {
        try {
            return m_mutex_impl.try_lock();
        } catch (...) {
            throw_except<interrupted_exception>();
        }
    }

}// namespace jstd