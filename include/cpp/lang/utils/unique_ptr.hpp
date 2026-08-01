#ifndef JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
#define JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/traits/SFINAE.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <typeinfo>

namespace tc
{

template<typename T>
class unique_ptr {

    /**
     * 
     */
    template<typename U>
    friend class unique_ptr;
   
     /**
     * 
     */
    template<typename A, typename B, typename>
    friend unique_ptr<A> static_pointer_cast(unique_ptr<B>&&);
    
    /**
     * 
     */
    template<typename A, typename B, typename>
    friend unique_ptr<A> const_pointer_cast(unique_ptr<B>&&);
    
    /**
     * 
     */
    template<typename A, typename B, typename>
    friend unique_ptr<A> dynamic_pointer_cast(unique_ptr<B>&&);

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend unique_ptr<A> reinterpret_pointer_cast(unique_ptr<B>&&);

    /**
     * 
     */
    tca::allocator* m_allocator;
    
    /**
     * 
     */
    T* m_object;

    /**
     * 
     */
    unique_ptr(T* p, tca::allocator* allocator);

    /**
     * deleted
     */
    unique_ptr(const unique_ptr<T>&) = delete;
    
    /**
     * deleted
     */
    unique_ptr<T>& operator= (const unique_ptr<T>&) = delete;

    /**
     * 
     */
    void cleanup();

    /**
     * 
     */
    void non_null_or_except() const;

public:
    /**
     * 
     */
    unique_ptr();

    /**
     * @deprecated
     */
    template<typename T_, typename = typename enable_if<
                                                is_same<
                                                        typename remove_cv<T>::type, 
                                                        typename remove_cv<T_>::type
                                                >::value
                                            >::type>
    unique_ptr(T_&& obj, tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * 
     */
    template<typename E, typename = typename enable_if<is_base_of<E, T>::value && is_cv_castable<E, T>::value>::type>
    unique_ptr(unique_ptr<E>&&);
    
    /**
     * 
     */
    template<typename E, typename = typename enable_if<is_base_of<E, T>::value>::type>
    unique_ptr<T>& operator= (unique_ptr<E>&&);

    /**
     * 
     */
    ~unique_ptr();
    
    /**
     * 
     */
    T* get() const;

    /**
     * 
     */
    operator bool() const;

    /**
     * 
     */
    T* operator->() const;

    /**
     * 
     */
    T& operator*() const;

    /**
     * 
     */
    tca::allocator* get_allocator() const;

    /**
     * 
     */
    operator T*() const;

    /**
     * 
     */
    bool operator== (const unique_ptr<T>&) const;

    /**
     * 
     */
    bool operator!= (const unique_ptr<T>&) const;
};

    template<typename T>
    unique_ptr<T>::unique_ptr() : m_allocator(nullptr), m_object(nullptr) {

    }

    template<typename T>
    unique_ptr<T>::unique_ptr(T* p, tca::allocator* allocator) :
    m_allocator(allocator),
    m_object(p) {

    }

    template<typename T>
    template<typename T_, typename>
    unique_ptr<T>::unique_ptr(T_&& obj, tca::allocator* allocator) : m_allocator(allocator) {
        void* mem = m_allocator->allocate_align(sizeof(T), alignof(T));
        if (!mem)
            throw_except<out_of_memory_error>("Out of memory!");
        try {
            m_object = new(mem) T(std::forward<T_>(obj));
        } catch (...) {
            m_allocator->deallocate(mem);
            throw;
        }
    }

    template<typename T>
    void unique_ptr<T>::cleanup() {
        if (m_allocator != nullptr && m_object != nullptr) {
            m_object->~T();
            m_allocator->deallocate((void*) m_object);
        }
    }

    template<typename T>
    unique_ptr<T>::~unique_ptr() {
        cleanup();
    }

    template<typename T>
    T* unique_ptr<T>::get() const {
        return m_object;
    }

    template<typename T>
    unique_ptr<T>::operator bool() const {
        return m_object != nullptr;
    }
    
    template<typename T>
    unique_ptr<T>::operator T*() const {
        return m_object;
    }

    template<typename T>
    template<typename E, typename>
    unique_ptr<T>::unique_ptr(unique_ptr<E>&& p) : 
    m_allocator(p.m_allocator), 
    m_object(static_cast<T*>(p.m_object)) {
        p.m_allocator   = nullptr;
        p.m_object      = nullptr;
    }
    
    template<typename T>
    template<typename E, typename>
    unique_ptr<T>& unique_ptr<T>::operator= (unique_ptr<E>&& p) {
        if (&p != this) {
            cleanup();
            m_allocator = p.m_allocator;
            m_object    = static_cast<T*>(p.m_object);
            p.m_allocator   = nullptr;
            p.m_object      = nullptr;
        }
        return *this;
    }

    template<typename T>
    void unique_ptr<T>::non_null_or_except() const {
        JSTD_DEBUG_CODE(
            if (!m_object)
                throw_except<null_pointer_exception>("m_object == null");
        );
    }

    template<typename T>
    T* unique_ptr<T>::operator->() const {
        JSTD_DEBUG_CODE(non_null_or_except());
        return m_object;
    }

    template<typename T>
    T& unique_ptr<T>::operator*() const {
        JSTD_DEBUG_CODE(non_null_or_except());
        return *m_object;
    }

    template<typename T>
    tca::allocator* unique_ptr<T>::get_allocator() const {
        return m_allocator;
    }

    template<typename T, typename T_ = T, typename = typename enable_if<
                                                                is_same<
                                                                        typename remove_cv<T>::type,
                                                                        typename remove_cv<T_>::type
                                                                >::value
                                                            >::type>
    unique_ptr<T> make_unique(T_&& obj = T(), tca::allocator* allocator = tca::get_scoped_or_default()) {
        return unique_ptr<T>(std::forward<T_>(obj), allocator);
    }

    template<typename T>
    bool unique_ptr<T>::operator== (const unique_ptr<T>& p) const {
        return m_object == p.m_object;
    }

    template<typename T>
    bool unique_ptr<T>::operator!= (const unique_ptr<T>& p) const {
        return m_object != p.m_object;
    }

    template<typename A, typename B, typename = typename enable_if<is_related<B, A>::value && is_cv_castable<A, B>::value>::type>
    unique_ptr<A> static_pointer_cast(unique_ptr<B>&& a) {
        unique_ptr<A> result(static_cast<A*>(a.m_object), a.m_allocator);
        a.m_allocator   = nullptr;
        a.m_object      = nullptr;
        return result;
    }

    template<typename A, typename B, typename = typename enable_if<
                                                                    is_same<
                                                                            typename remove_cv<A>::type, 
                                                                            typename remove_cv<B>::type
                                                                        >::value
                                                                    >::type>
    unique_ptr<A> const_pointer_cast(unique_ptr<B>&& a) {
        unique_ptr<A> result(const_cast<A*>(a.m_object), a.m_allocator);
        a.m_allocator   = nullptr;
        a.m_object      = nullptr;
        return result;
    }

    template<typename A, typename B, typename = typename enable_if<is_cv_castable<B, A>::value>::type>
    unique_ptr<A> reinterpret_pointer_cast(unique_ptr<B>&& a) {
        unique_ptr<A> result(reinterpret_cast<A*>(a.m_object), a.m_allocator);
        a.m_allocator   = nullptr;
        a.m_object      = nullptr;
        return result;
    }

    /**
     * @throws null_pointer_exception
     *      Если передаваемый указатель является нулевым.
     * 
     * @throws class_cast_exception
     *     Если код попытался привести объект к подклассу, экземпляром которого он не является.
     */
    template<typename A, typename B, typename = typename enable_if<is_related<B, A>::value && is_cv_castable<B, A>::value>::type>
    unique_ptr<A> dynamic_pointer_cast(unique_ptr<B>&& a) {
        JSTD_DEBUG_CODE(check_non_null(a.m_object));
        A* instance = dynamic_cast<A*>(a.m_object);
        if (!instance)
            throw_except<class_cast_exception>("Where [From = %s, To = %s]", typeid(A).name(), typeid(*a.m_object).name());
        unique_ptr<A> result(instance, a.m_allocator);
        a.m_allocator   = nullptr;
        a.m_object      = nullptr;
        return result;
    }
}

namespace tc
{

template<typename T>
class unique_ptr<T[]> {
    /**
     * 
     */
    tca::allocator* m_allocator;
    
    /**
     * 
     */
    T* m_array;

    /**
     * 
     */
    std::size_t m_length;

    /**
     * 
     */
    unique_ptr(const unique_ptr<T[]>&) = delete;
    
    /**
     * 
     */
    unique_ptr<T[]>& operator= (const unique_ptr<T[]>&) = delete;

    /**
     * 
     */
    void cleanup() const;

    /**
     * 
     */
    void non_null_or_except() const;

public:
    /**
     * 
     */
    unique_ptr();

    /**
     * 
     */
    unique_ptr(std::size_t len, tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * 
     */
    unique_ptr(unique_ptr<T[]>&&);
    
    /**
     * 
     */
    unique_ptr<T[]>& operator= (unique_ptr<T[]>&&);

    /**
     * 
     */
    ~unique_ptr();

    /**
     * 
     */
    T* get() const;

    /**
     * 
     */
    operator bool() const;

    /**
     * 
     */
    T& operator[] (std::size_t idx) const;

    /**
     * 
     */
    tca::allocator* get_allocator() const;

    /**
     * 
     */
    operator T*() const;
};

    template<typename T>
    unique_ptr<T[]>::unique_ptr() : m_allocator(nullptr), m_array(nullptr), m_length(0) {

    }

    template<typename T>
    unique_ptr<T[]>::unique_ptr(std::size_t len, tca::allocator* allocator) :
    m_allocator(allocator), 
    m_array(nullptr), 
    m_length(len) {
        void* mem = allocator->allocate_align(sizeof(T) * len, alignof(T));
        if (!mem)
            throw_except<out_of_memory_error>("Out of memory!");
        try {
            using NON_CONST_T = typename remove_cv<T>::type;
            placement_new<NON_CONST_T>(const_cast<NON_CONST_T*>(static_cast<T*>(mem)), len);
            m_array = static_cast<T*>(mem);
        } catch (...) {
            allocator->deallocate(mem, sizeof(T) * len);
            throw;
        }
    }

    template<typename T>
    unique_ptr<T[]>::unique_ptr(unique_ptr<T[]>&& p) :
    m_allocator(p.m_allocator),
    m_array(p.m_array),
    m_length(p.m_length) {
        p.m_allocator   = nullptr;
        p.m_array       = nullptr;
        p.m_length      = 0;
    }
    
    template<typename T>
    unique_ptr<T[]>& unique_ptr<T[]>::operator= (unique_ptr<T[]>&& p) {
        if (&p != this) {
            cleanup();
            m_allocator = p.m_allocator;
            m_array     = p.m_array;
            m_length    = p.m_length;            
            p.m_allocator   = nullptr;
            p.m_array       = nullptr;
            p.m_length      = 0;
        }
        return *this;
    }

    template<typename T>
    void unique_ptr<T[]>::cleanup() const {
        if (m_allocator != nullptr && m_array != nullptr) {
            placement_destroy(m_array, m_length);
            m_allocator->deallocate(m_array);
        }
    }

    template<typename T>
    unique_ptr<T[]>::~unique_ptr() {
        cleanup();
    }

    template<typename T>
    void unique_ptr<T[]>::non_null_or_except() const {
        if (!m_array)
            throw_except<null_pointer_exception>("m_array == null!");
    }

    template<typename T>
    T* unique_ptr<T[]>::get() const {
        return m_array;
    }

    template<typename T>
    unique_ptr<T[]>::operator bool() const {
        return m_array != nullptr;
    }

    template<typename T>
    tca::allocator* unique_ptr<T[]>::get_allocator() const {
        return m_allocator;
    }

    template<typename T>
    T& unique_ptr<T[]>::operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE(
            non_null_or_except(); 
            check_index(idx, m_length);
        );
        return m_array[idx];
    }

    template<typename T>
    unique_ptr<T[]>::operator T*() const {
        return m_array;
    }

    template<typename T>
    unique_ptr<T[]> make_unique_array(std::size_t len, tca::allocator* allocator = tca::get_scoped_or_default()) {
        return unique_ptr<T[]>(len, allocator);
    }
}

#endif//JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
