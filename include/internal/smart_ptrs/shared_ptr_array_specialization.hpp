

#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#include <internal/smart_ptrs/shared_ptr_t.hpp>
#include <cpp/lang/utils/arrays.hpp>

#if 1
namespace tc
{

template<typename T>
class shared_ptr<T[]>;

template<typename T>
class weak_ptr<T[]> {
    friend class shared_ptr<T[]>;

    /**
     * 
     */
    template<typename A>
    typename enable_if< is_array<A>::value, shared_ptr<A> >::type allocate_shared(tca::allocator*, std::size_t, const typename remove_extent<A>::type&);

    /**
     * 
     */
    internal::control_block* m_control;

    /**
     * Указатель на первый элемент массива.
     */
    T* m_arr;

    /**
     * Конструктор для внутреннего API!
     * 
     * @param arr
     *      Указатель на первый элемент массива, владеть которым будет этот shared_ptr.
     * 
     * @param length
     *      Длина массива, на будет указывать эта слабая ссылка.
     * 
     * Данный конструктор должен увеличивать счётчик weak-reference.
     */
    weak_ptr(T* arr, internal::control_block* block);
    
    /**
     * Выполняет очищение объекта.
     */
    void cleanup();

public:
    /**
     * Конструктор по умолчанию. Создаёт пустой weak_ptr.
     */
    weak_ptr();

    /**
     * Конструктор копирования. Увеличивает счётчик слабых ссылок.
     * 
     * @param other 
     *      Другой weak_ptr.
     */
    weak_ptr(const weak_ptr<T[]>& other);

    /**
     * Конструктор перемещения. Забирает владение у другого weak_ptr.
     * 
     * @param other 
     *      Другой weak_ptr.
     */
    weak_ptr(weak_ptr<T[]>&& other);

    /**
     * Деструктор. 
     * Уменьшает счётчик слабых ссылок.
     */
    ~weak_ptr();

    /**
     * Оператор присваивания (копирование).
     * 
     * @param other 
     *      Другой weak_ptr.
     * 
     * @return 
     *      *this.
     */
    weak_ptr<T[]>& operator=(const weak_ptr<T[]>& other);

    /**
     * Оператор присваивания (перемещение).
     * 
     * @param other 
     *      Другой weak_ptr.
     * 
     * @return 
     *      *this.
     */
    weak_ptr<T[]>& operator=(weak_ptr<T[]>&& other);

    /**
     * Возвращает количество активных shared_ptr, владеющих объектом.
     * 
     * @return 
     *      Счётчик сильных ссылок.
     */
    std::size_t use_count() const;

    /**
     * Создаёт shared_ptr из weak_ptr, если объект всё ещё существует.
     * 
     * @return 
     *      Новый shared_ptr или пустой, если объект уничтожен.
     */
    shared_ptr<T[]> lock() const;
};

template<typename T>
class shared_ptr<T[]> {

    /**
     * 
     */
    template<typename A>
    friend class shared_ptr;
    
    /**
     * 
     */
    friend class weak_ptr<T[]>;
    
    /**
     * 
     */
    internal::control_block* m_control;

    /**
     * Указатель на управляющий блок.
     */
    T* m_arr;

    /**
     * Выполняет очищение объекта.
     */
    void cleanup();
    
    /**
     * Проверяет доступность объекта.
     * 
     * @throws null_pointer_exception
     *      Если указатель на блок равен nullptr.
     * 
     * @throw illegal_state_exception
     *      Если значение strong-reference внутри управляющего блока равно 0.     
     */
    void check_access() const;

    /**
     * 
     * 
     */
    explicit shared_ptr(T* arr, internal::control_block* block);

public:
    /**
     * Создаёт нулевой shared_ptr.
     */
    shared_ptr();

    /**
     * Создаёт shared_ptr, выделяет память и инициализирует объекты.
     * 
     * @param p
     *      Указатель на ранее выделенный массив.
     * 
     * @param deleter
     *      Объект, отвечающий за освобождение передаваемого указателя.
     * 
     * @param allocator
     *      Аллокатор для выделения под контролирующий блок.
     */
    template<typename DELETER>
    shared_ptr(T* p, DELETER deleter, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Конструктор копирования. 
     * Увеличивает счётчик сильных ссылок.
     */
    shared_ptr(const shared_ptr<T[]>& other);

    /**
     * Конструктор перемещения. 
     * Забирает владение у другого shared_ptr.
     * 
     * @param 
     *      other Другой shared_ptr.
     */
    shared_ptr(shared_ptr<T[]>&& other);

    /**
     * Оператор присваивания (копирование). Увеличивает счётчик сильных ссылок.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T[]>& operator=(const shared_ptr<T[]>& other);

    /**
     * Оператор присваивания (перемещение). Забирает владение у другого shared_ptr.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T[]>& operator=(shared_ptr<T[]>&& other);

    /**
     * Деструктор. 
     * Уменьшает счётчик сильных ссылок. При обнулении уничтожает объект.
     */
    ~shared_ptr();

    /**
     * Возвращает сырой указатель на первый объект массива.
     * 
     * @return 
     *      Указатель или nullptr.
     */
    T* get() const;

    /**
     * Проверка на инициализацию.
     * 
     * @return 
     *      true, если shared_ptr указывает на объект.
     */
    operator bool() const;

    /**
     * Оператор индексирования по массиву.
     * 
     * @param idx
     *      Индекс массива.     
     * 
     * @throws jstd::null_pointer_exception
     *      Если указатель нулевой.
     * 
     * @return
     *      Ссылку на объект, хранящийся по индексу.
     */
    T& operator[] (std::size_t idx) const;

    /**
     * Приводит shared_ptr к сырому указателю.
     * 
     * @return 
     *      Значение указателя.
     */
    operator T*() const;

    /**
     * Возвращает weak_ptr для этого shared_ptr
     * 
     * @return 
     *      weak_ptr, указывающий на этот объект.
     */
    operator weak_ptr<T[]>() const;

    /**
     * Возвращает количество shared_ptr, владеющих объектом.
     * 
     * @return 
     *      Счётчик сильных ссылок.
     */
    std::size_t use_count() const;

    /**
     * Создаёт weak_ptr, наблюдающий за тем же объектом.
     * 
     * @return 
     *      Новый weak_ptr.
     */
    weak_ptr<T[]> get_weak() const;
};

    template<typename T>
    shared_ptr<T[]>::shared_ptr() : m_control(nullptr), m_arr(nullptr) {

    }

    template<typename T>
    template<typename DELETER>
    shared_ptr<T[]>::shared_ptr(T* arr, DELETER deleter, tca::allocator* allocator) {
        typedef internal::ptr_control_block<T, DELETER> control_block_type;

        control_block_type* control = (control_block_type*) allocator->allocate_align(sizeof(control_block_type), alignof(control_block_type));
        if (!control)
            throw_except<out_of_memory_error>("out of memory");

        try {
            m_control = new (control) control_block_type(arr, std::move(deleter), allocator);
            m_arr     = control->get_object();
        } catch (...) {
            allocator->deallocate(control);
            throw;
        }
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(T* arr, internal::control_block* control) : m_control(control), m_arr(arr) {
        
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(const shared_ptr<T[]>& ptr) : m_control(ptr.m_control), m_arr(ptr.m_arr) {
        if (m_control)
        {
            m_control->inc_strong();
        }
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(shared_ptr<T[]>&& ptr) : m_control(ptr.m_control), m_arr(ptr.m_arr) {
        ptr.m_arr     = nullptr;
        ptr.m_control = nullptr;
    }


    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(const shared_ptr<T[]>& ptr) {
        if (&ptr != this)
        {
            m_arr     = ptr.m_arr;
            m_control = ptr.m_control;
            cleanup();
            if (m_control)
            {
                m_control->inc_strong();
            }
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(shared_ptr<T[]>&& ptr) {
        if (&ptr != this)
        {
            std::swap(m_control, ptr.m_control);
            std::swap(m_arr,     ptr.m_arr);
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T[]>::cleanup() {
        if (m_control)
        {
            if (m_control->dec_strong() == 0)
            {
                m_control->destroy_object();
                if (m_control->dec_weak() == 0)
                {
                    m_control->destroy_control_block();
                }
            }
        }
    }

    template<typename T>
    void shared_ptr<T[]>::check_access() const {
        JSTD_DEBUG_CODE(
            if (m_arr == nullptr)
                throw_except<null_pointer_exception>("pointer must be != null");
        );
    }

    template<typename T>
    shared_ptr<T[]>::operator bool() const {
        return m_arr != nullptr;
    }
    
    template<typename T>
    T& shared_ptr<T[]>::operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE
        (
            check_access();
        )
        return m_arr[idx];
    }

    template<typename T>
    shared_ptr<T[]>::operator T*() const {
        JSTD_DEBUG_CODE
        (
            check_access();
        )
        return m_arr;
    }

    #if 0
    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T[]>::const_pointer_cast() const {
        static_assert(is_same<typename remove_cv<T[]>::type, typename remove_cv<E>::type>::value, "=== Type cast error! ===");
        if (m_control)
        {
        
            return shared_ptr<E>(const_cast<E*>(m_arr), m_len);
        }
        return shared_ptr<E>();
    }
    #endif

    template<typename T>
    shared_ptr<T[]>::operator weak_ptr<T[]>() const {
        return get_weak();
    }

    template<typename T>
    weak_ptr<T[]> shared_ptr<T[]>::get_weak() const {
        if (m_control)
        {
            return weak_ptr<T[]>(m_arr, m_control);
        }
        return weak_ptr<T[]>();
    }

    template<typename T>
    std::size_t shared_ptr<T[]>::use_count() const {
        if (m_control)
        {
            m_control->get_strong();
        }
        return 0;
    }

    /**
     * ############################################################################################
     * ############################################################################################
     * ############################################################################################
     */

    template<typename T>
    weak_ptr<T[]>::weak_ptr(T* arr, internal::control_block* control) : m_control(control), m_arr(arr) {
        if (m_control)
        {
            m_control->inc_weak();
        }
    }
    
    template<typename T>
    void weak_ptr<T[]>::cleanup() {
        if (m_control)
        {
            if (m_control->dec_weak() == 0)
            {
                m_control->destroy_control_block();
            }
        }
    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr() : m_control(nullptr), m_arr(nullptr) {

    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr(const weak_ptr<T[]>& ptr) : m_control(ptr.m_control), m_arr(ptr.m_arr) {
        if (m_control)
        {
            m_control->inc_weak();
        }
    }
    
    template<typename T>
    weak_ptr<T[]>::weak_ptr(weak_ptr<T[]>&& ptr) : m_control(ptr.m_control), m_arr(ptr.m_arr) {
        ptr.m_arr       = nullptr;
        ptr.m_control   = nullptr;
    }
    
    template<typename T>
    weak_ptr<T[]>::~weak_ptr() {
        cleanup();
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(const weak_ptr<T[]>& ptr) {
        if (*ptr != nullptr) {
            cleanup();

            m_control   = ptr.m_control;
            m_arr       = ptr.m_arr;

            if (m_control)
            {
                m_control->inc_weak();
            }
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(weak_ptr<T[]>&& ptr) {
        if (&ptr != this)
        {
            std::swap(m_control, ptr.m_control);
            std::swap(m_arr,     ptr.m_arr);
        }
        return *this;
    }
    
    template<typename T>
    std::size_t weak_ptr<T[]>::use_count() const {
        if (m_control)
        {
            return m_control->get_strong();
        }
        return 0;
    }
    
    template<typename T>
    shared_ptr<T[]> weak_ptr<T[]>::lock() const {
        if (m_control)
        {
            if (m_control->try_inc_strong())
            {
                return shared_ptr<T[]>(m_arr, m_control);
            }
        }
        return shared_ptr<T[]>();
    }
}

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H

#endif