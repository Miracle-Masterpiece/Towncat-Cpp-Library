#ifndef _ALLOCATORS_LINEAR_ALLOCATOR_H
#define _ALLOCATORS_LINEAR_ALLOCATOR_H

#include <allocators/allocator.hpp>

namespace tca
{

/**
 * Линейный распределитель памяти, который линейно выделяет память.
 * При этом освободить память можно только сбросом всего аллокатора.
 * 
 * @since 1.0
 */
class linear_allocator : public allocator {
    /**
     * Указатель на блок памяти аллокатора.
     */
    void* _buffer;
    
    /**
     * Общий объём доступной памяти.
     */
    std::size_t _capacity;
    
    /**
     * Смещение для нового выделения блока.
     */
    std::size_t _offset;
    
    /**
     * 
     */
    linear_allocator(const linear_allocator&);
    
    /**
     * 
     */
    linear_allocator& operator= (const linear_allocator&);
public:
    
    /**
     * 
     */
    linear_allocator();

    /**
     * @param size
     *      Размер выделенной арены.
     * 
     * @param allocator
     *      Аллокатор для выделения памяти для этого линейного аллокатора.
     */
    linear_allocator(std::size_t size, base_allocator* allocator = get_scoped_or_default());    
    
    /**
     * Создаёт обёртку вокруг уже созданной памяти.
     * 
     * @param base_buffer
     *      Указатель на блок памяти, который будет использоваться в качестве памяти для этого линейного аллокатора.
     * 
     * @param capacity
     *      Размер блока памяти.
     */
    linear_allocator(void* base_buffer, std::size_t capacity);
    
    /**
     * 
     */
    linear_allocator(linear_allocator&&);
    
    /**
     * 
     */
    linear_allocator& operator= (linear_allocator&&);
    
    /**
     * 
     */
    ~linear_allocator();
    
    /**
     * Выделяет блок памяти размером sz с выравниванием alignof(char)
     * 
     * @param sz
     *      Размер запрашиваемой памяти.     
     * 
     * @return
     *      Указатель на блок памяти или nullptr, если выделение не удалось.
     */
    void* allocate(std::size_t sz) override;
    
    /**
     * Выделяет блок памяти размером sz с выравниванием align
     * 
     * @param sz
     *      Размер запрашиваемой памяти.     
     * 
     * @param align
     *      Размер выравнивания для запрашиваемой памяти.
     * 
     * @return
     *      Указатель на блок памяти или nullptr, если выделение не удалось.
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;
    
    /**
     * 
     */
    void deallocate(void* ptr) override;
    
    /**
     * Возвращает строковое представление объекта.
     * 
     * @param buf
     *      Указатель на буфер в который будет записана строка.
     * 
     * @param buf_size
     *      Размер передаваемого буфера.
     * 
     * @return
     *      Сколько записано символов (Не включая нуль-терминатор).
     */
    int to_string(char buf[], std::size_t buf_size) const;
    
    /**
     * Возвращает распределитель в изначальное состояние.
     */
    void reset() {
        _offset = 0;
    }
    
    /**
     * Возвращает текущее смещение от начала памяти.
     * 
     * @return
     *      Смещение от начала блока памяти. (Сколько байт было выделено).
     */
    std::size_t position() {
        return _offset;
    }
    
    /**
     * Распечатывает отладочную информацию об этом распределителе.
     */
    void print() const;
};

}


#endif//_ALLOCATORS_LINEAR_ALLOCATOR_H
