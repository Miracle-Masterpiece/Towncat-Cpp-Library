#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H
#include <internal/smart_ptrs/shared_ptr_t.hpp>

namespace tc
{

template<typename T>
class shared_ptr<T[]>;

template<typename T>
class weak_ptr<T[]> {
    friend class shared_ptr<T[]>;
    
    /**
     * Указатель на управляющий блок.
     */
    internal::sptr::shared_control_block* m_block;

    /**
     * Размер массива, на который указывает этот слабый указатель.
     */
    std::size_t m_len;

    /**
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr или nullptr.
     * 
     * @param length
     *      Длина массива, на будет указывать эта слабая ссылка.
     * 
     * Данный конструктор должен увеличивать счётчик weak-reference.
     */
    weak_ptr(internal::sptr::shared_control_block* ctrl_block, std::size_t length);
    
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
    template<typename A>
    friend class shared_ptr;
    friend class weak_ptr<T[]>;
    
    /**
     * Указатель на управляющий блок.
     */
    internal::sptr::shared_control_block* m_block;
    
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
     * 
     * @param allocator
     *      Распределитель для выделения памяти.
     * 
     * @param length
     *      Длина массива объектов.
     */
    shared_ptr(tca::allocator* allocator, std::size_t length);

    /**
     * !@internal
     * 
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор увеличивает счётчик strong-reference, если ctrl_block != nullptr
     */
    explicit shared_ptr(internal::sptr::shared_control_block* ctrl_block, std::size_t len);

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
    shared_ptr<T[]>::shared_ptr() : m_block(nullptr), m_len(0) {

    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(tca::allocator* allocator, std::size_t length) : m_block(nullptr), m_len(length) {
        m_block = internal::sptr::make_control_block_array<T>(allocator, length);
        if (m_block == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
        m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(internal::sptr::shared_control_block* ctrl_block, std::size_t len) : m_block(ctrl_block), m_len(len) {
        if (m_block)
            m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(const shared_ptr<T[]>& other) : m_block(other.m_block), m_len(other.m_len) {
        if (m_block)
            m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T[]>::shared_ptr(shared_ptr<T[]>&& other) : m_block(other.m_block), m_len(other.m_len) {
        other.m_block   = nullptr;
        other.m_len     = 0;
    }


    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(const shared_ptr<T[]>& other) {
        if (&other != this) {
            cleanup();
            m_block = other.m_block;
            m_len   = other.m_len;
            if (m_block)
                m_block->inc_strong_ref();
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>& shared_ptr<T[]>::operator=(shared_ptr<T[]>&& other) {
        if (&other != this) {
            cleanup();
            m_block = other.m_block;
            m_len   = other.m_len;
            other.m_block   = nullptr;
            other.m_len     = 0;
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T[]>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T[]>::cleanup() {
        if (!m_block)
            return;
        m_block->dec_strong_ref();
        if (m_block->strong_count() == 0) {
            T* obj = reinterpret_cast<T*>(m_block->m_object);
            assert(obj != nullptr);
            destroy_n(obj, m_len);
            if (m_block->weak_count() == 0) {
                tca::allocator* allocator = m_block->m_allocator;
                if (allocator != nullptr) //если распределитель равен null, значит за выделение контрол блока отвечает shared_ptr
                    allocator->deallocate(m_block, m_block->m_blocksize);
            }
        }
    }

    template<typename T>
    void shared_ptr<T[]>::check_access() const {
        JSTD_DEBUG_CODE(
            if (m_block == nullptr)
                throw_except<null_pointer_exception>("pointer must be != null");
        );
    }

    template<typename T>
    T* shared_ptr<T[]>::get() const {
        check_access();
        return reinterpret_cast<T*>(m_block->m_object);
    }

    template<typename T>
    shared_ptr<T[]>::operator bool() const {
        return m_block != nullptr;
    }
    
    template<typename T>
    T& shared_ptr<T[]>::operator[] (std::size_t idx) const {
        check_access();
        check_index(idx, m_len);
        return reinterpret_cast<T*>(m_block->m_object)[idx];
    }

    template<typename T>
    shared_ptr<T[]>::operator T*() const {
        check_access();
        return reinterpret_cast<T*>(m_block->m_object);
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T[]>::const_pointer_cast() const {
        static_assert(is_same<typename remove_cv<T[]>::type, typename remove_cv<E>::type>::value, "=== Type cast error! ===");
        if (!m_block || !m_block->m_object)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block, m_len);
    }

    template<typename T>
    shared_ptr<T[]>::operator weak_ptr<T[]>() const {
        return get_weak();
    }

    template<typename T>
    weak_ptr<T[]> shared_ptr<T[]>::get_weak() const {
        if (!m_block)
            return weak_ptr<T[]>();
        return weak_ptr<T[]>(m_block, m_len);
    }

    template<typename T>
    std::size_t shared_ptr<T[]>::use_count() const {
        if (!m_block) return 0;
        return m_block->m_strong_refs;
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
    weak_ptr<T[]>::weak_ptr(internal::sptr::shared_control_block* ctrl_block, std::size_t length) :
    m_block(ctrl_block), m_len(length) {
        if (m_block)
            m_block->inc_weak_ref();
    }
    
    template<typename T>
    void weak_ptr<T[]>::cleanup() {
        if (!m_block)
            return;
        m_block->dec_weak_ref();
        if (m_block->strong_count() == 0 && m_block->weak_count() == 0) {
            m_block->~shared_control_block();
            tca::allocator* allocator = m_block->m_allocator;
            if (allocator)
                allocator->deallocate(m_block, m_block->m_blocksize);
        }
    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr() : m_block(nullptr), m_len(0) {

    }

    template<typename T>
    weak_ptr<T[]>::weak_ptr(const weak_ptr<T[]>& other) : m_block(other.m_block), m_len(other.m_len) {
        if (m_block)
            m_block->inc_weak_ref();
    }
    
    template<typename T>
    weak_ptr<T[]>::weak_ptr(weak_ptr<T[]>&& other) : m_block(other.m_block), m_len(other.m_len) {
        other.m_block   = nullptr;
        other.m_len     = 0;
    }
    
    template<typename T>
    weak_ptr<T[]>::~weak_ptr() {
        cleanup();
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(const weak_ptr<T[]>& other) {
        if (*other != nullptr) {
            cleanup();
            other.m_block->inc_weak_ref();
            m_block = other.m_block;
            m_len   = other.m_len;
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T[]>& weak_ptr<T[]>::operator=(weak_ptr<T[]>&& other) {
        if (*other != nullptr) {
            cleanup();
            m_block = other.m_block;
            m_len   = other.m_len;

            other.m_block   = nullptr;
            other.m_len     = 0;
        }
        return *this;
    }
    
    template<typename T>
    std::size_t weak_ptr<T[]>::use_count() const {
        if (!m_block)
            return 0;
        return m_block->strong_count();
    }
    
    template<typename T>
    shared_ptr<T[]> weak_ptr<T[]>::lock() const {
        if (!m_block || m_block->strong_count() == 0)
            return shared_ptr<T[]>();
        return shared_ptr<T[]>(m_block, m_len);
    }

    // template<typename T>
    // shared_ptr<T[]> make_shared_array(uint32_t length, tca::allocator* allocator) {
    //     return shared_ptr<T[]>(allocator, length);
    // }
}

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_ARRAY_SPECIALIZATION_H