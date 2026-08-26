#ifndef _ALLOCATORS_BASE_ALLOCATOR_H
#define _ALLOCATORS_BASE_ALLOCATOR_H

#include <cctype>
#include <cstddef>

namespace tca
{

/**
 * Базовый тип полиморфного распределителя.
 */
class base_allocator {
    /**
     * deleted
     */
    base_allocator(const base_allocator& base)              = delete;
    
    /**
     * deleted
     */
    base_allocator& operator= (const base_allocator& base)  = delete;

protected:    
    /**
     * Указатель на родительский объект распределителя.
     */
    base_allocator* m_parent;

public:    
    /**
     * 
     */
    base_allocator();

    /**
     * Создаёт распределитель памяти с использованием родительского распределителя.
     * 
     * @param parent
     *          Указатель на родительский распределитель.
     *          
     */
    base_allocator(base_allocator* parent);
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * 
     * @param base
     *          R-value ссылка рапределителя для перемещения данных из base в этот объект.
     */
    base_allocator(base_allocator&& base);
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * 
     * @param base
     *          R-value ссылка рапределителя для перемещения данных из base в этот объект.
     */
    base_allocator& operator= (base_allocator&& base);
    
    /**
     * В этой версии ничего не делает.
     */
    virtual ~base_allocator() = 0;
    
    /**
     * Выделяет блок памяти размером sz и возвращает на него указатель void*
     * 
     * @remark
     *      Данная функция не беспокоится о выравнивании данных.
     *      Будет ли блок памяти выровнен или нет зависит от реализации класса наследника.
     *      Чтобы аллоцировать блок с выравниванием используйте tca::base_allocator::allocate_align(std::size_t, sd::size_t);
     * 
     * @param sz
     *      Размер блока памяти.
     * 
     * @return 
     *      Указатель на начало блока.
     * 
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void* allocate(std::size_t sz);
    
    /**
     * Выделяет блок памяти размером sz и выравниванием align, 
     * и возвращает на него указатель void*
     * 
     * Данная функция обязана возвращать адрес выровненный не меньше, чем на align
     * 
     * @param sz
     *      Размер блока памяти.
     * 
     * @param align
     *      Выравнивание для выделяемого блока памяти.
     * 
     * @return 
     *      Указатель на начало блока.
     * 
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void* allocate_align(std::size_t sz, std::size_t align);
    
    /**
     * Особождает выделенную память.
     * 
     * @param ptr
     *      Указатель на выделенный ранее блок памяти.
     * 
     * @param sz
     *      Размер выделенного ранее блока памяти.
     * 
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void deallocate(void* ptr, std::size_t sz);
};

}


#endif//_ALLOCATORS_BASE_ALLOCATOR_H