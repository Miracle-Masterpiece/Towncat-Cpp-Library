#ifndef JSTD_CPP_LANG_CONCURRENCY_FIXED_THREAD_POOL_H
#define JSTD_CPP_LANG_CONCURRENCY_FIXED_THREAD_POOL_H

#include <cpp/lang/concurrency/pools/thread_pool.hpp>
#include <cpp/lang/concurrency/thread.hpp>
#include <cpp/lang/utils/linked_list.hpp>
#include <cpp/lang/array.hpp>

namespace tc
{
namespace concurrency
{

class fixed_thread_pool : public thread_pool {

    class worker : public thread {
        worker(const worker&)               = delete;
        worker& operator= (const worker&)   = delete;
        thread_pool*    m_owner;
        bool            m_alive;
        bool            m_sleeping;
        bool            m_force_non_sleeping;
        
        /**
         * объекты монитора
         */
        mutex           m_monitor_mtx;
        cond_var        m_monitor_condvar;

        bool is_alive();
    public:
        worker();
        worker(thread_pool* tpool);
        worker(worker&&);
        worker& operator= (worker&&);
        ~worker();
        void run() override;
        bool is_sleeping();
        void wakeup();
        void shutdown();
    };

    array<worker>           m_threads;
    linked_list<task*>      m_queue;
    mutex                   m_queue_locker;
    mutex                   m_tp_mutex;
    bool                    m_terminated;
public:
    // 
    // 
    // 
    fixed_thread_pool(std::size_t count_threads, tca::allocator* allocator = tca::get_scoped_or_default());
    
    // 
    // 
    // 
    void submit(task*) override;
    
    // 
    // 
    // 
    void submit(task* tasks[], std::size_t bufsize) override;
    
    // 
    // 
    // 
    task* get_task() override;
    
    // 
    // 
    // 
    void join_all() override;
    
    // 
    // 
    // 
    void shutdown() override;
    
    // 
    // 
    // 
    void force_shutdown() override;
};

}
}


#endif//JSTD_CPP_LANG_CONCURRENCY_FIXED_THREAD_POOL_H