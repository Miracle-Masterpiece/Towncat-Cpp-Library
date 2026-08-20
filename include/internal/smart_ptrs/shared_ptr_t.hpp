#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H

#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstdint>
#include <utility>
#include <cstdio>
#include <new>
#include <cassert>
#include <typeinfo>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/traits/sfinae.hpp>

namespace tc 
{

namespace internal
{

typedef typename int_of<16>::utype toffset;
typedef typename int_of<32>::utype tcounter;

struct ctrl_block {
    tca::allocator* m_alloc;
    tcounter        m_strong;
    tcounter        m_weak;
    toffset         m_off;
};

    /**
     * 
     */
    void* alloc_control_block(std::size_t t_size, std::size_t t_align, tca::allocator* alloc);
    
    /**
     * 
     */
    ctrl_block* get_ctrlblock(void* p) ;
    
    /**
     * 
     */
    void free_ctrlblock(void* p);
    
    /**
     * 
     */
    template<typename T, typename T_>
    T* allocate_memory_for_shared_ptr(T_&& val, tca::allocator* alloc) {
        void* mem = alloc_control_block(sizeof(T), alignof(T), alloc);
        
        if(!mem)
        {
            throw_except<out_of_memory_error>("out of memory");
        }
        
        try {
            new (mem) T(std::forward<T_>(val));
        } catch (...) {
            free_ctrlblock(mem);
            throw;
        }
        
        return static_cast<T*>(mem);
    }

} //namespace internal


template<typename T>
class shared_ptr;

#if 1
template<typename T>
class weak_ptr {
    
    /**
     * 
     */
    friend class shared_ptr<T>;
    
    /**
     * 
     */
    typedef typename remove_cv<T>::type Tvalue;

    /**
     * Указатель на объект.
     */
    Tvalue* m_obj;

    /**
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор должен увеличивать счётчик weak-reference.
     */
    weak_ptr(Tvalue* obj);
    
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
    weak_ptr(const weak_ptr<T>& other);

    /**
     * Конструктор перемещения. Забирает владение у другого weak_ptr.
     * 
     * @param other 
     *      Другой weak_ptr.
     */
    weak_ptr(weak_ptr<T>&& other);

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
    weak_ptr<T>& operator=(const weak_ptr<T>& other);

    /**
     * Оператор присваивания (перемещение).
     * 
     * @param other 
     *      Другой weak_ptr.
     * 
     * @return 
     *      *this.
     */
    weak_ptr<T>& operator=(weak_ptr<T>&& other);

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
    shared_ptr<T> lock() const;

};
#endif

template<typename T>
class shared_ptr {
    
    /**
     * 
     */
    typedef typename remove_cv<T>::type Tvalue;
    
    /**
     * 
     */
    template<typename A>
    friend class shared_ptr;
    
    #if 0
    /**
     * 
     */
    friend class weak_ptr<T>;
    #endif

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> static_pointer_cast(const shared_ptr<B>&);

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> const_pointer_cast(const shared_ptr<B>&);

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> dynamic_pointer_cast(const shared_ptr<B>&);

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> reinterpret_pointer_cast(const shared_ptr<B>&);

    /**
     * Указатель на управляющий блок.
     */
    Tvalue* m_obj;
    
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
     * !@internal
     * 
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор увеличивает счётчик strong-reference, если ctrl_block != nullptr
     */
    explicit shared_ptr(Tvalue* obj);

public:
    /**
     * Конструктор по умолчанию. Создаёт пустой shared_ptr.
     */
    shared_ptr();

    /**
     * Конструктор с размещением объекта.
     * 
     * @tparam _T 
     *      Тип передаваемого объекта.
     * 
     * @param allocator 
     *      Аллокатор для размещения объекта и блока управления.
     * 
     * @param obj 
     *      Объект, который будет скопирован или перемещён.
     */
    template<typename T_, typename = typename enable_if<
                                                is_same<
                                                        typename remove_cv<T>::type, 
                                                        typename remove_cv<T_>::type
                                                >::value
                                            >::type>
    shared_ptr(T_&& obj, tca::allocator* allocator);

    /**
     * Конструктор копирования. 
     * Увеличивает счётчик сильных ссылок.
     */
    shared_ptr(const shared_ptr<T>& other);

    /**
     * Конструктор копирования. 
     * Увеличивает счётчик сильных ссылок.
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr(const shared_ptr<E>& other);

    /**
     * Конструктор перемещения. 
     * Забирает владение у другого shared_ptr.
     * 
     * @param 
     *      other Другой shared_ptr.
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr(shared_ptr<E>&& other);

    /**
     * Оператор присваивания (копирование). 
     * Увеличивает счётчик сильных ссылок.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T>& operator=(const shared_ptr<T>& other);

    /**
     * Оператор перемещение. Забирает владение у другого shared_ptr.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T>& operator=(shared_ptr<T>&& other);

    /**
     * Оператор присваивания (копирование). Увеличивает счётчик сильных ссылок.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr<T>& operator=(const shared_ptr<E>& other);

    /**
     * Оператор перемещение. Забирает владение у другого shared_ptr.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr<T>& operator=(shared_ptr<E>&& other);

    /**
     * Деструктор. 
     * Уменьшает счётчик сильных ссылок. При обнулении уничтожает объект.
     */
    ~shared_ptr();

    /**
     * Разыменование указателя.
     * 
     * @return 
     *      Ссылка на объект.
     * 
     * @throws null_pointer_exception 
     *      Eсли объект не инициализирован.
     */
    T& operator*() const;

    /**
     * Доступ к членам объекта.
     * 
     * @return 
     *      Указатель на объект.
     * 
     * @throws null_pointer_exception 
     *      Eсли объект не инициализирован.
     */
    T* operator->() const;

    /**
     * Возвращает сырой указатель на объект.
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
    operator weak_ptr<T>() const;

    /**
     * Создаёт weak_ptr, наблюдающий за тем же объектом.
     * 
     * @return 
     *      Новый weak_ptr.
     */
    weak_ptr<T> get_weak() const;

    /**
     * Возвращает количество shared_ptr, владеющих объектом.
     * 
     * @return 
     *      Счётчик сильных ссылок.
     */
    std::size_t use_count() const;

    /**
     * Сравнивает значения указателей.
     * 
     * @return
     *      true тогда, и только тогда, когда оба shared_ptr указывают на один блок памяти.
     *      в остальных случаях всегда false.
     */
    template<typename E>
    bool operator== (const shared_ptr<E>& ptr) const {
        return ptr.get() == get();
    }
};


    template<typename T>
    shared_ptr<T>::shared_ptr() : m_obj(nullptr) {

    }
    
    template<typename T>
    shared_ptr<T>::shared_ptr(Tvalue* obj) : m_obj(obj) {
        if (m_obj != nullptr)
        {
            internal::get_ctrlblock(m_obj)->m_strong++;
        }
    }

    template<typename T>
    template<typename T_, typename>
    shared_ptr<T>::shared_ptr(T_&& obj, tca::allocator* allocator) {
        m_obj = internal::allocate_memory_for_shared_ptr<Tvalue>(std::forward<T_>(obj), allocator);
        internal::get_ctrlblock(m_obj)->m_strong++;
    }

    template<typename T>
    shared_ptr<T>::shared_ptr(const shared_ptr<T>& ptr) : m_obj(ptr.m_obj) {
        if (m_obj != nullptr)
        {
            internal::get_ctrlblock(m_obj)->m_strong++;
        }
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(const shared_ptr<E>& other) : m_obj(other.m_obj) {
        if (m_obj != nullptr)
        {
            internal::get_ctrlblock(m_obj)->m_strong++;
        }
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(shared_ptr<E>&& ptr) : m_obj(ptr.m_obj) {
        ptr.m_obj = nullptr;
    }
    
    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (const shared_ptr<E>& ptr) {
        if (&ptr != this) {
            cleanup();
            m_obj = ptr.m_obj;
            if (m_obj)
            {
                internal::get_ctrlblock(m_obj)->m_strong++;
            }
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& other) {
        if (&other != this) {
            cleanup();
            m_obj = other.m_obj;
            if (m_obj)
            {
                internal::get_ctrlblock(m_obj)->m_strong++;
            }
        }
        return *this;
    }
    
    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>&& other) {
        if (&other != this) {
            cleanup();
            m_obj       = other.m_obj;
            other.m_obj = nullptr;
        }
        return *this;
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (shared_ptr<E>&& ptr) {
        if (ptr.m_obj != m_obj)
        {
            cleanup();
            m_obj     = ptr.m_obj;
            ptr.m_obj = nullptr;
        }
        return *this;
    }

    template<typename T>
    void shared_ptr<T>::cleanup() {
        if (m_obj)
        {
            internal::ctrl_block* block = internal::get_ctrlblock(m_obj);
            assert(block->m_strong > 0);
            block->m_strong--;
            if (block->m_strong == 0)
            {
                m_obj->~T();
                if (block->m_weak == 0)
                {
                    internal::free_ctrlblock(m_obj);
                }
            }
        }
    }
    
    template<typename T>
    shared_ptr<T>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T>::check_access() const {
        JSTD_DEBUG_CODE(
            if (!m_obj)
                throw_except<null_pointer_exception>("pointer must be != null");
        );
    }

    template<typename T>
    shared_ptr<T>::operator T*() const {
        check_access();
        return m_obj;
    }

    template<typename T>
    T& shared_ptr<T>::operator*() const {
        check_access();
        return *m_obj;
    }
    
    template<typename T>
    T* shared_ptr<T>::operator->() const {
        check_access();
        return m_obj;
    }

    template<typename T>
    std::size_t shared_ptr<T>::use_count() const {
        if (m_obj)
        {
            return internal::get_ctrlblock(m_obj)->m_strong;
        }
        return 0;
    }

    template<typename T>
    shared_ptr<T>::operator bool() const {
        return get() != nullptr;
    }

    template<typename T>
    T* shared_ptr<T>::get() const {
        if (m_obj)
        {
            return m_obj;
        }
        return nullptr;
    }

    template<typename T>
    shared_ptr<T>::operator weak_ptr<T>() const {
        return get_weak();
    }
    
    template<typename T>
    weak_ptr<T> shared_ptr<T>::get_weak() const {
        return weak_ptr<T>(m_obj);
    }

    template<typename A, typename B, typename = typename enable_if<is_related<B, A>::value && is_cv_castable<B, A>::value>::type>
    shared_ptr<A> static_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(p.m_obj);
    }

    template<typename A, typename B, typename = typename enable_if<
                                                            is_same<
                                                                    typename remove_cv<A>::type, 
                                                                    typename remove_cv<B>::type
                                                            >::value
                                                        >::type>
    shared_ptr<A> const_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(p.m_obj);
    }
    
    template<typename A, typename B, typename = typename enable_if<is_related<A, B>::value && is_cv_castable<B, A>::value>::type>
    shared_ptr<A> dynamic_pointer_cast(const shared_ptr<B>& p) {
        B* const object = p.get();
        JSTD_DEBUG_CODE(check_non_null(object));
        if (!dynamic_cast<A*>(object))
            throw_except<class_cast_exception>("Where [To = %s, From = %s]", typeid(A).name(), typeid(*object).name());
        return shared_ptr<A>(
                                static_cast<shared_ptr<A>::Tvalue*>(p.m_obj)
                            );
    }

    template<typename A, typename B, typename = typename enable_if<is_cv_castable<B, A>::value>::type>
    shared_ptr<A> reinterpret_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(
                                static_cast<shared_ptr<A>::Tvalue*>(p.m_obj)
                            );
    }

    /**
     * #####################################################################################
     * 
     *                                  W E A K _ P T R
     * 
     * #####################################################################################
     */

    template<typename T>
    weak_ptr<T>::weak_ptr(Tvalue* obj) : m_obj(obj) {
        if (m_obj)
        {
            internal::get_ctrlblock(obj)->m_weak++;
        }
    }
    
    template<typename T>
    void weak_ptr<T>::cleanup() {
        if (m_obj)
        {
            internal::ctrl_block* block = internal::get_ctrlblock(m_obj);
            
            assert(block->m_weak > 0);
            block->m_weak--;
        
            if (block->m_strong == 0 && block->m_weak == 0)
            {
                internal::free_ctrlblock(m_obj);
            }
        }
    }

    template<typename T>
    weak_ptr<T>::weak_ptr() : m_obj(nullptr) {

    }
    
    template<typename T>
    weak_ptr<T>::~weak_ptr() {
        cleanup();
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) : m_obj(ptr.m_obj) {
        if (m_obj != nullptr)
        {
            internal::get_ctrlblock(m_obj)->m_weak++;
        }
    }
    
    template<typename T>
    weak_ptr<T>::weak_ptr(weak_ptr<T>&& ptr) : m_obj(ptr.m_obj) {
        ptr.m_obj= nullptr;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<T>& ptr) {
        if (&ptr != this) 
        {
            cleanup();
            m_obj = ptr.m_obj;
            if (m_obj != nullptr)
            {
                internal::get_ctrlblock(m_obj)->m_weak++;
            }
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<T>&& ptr) {
        if (&ptr != this)
        {
            cleanup();
            m_obj       = ptr.m_obj;
            ptr.m_obj   = nullptr;
        }
        return *this;
    }

    template<typename T>
    std::size_t weak_ptr<T>::use_count() const {
        if (m_obj)
        {
            internal::get_ctrlblock(m_obj)->m_strong;
        }
        return 0;
    }
    
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock() const {
        if (m_obj)
        {
            if (internal::get_ctrlblock(m_obj)->m_strong > 0)
            {
                return shared_ptr<T>(m_obj);
            }
        }
        return shared_ptr<T>();
    }

    template<typename T>
    weak_ptr<T> make_weak() {
        return weak_ptr<T>();
    }

} //namespace jstd

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H