#ifndef AD5BC29C_C3A5_487E_9866_DC873F692E1B
#define AD5BC29C_C3A5_487E_9866_DC873F692E1B

#include <allocators/allocator.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>
#include <cstddef>
#include <atomic>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/utils/pair.hpp>

namespace tc
{
namespace internal
{

typedef typename int_of<32>::utype tcounter;

class control_block
{
    tca::allocator*         m_allocator;
    std::atomic<tcounter>   m_strong;
    std::atomic<tcounter>   m_weak;
public:
    /**
     * After contructing:
     *  m_strong = 1;
     *  m_weak   = 1;
     */
    control_block(tca::allocator* alloc) : m_allocator(alloc), m_strong(1), m_weak(1) {}
    
    /**
     * Call contained object destructor
     */
    virtual void destroy_object()   = 0;
    
    /**
     * Destroy control block
     */
    virtual ~control_block();
    
    /**
     * Call destructor control block and free memory
     */
    void destroy_control_block();
    
    /*###########################################################
                        I N C R E M E N T
    ###########################################################*/

    std::size_t inc_strong() {return ++m_strong;}
    std::size_t dec_strong() {return --m_strong;}
    
    /*###########################################################
                        D E C R E M E N T
    ###########################################################*/

    std::size_t dec_weak() {return --m_weak;}
    std::size_t inc_weak() {return ++m_weak;}

    /*###########################################################
                        G E T T E R S
    ###########################################################*/

    std::size_t get_strong() const {return m_strong;}
    std::size_t get_weak() const  {return m_weak;}
    
    /*###########################################################
                        L O C K
    ###########################################################*/

    bool try_inc_strong() {
        tcounter val = m_strong.load();
        while (true)
        {
            if (val == 0)
            {
                return false;
            }
            
            tcounter expected = val + 1;
            
            if (m_strong.compare_exchange_weak(val, expected))
            {
                return true;
            }
        }
    }

private:
    control_block(const control_block&) = delete;
    control_block(control_block&&) = delete;
    control_block& operator= (const control_block&) = delete;
    control_block& operator= (control_block&&) = delete;
};

} //namespace internal
} //namespace tc

namespace tc
{
namespace internal
{

template<typename T>
class obj_inline_control_block : public control_block {
    typedef typename remove_cv<T>::type Tvalue;
    union
    {
        Tvalue m_obj;
    };
public:
    template<typename... Args>
    obj_inline_control_block(tca::allocator* alloc, Args&&... args) : control_block(alloc) {
        new (&m_obj) Tvalue(std::forward<Args>(args)...);
    };
    void destroy_object() {m_obj.~Tvalue();}
    ~obj_inline_control_block() {}
    T* get_object() {return &m_obj;}
};

} //namespace internal
} //namespace tc

namespace tc
{
namespace internal
{

template<typename T, typename DELETER>
class ptr_control_block : public control_block {
    
    /**
     * Non const T type
     */
    typedef typename remove_cv<T>::type Tvalue;

    //Store pointer and deleter
    tc::pair<T*, DELETER> m_pair;
public:
    
    ptr_control_block(T* p, DELETER deleter, tca::allocator* alloc) : control_block(alloc), m_pair(p, std::move(deleter)) {
        
    };
    
    /**
     * 
     */
    void destroy_object() {m_pair.second()(const_cast<Tvalue*>(m_pair.first()));}
    
    /**
     * 
     */
    ~ptr_control_block() {}

    /**
     * 
     */
    T* get_object() {return m_pair.first();}
};

} //namespace internal
} //namespace tc

#endif /* AD5BC29C_C3A5_487E_9866_DC873F692E1B */
