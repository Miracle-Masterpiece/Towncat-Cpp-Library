#ifndef AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF
#define AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF

#include <allocators/base_allocator.hpp>

namespace tca
{
/**
 * @since 2.0
 */
class allocator : public tca::base_allocator {
    
    /**
     * 
     */
    allocator(const allocator&);
    
    /**
     * 
     */
    allocator& operator= (const allocator&);

public:
    /**
     * 
     */
    allocator();

    /**
     * 
     */
    allocator(base_allocator* parent);

    /**
     * 
     */
    allocator(allocator&&);
    
    /**
     * 
     */
    allocator& operator= (allocator&&);
        
    /**
     * 
     */
    virtual void deallocate(void*) = 0;

    /**
     * 
     */
    void deallocate(void*, std::size_t) override;

    /**
     * 
     */
    virtual ~allocator();
};

}

namespace tca
{

namespace internal 
{
    extern thread_local allocator* scoped_allocator;
}

    /**
     * Возвращает аллокатор по-умолчанию.
     */
    allocator* get_default_allocator();

    /**
     * Возвращает значение scoped_allocator, или get_default_allocator(), если scoped_allocator == nullptr
     */
    allocator* get_scoped_or_default();

    /**
     * Возвращает аллокатор для выделения памяти под исключений.
     */
    allocator* get_exception_allocator();

/**
 * Класс для установки некого аллокатора, как общего для текущей области видимости.
 */
class scope_allocator {
    /**
     * Указатель на предыдущий распределитель.
     */
    allocator* m_prev;
    
    /**
     * 
     */
    scope_allocator(const scope_allocator&)             = delete;
    
    /**
     * 
     */
    scope_allocator& operator= (const scope_allocator&) = delete;
    
    /**
     * 
     */
    scope_allocator(scope_allocator&&)                  = delete;
    
    /**
     * 
     */
    scope_allocator& operator= (scope_allocator&&)      = delete;
    
public:
    /**
     * @param allocator
     *      Указатель на распределитель, который будет установлен как аллокатор для текущей области видимости.
     */
    scope_allocator(allocator* allocator);
    
    /**
     * 
     */
    ~scope_allocator();
    
    /**
     * Возвращает указатель на распределитель, установленный в предыдущей области видимости.
     */
    allocator* get_prev() const;
};

}

#endif /* AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF */
