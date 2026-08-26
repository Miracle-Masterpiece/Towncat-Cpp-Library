#ifndef JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
#define JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/traits/sfinae.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/traits/type_properties.hpp>
#include <cpp/lang/traits/pure_traits.hpp>
#include <cpp/lang/utils/pair.hpp>
#include <typeinfo>

namespace tc
{

template<typename T>
struct default_deleter {
    
    default_deleter() {}

    template<typename E, typename = typename enable_if<is_cv_castable<E, T>::value && is_base_of<E, T>::value>::type>
    default_deleter(const default_deleter<E>& a) {}
    
    void operator()(T* p) const {
        p->~T();
        tca::get_default_allocator()->deallocate((void*) p);
    }
};

template<typename T>
struct alloc_deleter {
    tca::allocator* m_alloc;

    alloc_deleter(tca::allocator* alloc) : m_alloc(alloc) {}
    
    alloc_deleter() : m_alloc(nullptr) {}

    template<typename E, typename = typename enable_if<is_cv_castable<E, T>::value && is_base_of<E, T>::value>::type>
    alloc_deleter(const alloc_deleter<E>& a) : m_alloc(a.m_alloc) {}

    void operator()(T* p) const {
        p->~T();
        m_alloc->deallocate(static_cast<void*>(p));
    }
};


template<typename, typename>
class unique_ptr;

namespace polymorph
{
    template<typename T, typename... Args>
    tc::unique_ptr<T, alloc_deleter<T>> allocate_unique(tca::allocator*, Args&&...);
}

template<typename T, typename DELETER = default_deleter<T>>
class unique_ptr {

    /**
     * 
     */
    template<typename U, typename D>
    friend class unique_ptr;
   
    /**
     * 
     */
    template<typename A, typename D>
    friend tc::unique_ptr<A, D> wrap_unique(T*, D);
    
    /**
     * 
     */
    template<typename A, typename... Args>
    friend tc::unique_ptr<A> make_unique(Args&&...);

    /**
     * 
     */
    template<typename A, typename... Args>
    friend tc::unique_ptr<A, alloc_deleter<A>> polymorph::allocate_unique(tca::allocator*, Args&&...);

    /**
     * first  - pointer
     * second - deleter
     * 
     */
    pair<T*, DELETER> m_pair;

    /**
     * deleted
     */
    unique_ptr(const unique_ptr<T, DELETER>&) = delete;
    
    /**
     * deleted
     */
    unique_ptr<T, DELETER>& operator= (const unique_ptr<T, DELETER>&) = delete;

    /**
     * 
     */
    void cleanup();

    /**
     * 
     */
    void non_null_or_except() const;

    /**
     * 
     */
    unique_ptr(T* obj, DELETER deleter = DELETER());

public:
    /**
     * 
     */
    unique_ptr();
    
    /**
     * 
     */
    template<typename E, typename EDELETER, typename = typename enable_if<is_base_of<E, T>::value && is_cv_castable<E, T>::value>::type>
    unique_ptr(unique_ptr<E, EDELETER>&&);

    /**
     * 
     */
    template<typename E, typename = typename enable_if<is_base_of<E, T>::value>::type>
    unique_ptr<T, DELETER>& operator= (unique_ptr<E, DELETER>&&);

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
    bool operator== (const unique_ptr<T>&) const;

    /**
     * 
     */
    bool operator!= (const unique_ptr<T>&) const;

    /**
     * 
     */
    T* release();
};

    template<typename T, typename DELETER>
    unique_ptr<T, DELETER>::unique_ptr() : m_pair() {

    }

    template<typename T, typename DELETER>
    unique_ptr<T, DELETER>::unique_ptr(T* p, DELETER deleter) : m_pair(p, std::move(deleter)) {

    }

    template<typename T, typename DELETER>
    void unique_ptr<T, DELETER>::cleanup() {
        if (get())
        {
            m_pair.second()(get());
        }
    }

    template<typename T, typename DELETER>
    unique_ptr<T, DELETER>::~unique_ptr() {
        cleanup();
    }

    template<typename T, typename DELETER>
    T* unique_ptr<T, DELETER>::release() {
        T* ptr = get();
        m_pair.first() = nullptr;
        return ptr;
    }

    template<typename T, typename DELETER>
    T* unique_ptr<T, DELETER>::get() const {
        return m_pair.first();
    }

    template<typename T, typename DELETER>
    unique_ptr<T, DELETER>::operator bool() const {
        return get() != nullptr;
    }
    
    template<typename T, typename DELETER>
    template<typename E, typename EDELETER, typename>
    unique_ptr<T, DELETER>::unique_ptr(unique_ptr<E, EDELETER>&& p) : m_pair(p.m_pair.first(), std::move(p.m_pair.second())) {
        p.m_pair.first() = nullptr;
    }
    
    template<typename T, typename DELETER>
    template<typename E, typename>
    unique_ptr<T, DELETER>& unique_ptr<T, DELETER>::operator= (unique_ptr<E, DELETER>&& p) {
        if (&p != this) {
            cleanup();
            m_pair = std::move(p.m_pair);
            p.m_pair.first() = nullptr;
        }
        return *this;
    }
    
    template<typename T, typename DELETER>
    void unique_ptr<T, DELETER>::non_null_or_except() const {
        JSTD_DEBUG_CODE(
            if (!get())
                throw_except<null_pointer_exception>("m_object == null");
        );
    }

    template<typename T, typename DELETER>
    T* unique_ptr<T, DELETER>::operator->() const {
        JSTD_DEBUG_CODE(non_null_or_except());
        return get();
    }

    template<typename T, typename DELETER>
    T& unique_ptr<T, DELETER>::operator*() const {
        JSTD_DEBUG_CODE(non_null_or_except());
        return *get();
    }

    template<typename T, typename DELETER>
    bool unique_ptr<T, DELETER>::operator== (const unique_ptr<T>& p) const {
        return get() == p.get();
    }

    template<typename T, typename DELETER>
    bool unique_ptr<T, DELETER>::operator!= (const unique_ptr<T>& p) const {
        return get() != p.get();
    }

namespace polymorph
{
    template<typename T, typename... Args>
    tc::unique_ptr<T, alloc_deleter<T>> allocate_unique(tca::allocator* alloc, Args&&... args) {
        void* p = alloc->allocate_align(sizeof(T), alignof(T));
        if (!p)
            throw_except<out_of_memory_error>("out of memory");
    
        try {
            T* obj = new(p) T(std::forward<Args>(args)...);
            return unique_ptr<T, alloc_deleter<T>>(obj, {alloc});
        } catch(...) {
            alloc->deallocate(p);
            throw;
        }
    }
}

    template<typename T, typename... Args>
    tc::unique_ptr<T> make_unique(Args&&... args) {
        
        tca::allocator* default_alloc = tca::get_default_allocator();

        void* p = default_alloc->allocate_align(sizeof(T), alignof(T));
        if (!p) throw_except<out_of_memory_error>("out of memory");
    
        try {
            T* obj = new(p) T(std::forward<Args>(args)...);
            return unique_ptr<T>(obj);
        } catch(...) {
            default_alloc->deallocate(p);
            throw;
        }

    }
    
    template<typename T, typename DELETER>
    tc::unique_ptr<T, DELETER> wrap_unique(T* p, DELETER deleter) {
        return tc::unique_ptr<T, DELETER>(p, std::move(deleter));
    }
}

#if 0
// Я хер знает, буду ли я доделывать этот класс. 
// кто-то вообще использует массив внутри unique_ptr?
namespace tc
{

template<typename T, typename DELETER>
class unique_ptr<T[]> {

    /**
     * 
     */
    pair<T*, DELETER> m_pair;

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
    unique_ptr(T* arr, DELETER deleter);

    /**
     * 
     */
    unique_ptr(std::size_t len, const T& v = T(), tca::allocator* allocator = tca::get_default_allocator());

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
};

    template<typename T>
    unique_ptr<T[]>::unique_ptr() : m_allocator(nullptr), m_array(nullptr), m_length(0) {

    }

    template<typename T>
    unique_ptr<T[]>::unique_ptr(std::size_t len, const T& v, tca::allocator* allocator) :
    m_allocator(allocator), 
    m_array(nullptr), 
    m_length(len) {
        void* mem = allocator->allocate_align(sizeof(T) * len, alignof(T));
        if (!mem)
            throw_except<out_of_memory_error>("Out of memory!");
        try {
            m_array = static_cast<Tvalue*>(mem);
            uninitialized_construct_n(m_array, len, v);
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
        if (m_allocator != nullptr && m_array != nullptr)
        {
            destroy_n(m_array, m_length);
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
}

namespace tc
{

    template<typename T, typename T_ = T, typename = typename enable_if<is_same<typename pure_type<T>::type, typename pure_type<T_>::type>::value>::type>
    typename enable_if< !is_array<T>::value, unique_ptr<T> >::type make_unique(T_&& v = T(), tca::allocator* allocator = tca::get_default_allocator()) {
        return unique_ptr<T>(std::forward<T_>(v), allocator);
    }
    
    template<typename T, typename T_ = T>
    typename enable_if< !is_array<T>::value, unique_ptr<T> >::type make_unique(tca::allocator* allocator) {
        return unique_ptr<T>(T(), allocator);
    }

    template<typename T>
    typename enable_if< is_array<T>::value, unique_ptr<T> >::type make_unique(std::size_t len, const typename pure_type<T>::type& val = typename pure_type<T>::type(), tca::allocator* allocator = tca::get_default_allocator()) {
        return unique_ptr<T>(len, val, allocator);
    }
    
    template<typename T>
    typename enable_if< is_array<T>::value, unique_ptr<T> >::type make_unique(std::size_t len, tca::allocator* allocator = tca::get_default_allocator()) {
        return unique_ptr<T>(len, typename pure_type<T>::type(), allocator);
    }
}
#endif

namespace tc
{
namespace polymorph {
    template<typename T> using unique_ptr = tc::unique_ptr<T, alloc_deleter<T>>;
}
}

#endif//JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
