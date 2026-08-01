#include <cpp/lang/concurrency/thread.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cpp/lang/concurrency/runnable.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/system.hpp>
#include <cstring>
#include <exception>
#include <iostream>

namespace tc {
    
    /*static*/ const thread::state thread::state::NEW         = {0};
    /*static*/ const thread::state thread::state::RUNNABLE    = {1};
    /*static*/ const thread::state thread::state::TERMINATED  = {2};
    
    const char* thread::state::to_string() const {
        if (*this == NEW)               return "NEW";
        else if (*this == RUNNABLE)     return "RUNNABLE";
        else if (*this == TERMINATED)   return "TERMINATED";
        else                            return "Illegal state!";
    }
    
    std::size_t thread::state::hashcode() const {
        return (std::size_t) m_state;
    }
    
    bool thread::state::equals(const state& other) const {
        return m_state == other.m_state;
    }

    bool thread::state::operator==(const thread::state& state) const {
        return equals(state);
    }
    
    bool thread::state::operator!=(const thread::state& state) const {
        return !equals(state);
    }


    /**
     * Функция для запуска нового потока.
     * 
     * @param _this
     *      Указатель на объект потока, который владеет реальным потоком ОС.
     */
    void new_thread_start_func(thread* _this);

    /*static */ std::size_t thread::total_threads = 0;

    /*static */ std::mutex thread::global_lock;

    thread::thread() : thread(nullptr, nullptr, 0) {

    }

    thread::thread(runnable* task, const char* thread_name, std::size_t) : m_thread(), m_runnable(nullptr), m_thread_id(0) {
        global_lock.lock(); {
            m_thread_id = total_threads++;
        } global_lock.unlock();
          
        m_runnable  = task;    
        m_state     = thread::state::NEW;

        if (thread_name != nullptr)
            set_name(thread_name);
        else
            std::snprintf(m_name, sizeof(m_name), "Thread-%llu", (unsigned long long ) m_thread_id);
    }
    
    thread::thread(thread&& t) : m_thread(), m_runnable(nullptr), m_thread_id(0), m_state(state::NEW) {
        *this = std::move(t);
    }
    
    thread& thread::operator= (thread&& t) {
        if (&t != this) {
            if (joinable())
                throw_except<illegal_state_exception>("Move assignment is invalid! Target thread must not be joinable");
            
            m_thread    = std::move(t.m_thread);
            m_thread_id = t.m_thread_id;
            m_state     = t.get_state();
            set_name(t.get_name());
            m_runnable  = t.m_runnable;

        }
        return *this;
    }

    void thread::start() {
        m_state     = thread::state::RUNNABLE;
        m_thread    = std::thread(new_thread_start_func, this);
    }

    void new_thread_start_func(thread* _this) {
        try {
            _this->run();
        } catch (const throwable& e1) {
 #if defined(JSTD_ENABLE_CALLTRACE)
            static tc::mutex mutex;
            mutex.lock();
                system::tsprintf("Exception in thread\"%s\"\n", _this->get_name());
                e1.print_stack_trace();
            mutex.unlock();
#else
            system::tsprintf("Exception in thread \"%s\": %s\n", _this->get_name(), e1.cause());
#endif
        } catch (const std::exception& e2) {
            system::tsprintf("Exception in thread \"%s\": %s\n", _this->get_name(), e2.what());
        }
        
        _this->m_state = thread::state::TERMINATED;
        
    }

    void thread::run() {
        if (m_runnable != nullptr)
            m_runnable->run();
    }
    
    thread::~thread() {
        
    }

    const char* thread::get_name() const {
        return m_name;
    }

    void thread::set_name(const char* new_name) {
        ncopy(m_name, new_name, sizeof(m_name));
    }
    
    void thread::join() {
        m_thread.join();
    }

    bool thread::joinable() const {
        return m_thread.joinable();
    }

    thread::state thread::get_state() const {
        return m_state;
    }
}