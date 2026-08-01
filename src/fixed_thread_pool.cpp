#include <algorithm>
#include <allocators/base_allocator.hpp>
#include <cpp/lang/concurrency/pools/thread_pool.hpp>
#include <cpp/lang/concurrency/pools/fixed_thread_pool.hpp>


namespace tc
{
namespace concurrency
{

    using ftp = fixed_thread_pool;

    ftp::worker::worker() : thread() {

    }


    ftp::worker::worker(thread_pool* tpool) : thread(), 
    m_owner(tpool), 
    m_alive(true), 
    m_sleeping(false), 
    m_force_non_sleeping(false), 
    m_monitor_mtx(), 
    m_monitor_condvar() {

    }
    
    ftp::worker::worker(worker&& w) : thread(std::move(w)), 
    m_owner(w.m_owner), 
    m_alive(w.m_alive), 
    m_sleeping(w.m_sleeping), 
    m_force_non_sleeping(w.m_force_non_sleeping),
    m_monitor_mtx(), 
    m_monitor_condvar() {
        w.m_owner               = nullptr;
        m_alive                 = false;
        w.m_sleeping            = false;
        w.m_force_non_sleeping  = false;
    }
    
    ftp::worker& ftp::worker::operator= (worker&& w) {
        if (&w != this) {
            thread::operator=( std::move(w) );
            m_owner                 = w.m_owner;
            m_alive                 = w.m_alive;
            m_sleeping              = w.m_sleeping;
            m_force_non_sleeping    = w.m_force_non_sleeping;
            w.m_owner       = nullptr;
            w.m_alive       = false;
            w.m_sleeping    = false;
        }
        return *this;
    }

    ftp::worker::~worker() {

    }
    
    bool ftp::worker::is_alive() {
        unique_lock lock(m_monitor_mtx);
        return m_alive;
    }

    void ftp::worker::run() {
        while (true) {
            JSTD_DEBUG_CODE(
                if (m_owner == nullptr)
                    throw_except<illegal_state_exception>("owner thread pool == null");
            );
            
            task* current_task = m_owner->get_task();
            if (current_task != nullptr) {
                current_task->execute();
                current_task->set_done(true);
            } 
            
            else if(!is_alive()) {
                break;
            } 
            
            else {
                {
                    unique_lock lock(m_monitor_mtx);
                    if (m_force_non_sleeping) {
                        m_force_non_sleeping = false;
                        continue;
                    }
                    m_sleeping = true;
                    m_monitor_condvar.wait(lock);
                    m_sleeping = false;
                }
            }
        }
    }

    void ftp::worker::shutdown() {
        unique_lock lock(m_monitor_mtx);
        m_alive = false;
    }

    bool ftp::worker::is_sleeping() {
        unique_lock lock(m_monitor_mtx);
        return m_sleeping;
    }

    void ftp::worker::wakeup() {
        unique_lock lock(m_monitor_mtx);
        if (m_sleeping)
            m_monitor_condvar.notify();
        else 
            m_force_non_sleeping = true;
    }

    fixed_thread_pool::fixed_thread_pool(std::size_t count_threads, tca::allocator* allocator) : 
    m_threads(count_threads, allocator),
    m_queue(allocator),
    m_queue_locker(),
    m_tp_mutex(),
    m_terminated(false) {
        for (std::size_t i = 0; i < count_threads; ++i) {
            m_threads[i] = worker(this);
            m_threads[i].start();
        }
    }

    void ftp::submit(task* task) {
        JSTD_DEBUG_CODE(
            if (task == nullptr)
                throw_except<illegal_argument_exception>("task == null");
        );
        submit(&task, 1);
        unique_lock lock(m_queue_locker);
        m_queue.add_last(task);
        m_threads[0].wakeup();
    }

    void fixed_thread_pool::submit(task* tasks[], std::size_t bufsize) {
        {
            unique_lock pool_lock(m_tp_mutex);
            JSTD_DEBUG_CODE(
                if (tasks == nullptr)   throw_except<illegal_argument_exception>("tasks == null");
                if (m_terminated)       throw_except<illegal_state_exception>("thread_pool is terminated!");
            );
            {
                unique_lock lock(m_queue_locker);
                for (std::size_t i = 0; i < bufsize; ++i)
                    m_queue.add_last(tasks[i]);
            }
        }
        std::size_t neeed_threads = std::min<std::size_t>(bufsize, m_threads.length);
        for (std::size_t i = 0; i < neeed_threads; ++i)
            m_threads[i].wakeup();
    }

    task* fixed_thread_pool::get_task() {
        unique_lock lock(m_queue_locker);
        task* first_task = nullptr;
        if (m_queue.size() > 0)
            m_queue.remove_first(&first_task);
        return first_task;
    }

    void fixed_thread_pool::join_all() {
        std::size_t i = m_threads.length;
        while (i > 0)
            m_threads[--i].join();
    }

    void fixed_thread_pool::shutdown() {
        unique_lock lock(m_tp_mutex);
        if (m_terminated)
            return;
        for (std::size_t i = 0; i < m_threads.length; ++i) {
            m_threads[i].shutdown();
            m_threads[i].wakeup();
        }
        m_terminated = true;
    }

    void fixed_thread_pool::force_shutdown() {
        {
            unique_lock q_lock(m_queue_locker);
            m_queue.clear();
        }
        shutdown();
    }

}//namespace concurrency
}//namespace jstd