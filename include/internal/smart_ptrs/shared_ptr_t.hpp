#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H

#include <allocators/Helpers.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstdint>
#include <utility>
#include <cstdio>
#include <new>
#include <cassert>
#include <cpp/lang/utils/arrays.hpp>
#include <typeinfo>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/traits/SFINAE.hpp>

namespace jstd 
{
namespace internal 
{
namespace sptr 
{

    /**
     * @internal
     * @private
     */
    struct shared_control_block {
        
        /**
         * @internal
         * @private
         */
        tca::allocator* m_allocator;
        
        /**
         * @internal
         * @private
         */
        void* m_object;
        
        /**
         * @internal
         * @private
         */
        std::size_t m_blocksize;
        
        /**
         * @internal
         * @private
         */
        std::size_t m_strong_refs;
        
        /**
         * @internal
         * @private
         */
        std::size_t m_weak_refs;

        /**
         * @internal
         * @private
         */
        shared_control_block();
        
        /**
         * @internal
         * @private
         */
        shared_control_block(tca::allocator* allocator, void* object, std::size_t blocksize);
        
        /**
         * @internal
         * @private
         */
        ~shared_control_block();

        /**
         * @internal
         * @private
         */
        void inc_strong_ref();
        
        /**
         * @internal
         * @private
         */
        void dec_strong_ref();

        /**
         * @internal
         * @private
         */
        void inc_weak_ref();
        
        /**
         * @internal
         * @private
         */
        void dec_weak_ref();

        /**
         * @internal
         * @private
         */
        std::size_t strong_count() const;
        
        /**
         * @internal
         * @private
         */
        std::size_t weak_count() const;
    };

    /**
     * @internal
     * @private
     */
    std::size_t calc_ctr_block_total_size(std::size_t data_type_sizeof, std::size_t data_type_alignof, std::size_t* offset_to_object = nullptr);

    /**
     * @internal
     * @private
     *
     * 
     * Выделяет память под контролирующий блок и объект в одной области памяти.
     * 
     * @note
     *      Данная функция не вызывает конструкторы объектов!
     * 
     * @param allocator
     *      Распределитель памяти под котролирующий блок и объект.
     * 
     * @param object_size
     *      sizeof объекта.
     * 
     * @param object_align
     *      alignof объект.
     * 
     * @param n_objects
     *      Количество объектов
     * 
     * @return
     *      Указатель на контролирующий блок или nullptr, если выделение не удалось.
     */
    shared_control_block* alloc_memory_to_control_block(tca::allocator* allocator, std::size_t object_size, std::size_t object_align, std::size_t n_objects = 1);

    /**
     * @internal
     * @private
     */
    template<typename T, typename _T>
    shared_control_block* make_control_block(tca::allocator* allocator, _T&& obj) {
        shared_control_block* ctrl_block = alloc_memory_to_control_block(allocator, sizeof(T), alignof(T));
        if (!ctrl_block)
            return nullptr;
        try {
            assert((std::uintptr_t) ctrl_block->m_object % alignof(T) == 0);
            new (ctrl_block->m_object) T(std::forward<_T>(obj));
        } catch (...) {
            allocator->deallocate(ctrl_block);
            throw;
        }
        return ctrl_block;
    }

    /**
     * @internal
     * @private
     */
    template<typename T>
    shared_control_block* make_control_block_array(tca::allocator* allocator, std::size_t length) {
        shared_control_block* ctrl_block = alloc_memory_to_control_block(allocator, sizeof(T), alignof(T), length);
        if (!ctrl_block)
            return nullptr;
        try {
            assert((std::intptr_t) ctrl_block->m_object % alignof(T) == 0);
            using non_const_T = typename remove_cv<T>::type;
            placement_new<non_const_T>(reinterpret_cast<non_const_T*>(ctrl_block->m_object), length);
        } catch (...) {
            allocator->deallocate(ctrl_block);
            throw;
        }
        return ctrl_block;
    }

} //namespace sptr
} //namespace internal


template<typename T>
class shared_ptr;

template<typename T>
class weak_ptr {
    friend class shared_ptr<T>;
    
    /**
     * Указатель на управляющий блок.
     */
    internal::sptr::shared_control_block* m_block;

    /**
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор должен увеличивать счётчик weak-reference.
     */
    weak_ptr(internal::sptr::shared_control_block* ctrl_block);
    
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

template<typename T>
class shared_ptr {
    
    /**
     * 
     */
    template<typename A>
    friend class shared_ptr;
    
    /**
     * 
     */
    friend class weak_ptr<T>;
    
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
    internal::sptr::shared_control_block* m_block;
    
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
    explicit shared_ptr(internal::sptr::shared_control_block* ctrl_block);

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
    weak_ptr<T> get_weak() const;

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
    shared_ptr<T>::shared_ptr() : m_block(nullptr) {

    }
    
    template<typename T>
    shared_ptr<T>::shared_ptr(internal::sptr::shared_control_block* ctrl_block) : m_block(ctrl_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }

    template<typename T>
    template<typename T_, typename>
    shared_ptr<T>::shared_ptr(T_&& obj, tca::allocator* allocator) {
        m_block = internal::sptr::make_control_block<T>(allocator, std::forward<T_>(obj));
        if (m_block == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
        m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T>::shared_ptr(const shared_ptr<T>& ptr) : m_block(ptr.m_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(const shared_ptr<E>& other) : m_block(other.m_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(shared_ptr<E>&& ptr) : m_block(ptr.m_block) {
        ptr.m_block = nullptr;
    }
    
    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (const shared_ptr<E>& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block = ptr.m_block;
            if (m_block != nullptr)
                m_block->inc_strong_ref();
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& other) {
        if (&other != this) {
            cleanup();
            m_block = other.m_block;
            if (m_block != nullptr)
                m_block->inc_strong_ref();
        }
        return *this;
    }
    
    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>&& other) {
        if (&other != this) {
            cleanup();
            m_block = other.m_block;
            other.m_block = nullptr;
        }
        return *this;
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (shared_ptr<E>&& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block     = ptr.m_block;
            ptr.m_block = nullptr;
        }
        return *this;
    }

    template<typename T>
    void shared_ptr<T>::cleanup() {
        if (m_block != nullptr) {
            m_block->dec_strong_ref();
            if (m_block->strong_count() == 0) {
                T* obj = reinterpret_cast<T*>(m_block->m_object);
                assert(obj != nullptr);
                obj->~T();
                if (m_block->weak_count() == 0) {
                    tca::allocator* allocator = m_block->m_allocator;
                    if (allocator != nullptr) //если распределитель равен null, значит за выделение контрол блока отвечает shared_ptr
                        allocator->deallocate(m_block, m_block->m_blocksize);
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
            if (m_block == nullptr)
                throw_except<null_pointer_exception>("pointer must be != null");
        );
    }

    template<typename T>
    shared_ptr<T>::operator T*() const {
        check_access();
        return (T*) (m_block->m_object);
    }

    template<typename T>
    T& shared_ptr<T>::operator*() const {
        check_access();
        return *((T*) m_block->m_object);
    }
    
    template<typename T>
    T* shared_ptr<T>::operator->() const {
        check_access();
        return (T*) m_block->m_object;
    }

    template<typename T>
    std::size_t shared_ptr<T>::use_count() const {
        if (m_block != nullptr)
            return m_block->strong_count();
        return 0;
    }

    template<typename T>
    shared_ptr<T>::operator bool() const {
        return get() != nullptr;
    }

    template<typename T>
    shared_ptr<T>::operator weak_ptr<T>() const {
        return get_weak();
    }

    template<typename T>
    T* shared_ptr<T>::get() const {
        if (m_block == nullptr || m_block->strong_count() == 0)
            return nullptr;
        return (T*) m_block->m_object;
    }

    template<typename T>
    weak_ptr<T> shared_ptr<T>::get_weak() const {
        return weak_ptr<T>(m_block);
    }

    template<typename T, typename T_ = T, typename = typename enable_if<
                                                                is_same<
                                                                        typename remove_cv<T>::type,
                                                                        typename remove_cv<T_>::type
                                                                >::value
                                                            >::type>
    shared_ptr<T> make_shared(T_&& obj = T(), tca::allocator* allocator = tca::get_default_allocator()) {
        return shared_ptr<T>(std::forward<T_>(obj), allocator);
    }

    template<typename A, typename B, typename = typename enable_if<is_related<B, A>::value && is_cv_castable<B, A>::value>::type>
    shared_ptr<A> static_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(p.m_block);
    }

    template<typename A, typename B, typename = typename enable_if<
                                                            is_same<
                                                                    typename remove_cv<A>::type, 
                                                                    typename remove_cv<B>::type
                                                            >::value
                                                        >::type>
    shared_ptr<A> const_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(p.m_block);
    }
    
    template<typename A, typename B, typename = typename enable_if<is_related<A, B>::value && is_cv_castable<B, A>::value>::type>
    shared_ptr<A> dynamic_pointer_cast(const shared_ptr<B>& p) {
        B* const object = p.get();
        JSTD_DEBUG_CODE(check_non_null(object));
        if (!dynamic_cast<A*>(object))
            throw_except<class_cast_exception>("Where [To = %s, From = %s]", typeid(A).name(), typeid(*object).name());
        return shared_ptr<A>(p.m_block);
    }

    template<typename A, typename B, typename = typename enable_if<is_cv_castable<B, A>::value>::type>
    shared_ptr<A> reinterpret_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(p.m_block);
    }

    /**
     * #####################################################################################
     * 
     *                                  W E A K _ P T R
     * 
     * #####################################################################################
     */

    template<typename T>
    weak_ptr<T>::weak_ptr(internal::sptr::shared_control_block* ctrl_block) : m_block(ctrl_block) {
        if (ctrl_block != nullptr)
            ctrl_block->inc_weak_ref();
    }
    
    template<typename T>
    void weak_ptr<T>::cleanup() {
        if (m_block != nullptr) {
            m_block->dec_weak_ref();
            if (m_block->strong_count() == 0 && m_block->weak_count() == 0) {
                tca::allocator* allocator = m_block->m_allocator;
                m_block->~shared_control_block();
                if (allocator != nullptr)
                    allocator->deallocate(m_block, m_block->m_blocksize);
            }
        }
    }

    template<typename T>
    weak_ptr<T>::weak_ptr() : m_block(nullptr) {

    }
    
    template<typename T>
    weak_ptr<T>::~weak_ptr() {
        cleanup();
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) : m_block(ptr.m_block) {
        if (m_block != nullptr)
            m_block->inc_weak_ref();
    }
    
    template<typename T>
    weak_ptr<T>::weak_ptr(weak_ptr<T>&& ptr) : m_block(ptr.m_block) {
        ptr.m_block = nullptr;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<T>& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block = ptr.m_block;
            if (m_block != nullptr)
                m_block->inc_weak_ref();
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<T>&& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block     = ptr.m_block;
            ptr.m_block = nullptr;
        }
        return *this;
    }

    template<typename T>
    std::size_t weak_ptr<T>::use_count() const {
        if (m_block == nullptr)
            return 0;
        return m_block->strong_count();
    }
    
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock() const {
        if (!m_block || m_block->strong_count() == 0)
            return shared_ptr<T>();
        return shared_ptr<T>(m_block);
    }

    template<typename T>
    weak_ptr<T> make_weak() {
        return weak_ptr<T>();
    }

} //namespace jstd

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H