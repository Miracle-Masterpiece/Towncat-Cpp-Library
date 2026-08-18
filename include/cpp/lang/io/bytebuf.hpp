#ifndef DCB0691A_7801_4070_9298_47C5228C552F
#define DCB0691A_7801_4070_9298_47C5228C552F

#include <cpp/lang/io/basebuf.hpp>
#include <allocators/allocator.hpp>

namespace tc
{

class bytebuf : public basebuf {
    
    /**
     * 
     */
    tca::allocator* const m_alloc;
public:
    /**
     * 
     */
    bytebuf(tca::allocator* alloc = tca::get_default_allocator());
    
    /**
     * 
     */
    bytebuf(std::size_t capacity, tca::allocator* alloc = tca::get_default_allocator());
    
    /**
     * 
     */
    bytebuf(bytebuf&&);
    
    /**
     * 
     */
    bytebuf(const bytebuf&);
    
    /**
     * 
     */
    bytebuf& operator= (bytebuf&&);

    /**
     * 
     */
    bytebuf& operator= (const bytebuf& buf);

    /**
     * Возвращает аллокатор, управляющий памятью этого буфера.
     */
    tca::allocator* get_allocator() const {
        return m_alloc;
    }

    /**
     * 
     */
    ~bytebuf();
};

}

#endif /* DCB0691A_7801_4070_9298_47C5228C552F */
