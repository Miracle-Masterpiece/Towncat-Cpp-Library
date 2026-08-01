#include <cpp/lang/concurrency/pools/thread_pool.hpp>

namespace tc
{
namespace concurrency
{
    
    task::task() : m_mutex(), m_cond_var(), m_is_done(false) {

    }
    
    task::task(const task& t) : m_mutex(), m_cond_var(), m_is_done(t.m_is_done) {

    }
    
    task::task(task&& t) : m_mutex(), m_cond_var(), m_is_done(t.m_is_done) {
        t.m_is_done = false;
    }
    
    task& task::operator=(const task& t) {
        if (&t != this)
            m_is_done = t.m_is_done;
        return *this;
    }
    
    task& task::operator=(task&& t) {
        if (&t != this) {
            m_is_done   = t.m_is_done;
            t.m_is_done = false;
        }
        return *this;
    }

    task::~task() {
        
    }
    
    void task::reset() {
        m_is_done = false;
    }

    bool task::is_done() {
        unique_lock locker(m_mutex);
        return m_is_done;
    }

    void task::set_done(bool is_done) {
        unique_lock locker(m_mutex);
            m_is_done = is_done;
        m_cond_var.notify_all();
    }
    
    void task::join() {
        if (m_is_done)
            return;
        unique_lock locker(m_mutex);
        if (!m_is_done)
            m_cond_var.wait(locker);
    }

}// namespace concurrency
}// namespace jstd