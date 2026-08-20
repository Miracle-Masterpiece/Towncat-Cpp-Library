

#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#include <internal/smart_ptrs/shared_ptr_t.hpp>
#include <cpp/lang/utils/arrays.hpp>

namespace tc
{

namespace internal
{
    template<typename T, typename T_>
    T* allocate_memory_for_shared_ptr_array(std::size_t len, tca::allocator* alloc, const T_& val) {
        void* mem = alloc_control_block(sizeof(T) * len, alignof(T), alloc);
        
        if(!mem)
        {
            throw_except<out_of_memory_error>("out of memory");
        }
        
        try {
            uninitialized_construct_n<T>(static_cast<T*>(mem), len, val);
        } catch (...) {
            free_ctrlblock(mem);
            throw;
        }
        
        return static_cast<T*>(mem);
    }
}

template<typename T>
class shared_ptr<T[]>;

template<typename T>
class weak_ptr<T[]> {
    friend class shared_ptr<T[]>;
    
    typedef typename remove_cv<T>::type Tvalue;

    /**
     * Указатель на первый элемент массива.
     */
    Tvalue* m_arr;

    /**
     * Размер массива, на который указывает этот слабый указатель.
     */
    std::size_t m_len;

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
    weak_ptr(Tvalue* arr, std::size_t length);
    
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
    typedef typename remove_cv<T>::type Tvalue;

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
     * Указатель на управляющий блок.
     */
    Tvalue* m_arr;
    
    /**
     * Размер массива.
     */
    std::size_t m_len;

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

public:
    /**
     * Создаёт нулевой shared_ptr.
     */
    shared_ptr();

    /**
     * Создаёт shared_ptr, выделяет память и инициализирует объекты.
     * 
     * @param val
     *      Значение по-умолчанию для всех элементов массива.
     * 
     * @param length
     *      Длина массива объектов.
     * 
     * @param allocator
     *      Аллокатор для выделения памяти.
     * 
     */
    shared_ptr(std::size_t length, const T& val, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * !@internal
     * 
     * Конструктор для внутреннего API!
     * 
     * @param arr
     *      Указатель на первый элемент массива, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор увеличивает счётчик strong-reference, если ctrl_block != nullptr
     */
    explicit shared_ptr(Tvalue* arr, std::size_t len);

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

    /**
     * Преобразует shared_ptr<T[]> в shared_ptr<E[]> с использованием const_cast.
     * 
     * @tparam E 
     *      Целевой тип.
     * 
     * @return 
     *      Новый shared_ptr с преобразованным типом.
     */
    template<typename E>
    shared_ptr<E> const_pointer_cast() const;

    /**
     * Возвращает длину массива.
     */
    std::size_t length() const;
};

    template<typename T>
    shared_ptr<T[]>::shared_ptr() : m_arr(nullptr), m_len(0) {

    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(std::size_t length, const T& val, tca::allocator* allocator) : m_arr(nullptr), m_len(length) {
        m_arr = internal::allocate_memory_for_shared_ptr_array<Tvalue>(length, allocator, val);
        internal::get_ctrlblock(m_arr)->m_strong++;
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(Tvalue* arr, std::size_t len) : m_arr(arr), m_len(len) {
        if (m_arr)
        {
            internal::get_ctrlblock(m_arr)->m_strong++;
        }
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(const shared_ptr<T[]>& other) : shared_ptr<T[]>(other.m_arr, other.m_len) {
        
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(shared_ptr<T[]>&& other) : m_arr(other.m_arr), m_len(other.m_len) {
        other.m_arr = nullptr;
        other.m_len = 0;
    }


    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(const shared_ptr<T[]>& other) {
        if (&other != this)
        {
            cleanup();
            m_arr = other.m_arr;
            m_len = other.m_len;
            if (m_arr)
            {
                internal::get_ctrlblock(m_arr)->m_strong++;
            }
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(shared_ptr<T[]>&& other) {
        if (&other != this)
        {
            cleanup();
            m_arr = other.m_arr;
            m_len = other.m_len;
            other.m_arr = nullptr;
            other.m_len = 0;
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T[]>::cleanup() {
        if (m_arr)
        {
            internal::ctrl_block* block = internal::get_ctrlblock(m_arr);
            
            assert(block->m_strong > 0);
            block->m_strong--;
        
            if (block->m_strong == 0)
            {
                destroy_n(m_arr, m_len);
                if (block->m_weak == 0)
                {
                    internal::free_ctrlblock(m_arr);
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
            check_index(idx, m_len);
        )
        return m_arr[idx];
    }

    template<typename T>
    shared_ptr<T[]>::operator T*() const {
        JSTD_DEBUG_CODE(
            check_access();
        )
        return m_arr;
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T[]>::const_pointer_cast() const {
        static_assert(is_same<typename remove_cv<T[]>::type, typename remove_cv<E>::type>::value, "=== Type cast error! ===");
        if (m_arr)
        {
        
            return shared_ptr<E>(m_arr, m_len);
        }
        return shared_ptr<E>();
    }

    template<typename T>
    shared_ptr<T[]>::operator weak_ptr<T[]>() const {
        return get_weak();
    }

    template<typename T>
    weak_ptr<T[]> shared_ptr<T[]>::get_weak() const {
        if (m_arr)
        {
            return weak_ptr<T[]>(m_arr, m_len);
        }
        return weak_ptr<T[]>();
    }

    template<typename T>
    std::size_t shared_ptr<T[]>::use_count() const {
        if (m_arr)
        {
            internal::get_ctrlblock(m_arr)->m_strong;
        }
        return 0;
    }

    template<typename T>
    std::size_t shared_ptr<T[]>::length() const {
        return m_len;
    }

    /**
     * ############################################################################################
     * ############################################################################################
     * ############################################################################################
     */

    template<typename T>
    weak_ptr<T[]>::weak_ptr(Tvalue* arr, std::size_t length) : m_arr(arr), m_len(length) {
        if (m_arr)
        {
            internal::get_ctrlblock(m_arr)->m_weak++;
        }
    }
    
    template<typename T>
    void weak_ptr<T[]>::cleanup() {
        if (m_arr)
        {
            internal::ctrl_block* block = internal::get_ctrlblock(m_arr);

            assert(block->m_weak > 0);
            block->m_weak--;

            if (block->m_strong == 0 && block->m_weak == 0)
            {
                internal::free_ctrlblock(m_arr);
            }
        }
    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr() : m_arr(nullptr), m_len(0) {

    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr(const weak_ptr<T[]>& other) : m_arr(other.m_arr), m_len(other.m_len) {
        if (m_arr)
        {
            internal::get_ctrlblock(m_arr)->m_weak++;
        }
    }
    
    template<typename T>
    weak_ptr<T[]>::weak_ptr(weak_ptr<T[]>&& other) : m_arr(other.m_arr), m_len(other.m_len) {
        other.m_arr = nullptr;
        other.m_len = 0;
    }
    
    template<typename T>
    weak_ptr<T[]>::~weak_ptr() {
        cleanup();
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(const weak_ptr<T[]>& other) {
        if (*other != nullptr) {
            cleanup();

            m_arr = other.m_arr;
            m_len = other.m_len;

            if (m_arr)
            {
                internal::get_ctrlblock(m_arr)->m_weak++;
            }
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(weak_ptr<T[]>&& other) {
        if (&other != this)
        {
            cleanup();
            m_arr = other.m_arr;
            m_len = other.m_len;

            other.m_arr = nullptr;
            other.m_len = 0;
        }
        return *this;
    }
    
    template<typename T>
    std::size_t weak_ptr<T[]>::use_count() const {
        if (m_arr)
        {
            return internal::get_ctrlblock(m_arr)->m_strong;
        }
        return 0;
    }
    
    template<typename T>
    shared_ptr<T[]> weak_ptr<T[]>::lock() const {
        if (m_arr)
        {
            if (internal::get_ctrlblock(m_arr)->m_strong > 0)
            {
                return shared_ptr<T[]>(m_arr, m_len);
            }
        }
        return shared_ptr<T[]>();
    }

    // template<typename T>
    // shared_ptr<T[]> make_shared<T[]>(std::size_t length, const T& val, tca::allocator* allocator) {
    //     return shared_ptr<T[]>(length, val, allocator);
    // }
}

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H