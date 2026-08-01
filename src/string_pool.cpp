#include <cpp/lang/string_pool.hpp>

namespace tc
{
    using scp = string_const_pool;
    
    scp::string_const_pool(tca::allocator* allocator) : m_allocator(allocator), m_map(allocator) {

    }

    scp::~string_const_pool() {

    }
    
    shared_ptr<const string>& scp::get(const char* str) {
        JSTD_DEBUG_CODE(if (m_allocator == nullptr) throw_except<illegal_state_exception>("allocator is null"));
        shared_ptr<const string>& in_map = m_map.get(cstr(str));
        if (!in_map)
        {
            shared_ptr<const string> shared = 
                                                make_shared<const string>(
                                                    string(str, m_allocator), m_allocator
                                                );
            m_map.put(str, shared);
            return m_map.get(cstr(str));
        }
        else
        {
            return in_map;
        }
    }

    bool scp::deintern(const char* str) {
        JSTD_DEBUG_CODE(
            if (str == nullptr)
                throw_except<null_pointer_exception>("str is null");
        );
        return m_map.remove(str);
    }
    
    bool scp::deintern(shared_ptr<const string> str) {
        return deintern(str->cstr());
    }

    void scp::print_log() const {
        for (const hash_map<cstr, shared_ptr<const string>>::entry& e : m_map)
            system::tsprintf("[%s]\n", e.get_value()->cstr());
    }

    bool scp::intern(const char* str) {
        JSTD_DEBUG_CODE(
            if (m_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator is null");
        );
        return m_map.put(str, 
            make_shared<const string>(
                string(str, m_allocator), m_allocator
            )
        );
    }

    void scp::clear() {
        m_map.clear();
    }

    /**
     * ###################################################################
     *      C O N C U R R E N C Y _ S T R I N G _ C O N S T _ P O O l
     * ###################################################################
     * 
     */

    using cscp = concurrency_string_const_pool;
    cscp::concurrency_string_const_pool(tca::allocator* allocator) : m_strpool(allocator), m_mutex() {

    }

    cscp::~concurrency_string_const_pool() {

    }
    
    shared_ptr<const string>& cscp::get(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.get(str);
    }
    
    bool cscp::intern(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.intern(str);
    }
    
    bool cscp::deintern(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.deintern(str);
    }
    
    bool cscp::deintern(shared_ptr<const string> str) {
        unique_lock lock(m_mutex);
        return m_strpool.deintern(str);
    }
    
    void cscp::print_log() const {
        m_strpool.print_log();
    }
    
    void cscp::clear() {
        unique_lock lock(m_mutex);
        m_strpool.clear();
    }
}