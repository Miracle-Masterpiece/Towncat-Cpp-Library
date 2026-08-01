#include <cpp/lang/concurrency/mutex.hpp>
#include <utility>

namespace tc
{

    mutex::mutex() {

    }

    mutex::~mutex() {
        
    }
    
    void mutex::lock() {
        m_mutex_impl.lock();
    }
    
    void mutex::unlock() {
        m_mutex_impl.unlock();
    }
    
    bool mutex::try_lock() {
        return m_mutex_impl.try_lock();
    }

}// namespace jstd