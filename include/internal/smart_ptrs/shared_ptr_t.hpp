#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H

#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstddef>
#include <utility>
#include <cstdio>
#include <new>
#include <cassert>
#include <typeinfo>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/traits/sfinae.hpp>
#include <internal/smart_ptrs/control_block.hpp>

namespace tc 
{

template<typename T>
class shared_ptr;

/**
 * @brief A weak reference to an object managed by shared_ptr.
 * 
 * `weak_ptr` provides a non-owning reference to an object managed by one or more
 * `shared_ptr` instances. It does not affect the object's lifetime and is used
 * to break circular references.
 * 
 * A `weak_ptr` can be converted to a `shared_ptr` using `lock()`, which returns
 * a valid shared pointer if the object still exists, or an empty one otherwise.
 * 
 * @tparam T
 *      The type of the managed object.
 */
template<typename T>
class weak_ptr {
    
    /**
     * @brief Grants shared_ptr access to internal members.
     */
    friend class shared_ptr<T>;
    
    /**
     * @brief Pointer to the control block managing reference counts.
     */
    internal::control_block* m_control;

    /**
     * @brief Raw pointer to the managed object.
     */
    T* m_obj;

    /**
     * @internal
     * @brief Internal constructor for shared_ptr's use.
     * 
     * Creates a weak_ptr from an existing object and control block.
     * Increments the weak reference count.
     * 
     * This constructor is intended for internal use only and is called
     * by shared_ptr when creating weak_ptr instances.
     * 
     * @param obj
     *      Pointer to the managed object.
     * 
     * @param block
     *      Pointer to the control block.
     * 
     */
    weak_ptr(T* obj, internal::control_block* block);
    
    /**
     * @internal
     * @brief Releases the weak reference.
     * 
     * Decrements the weak reference count. If it reaches zero, destroys the
     * control block.
     */
    void cleanup();

public:
    /**
     * @brief Default constructor. Creates an empty weak_ptr.
     * 
     * The created weak_ptr does not point to any object.
     * 
     * @example
     *      tc::weak_ptr<int> w;  // empty
     *      assert(w.use_count() == 0);
     */
    weak_ptr();

    /**
     * @brief Copy constructor. Creates a new weak reference.
     * 
     * Increments the weak reference count of the control block.
     * 
     * @param other
     *      Another weak_ptr to copy from.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(42);
     *      tc::weak_ptr<int> w1(sp);
     *      tc::weak_ptr<int> w2(w1);  // copy
     *      assert(w1.use_count() == w2.use_count());
     */
    weak_ptr(const weak_ptr<T>& other);

    /**
     * @brief Move constructor. Transfers ownership of the weak reference.
     * 
     * The source weak_ptr becomes empty after the move.
     * 
     * @param other
     *      Another weak_ptr to move from.
     * 
     * @example
     *      tc::weak_ptr<int> w1(sp);
     *      tc::weak_ptr<int> w2(std::move(w1));
     *      assert(!w1);       // w1 is now empty
     *      assert(w2.lock()); // w2 points to the object
     */
    weak_ptr(weak_ptr<T>&& other);

    /**
     * @brief Destructor. Releases the weak reference.
     * 
     * Decrements the weak reference count. If it reaches zero and the
     * strong count is also zero, the control block is destroyed.
     */
    ~weak_ptr();

    /**
     * @brief Copy assignment operator.
     * 
     * Releases the current weak reference and acquires a new one from `other`.
     * Increments the weak reference count of the new control block.
     * 
     * @param other
     *      Another weak_ptr to copy from.
     * 
     * @return Reference to this weak_ptr.
     * 
     * @example
     *      tc::weak_ptr<int> w1(sp1);
     *      tc::weak_ptr<int> w2(sp2);
     *      w1 = w2;  // w1 now points to sp2's object
     */
    weak_ptr<T>& operator=(const weak_ptr<T>& other);

    /**
     * @brief Move assignment operator.
     * 
     * Transfers the weak reference from `other` to this. The source becomes
     * empty after the move.
     * 
     * @param other
     *      Another weak_ptr to move from.
     * 
     * @return
     *      Reference to this weak_ptr.
     * 
     * @example
     *      tc::weak_ptr<int> w1(sp1);
     *      tc::weak_ptr<int> w2(sp2);
     *      w1 = std::move(w2);  // w1 now points to sp2's object, w2 is empty
     */
    weak_ptr<T>& operator=(weak_ptr<T>&& other);

    /**
     * @brief Returns the number of shared_ptr instances managing the object.
     * 
     * @return
     *      The strong reference count, or 0 if this weak_ptr is empty.

     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(42);
     *      tc::weak_ptr<int> w(sp);
     *      assert(w.use_count() == 1);
     *      
     *      tc::shared_ptr<int> sp2 = sp;
     *      assert(w.use_count() == 2);
     */
    std::size_t use_count() const;

    /**
     * @brief Creates a shared_ptr from this weak_ptr if the object still exists.
     * 
     * If the managed object still exists (strong count > 0), increments the
     * strong reference count and returns a shared_ptr to the object.
     * Otherwise, returns an empty shared_ptr.
     * 
     * @return A shared_ptr to the managed object, or an empty one if the
     *         object has been destroyed.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(42);
     *      tc::weak_ptr<int> w(sp);
     *      
     *      auto locked = w.lock();
     *      assert(locked);           // object exists
     *      assert(*locked == 42);
     *      
     *      sp.reset();               // destroy the object
     *      locked = w.lock();
     *      assert(!locked);          // object is gone
     */
    shared_ptr<T> lock() const;

};

/**
 * @brief A shared pointer with reference-counted ownership.
 * 
 * `shared_ptr` is a smart pointer that retains shared ownership of an object
 * through a pointer. Several `shared_ptr` objects may own the same object.
 * The object is destroyed and its memory deallocated when either:
 * - The last remaining `shared_ptr` owning the object is destroyed.
 * - The last remaining `shared_ptr` owning the object is reassigned via
 *   `operator=` or `reset()`.
 * 
 * The object is destroyed using the deleter provided at construction time.
 * By default, `delete` is used.
 * 
 * @tparam T
 *      The type of the managed object.
 * 
 * @note `shared_ptr` supports custom deleters and custom allocators.
 * @note `shared_ptr` provides strong exception guarantee for most operations.
 * @note `shared_ptr` can be converted to `weak_ptr` to break circular references.
 * 
 * @see weak_ptr
 * @see allocate_shared
 * @see make_shared
 */
template<typename T>
class shared_ptr {
    
    /**
     * @brief Allows converting between different shared_ptr types.
     */
    template<typename A>
    friend class shared_ptr;
    
    /**
     * @brief Grants weak_ptr access to internal members.
     */
    friend class weak_ptr<T>;

    /**
     * @brief Grants allocate_shared access to internal constructor.
     */
    template<typename A, typename... Args>
    friend typename enable_if< !is_array<A>::value, shared_ptr<A> >::type allocate_shared(tca::allocator*, Args&&...);

    /**
     * 
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> static_pointer_cast(const shared_ptr<B>&);

    /**
     * @brief Grants static_pointer_cast access to internal members.
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> const_pointer_cast(const shared_ptr<B>&);

    /**
     * @brief Grants const_pointer_cast access to internal members.
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> dynamic_pointer_cast(const shared_ptr<B>&);

    /**
     * @brief Grants dynamic_pointer_cast access to internal members.
     */
    template<typename A, typename B, typename>
    friend shared_ptr<A> reinterpret_pointer_cast(const shared_ptr<B>&);

    /**
     * @brief Pointer to the control block managing reference counts and deleter.
     */
    internal::control_block* m_control;

    /**
     * @brief Raw pointer to the managed object.
     */
    T* m_obj;
    
    /**
     * @internal
     * @brief Releases ownership of the managed object.
     * 
     * Decrements the strong reference count. If it reaches zero, the object
     * is destroyed via the deleter and the weak count is checked.
     * If both counts are zero, the control block is destroyed.
     */
    void cleanup();
    
    /**
     * @internal
     * @brief Checks if the object is accessible.
     * 
     * @throws (in DEBUG build)
     *      null_pointer_exception If the object pointer is null.
     * 
     * @throws (in DEBUG build)
     *      illegal_state_exception If the strong reference count is 0.
     */
    void check_access() const;

    /**
     * @internal
     * @brief Internal constructor for use by factory functions and casts.
     * 
     * @param obj
     *      Pointer to the managed object.
     * 
     * @param block
     *      Pointer to the control block.
     * 
     * @note This constructor does NOT increment the strong reference count.
     *       The caller is responsible for proper reference counting.
     */
    explicit shared_ptr(T* obj, internal::control_block* block);

public:
    /**
     * @brief Default constructor. Creates an empty shared_ptr.
     * 
     * The created shared_ptr does not own any object.
     * 
     * @example
     *      tc::shared_ptr<int> ptr;  // empty
     *      assert(!ptr);
     *      assert(ptr.get() == nullptr);
     */
    shared_ptr();

    /**
     * @brief Copy constructor. Shares ownership with another shared_ptr.
     * 
     * Increments the strong reference count of the control block.
     * Both shared_ptrs now share ownership of the same object.
     * 
     * @param other
     *      Another shared_ptr to copy from.
     * 
     * @example
     *      tc::shared_ptr<int> sp1 = tc::make_shared<int>(42);
     *      tc::shared_ptr<int> sp2(sp1);  // both point to the same object
     *      assert(sp1.use_count() == 2);
     *      assert(sp2.use_count() == 2);
     *      assert(*sp1 == *sp2);
     */
    shared_ptr(const shared_ptr<T>& other);

    /**
     * @brief Converting copy constructor.
     * 
     * Allows constructing a shared_ptr from another shared_ptr of a related type.
     * The source type must be a base class of T or convertible with cv-qualifiers.
     * 
     * @param other
     *      Another shared_ptr to copy from.
     * 
     * @example
     *      struct Base {};
     *      struct Derived : Base {};
     *      
     *      tc::shared_ptr<Derived> derived = tc::make_shared<Derived>();
     *      tc::shared_ptr<Base> base(derived);  // upcast is allowed
     *      assert(base.use_count() == 2);
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr(const shared_ptr<E>& other);

    /**
     * @brief Converting move constructor.
     * 
     * Transfers ownership from another shared_ptr of a related type.
     * The source shared_ptr becomes empty after the move.
     * 
     * @param other
     *      Another shared_ptr to move from.
     * 
     * @example
     *      tc::shared_ptr<Derived> derived = tc::make_shared<Derived>();
     *      tc::shared_ptr<Base> base(std::move(derived));  // move upcast
     *      assert(!derived);  // derived is now empty
     *      assert(base);      // base owns the object
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr(shared_ptr<E>&& other);

    /**
     * @brief Copy assignment operator.
     * 
     * Releases the current object (if any) and shares ownership with `other`.
     * Increments the strong reference count of `other`'s control block.
     * 
     * @param other
     *      Another shared_ptr to copy from.
     * 
     * @return
     *      Reference to this shared_ptr.
     * 
     * @example
     *      tc::shared_ptr<int> sp1 = tc::make_shared<int>(42);
     *      tc::shared_ptr<int> sp2 = tc::make_shared<int>(64);
     *      sp1 = sp2;  // sp1 now points to sp2's object
     *      assert(*sp1 == 64);
     *      assert(sp1.use_count() == 2);
     */
    shared_ptr<T>& operator=(const shared_ptr<T>& other);

    /**
     * @brief Move assignment operator.
     * 
     * Transfers ownership from `other` to this. The source becomes empty.
     * 
     * @param other
     *      Another shared_ptr to move from.
     * 
     * @return
     *      Reference to this shared_ptr.
     * 
     * @example
     *      tc::shared_ptr<int> sp1 = tc::make_shared<int>(42);
     *      tc::shared_ptr<int> sp2 = tc::make_shared<int>(64);
     *      sp1 = std::move(sp2);  // sp1 now owns sp2's object
     *      assert(*sp1 == 64);
     *      assert(!sp2);  // sp2 is empty
     */
    shared_ptr<T>& operator=(shared_ptr<T>&& other);

    /**
     * @brief Copy assignment from a related shared_ptr type.
     * 
     * @param other
     *      Another shared_ptr to copy from.
     * 
     * @return
     *      Reference to this shared_ptr.
     * 
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr<T>& operator=(const shared_ptr<E>& other);

    /**
     * @brief Move assignment from a related shared_ptr type.
     * 
     * @param other
     *      Another shared_ptr to move from.
     * 
     * @return
     *      Reference to this shared_ptr.
     */
    template<typename E, typename = typename enable_if<
                                                is_base_of<E, T>::value && is_cv_castable<E, T>::value
                                            >::type>
    shared_ptr<T>& operator=(shared_ptr<E>&& other);

    /**
     * @brief Destructor. Releases ownership of the managed object.
     * 
     * Decrements the strong reference count. If it reaches zero, the object
     * is destroyed and the memory is deallocated using the stored deleter.
     */
    ~shared_ptr();

    /**
     * @brief Dereferences the managed object.
     * 
     * @return
     *      A reference to the managed object.
     * 
     * @throws null_pointer_exception (in DEBUG build)
     *      If this shared_ptr is empty.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(7);
     *      int& ref = *sp;
     *      ref = 100;
     *      assert(*sp == 100);
     */
    T& operator*() const;

    /**
     * @brief Provides access to the members of the managed object.
     * 
     * @return
     *      A pointer to the managed object.
     * 
     * @throws null_pointer_exception (in DEBUG build)
     *      If this shared_ptr is empty.
     * 
     * @example
     *      struct Point { int x, y; };
     *      tc::shared_ptr<Point> sp = tc::make_shared<Point>(Point{10, 20});
     *      sp->x = 30;
     *      assert(sp->x == 30);
     *      assert((*sp).y == 20);
     */
    T* operator->() const;

    /**
     * @brief Returns the raw pointer to the managed object.
     * 
     * @return
     *      The raw pointer, or nullptr if this shared_ptr is empty.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(23);
     *      int* raw = sp.get();
     *      assert(*raw == 23);
     *      
     *      tc::shared_ptr<int> empty;
     *      assert(empty.get() == nullptr);
     */
    T* get() const;

    /**
     * @brief Checks if this shared_ptr owns an object.
     * 
     * @return
     *      true if the shared_ptr owns an object, false otherwise.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(34);
     *      if (sp) {
     *          std::cout << "sp owns an object\n";
     *      }
     */
    operator bool() const;

    /**
     * @brief Implicit conversion to raw pointer.
     * 
     * Allows using shared_ptr where a raw pointer is expected.
     * 
     * @return
     *      The raw pointer.
     * 
     * @throws null_pointer_exception (in DEBUG mode)
     *      If this shared_ptr is empty.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(34);
     *      int* raw = sp;  // implicit conversion
     *      assert(*raw == 34);
     */
    operator T*() const;

    /**
     * @brief Creates a weak_ptr observing the same object.
     * 
     * @return
     *      A weak_ptr that points to the same object.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(34);
     *      tc::weak_ptr<int> w = sp;  // implicit conversion
     *      assert(w.lock() == sp);
     */
    operator weak_ptr<T>() const;

    /**
     * @brief Creates a weak_ptr observing the same object.
     * 
     * @return
     *      A weak_ptr that points to the same object.
     * 
     * @example
     *      tc::shared_ptr<int> sp = tc::make_shared<int>(34);
     *      auto w = sp.get_weak();
     *      assert(w.lock() == sp);
     */
    weak_ptr<T> get_weak() const;

    /**
     * @brief Returns the number of shared_ptr instances owning the same object.
     * 
     * @return
     *      The strong reference count, or 0 if this shared_ptr is empty.
     */
    std::size_t use_count() const;

    /**
     * @brief Compares two shared_ptrs for equality.
     * 
     * Returns true if both shared_ptrs point to the same object.
     * 
     * @param ptr
     *      The other shared_ptr to compare with.
     * 
     * @return
     *      true if both point to the same object, false otherwise.
     * 
     * @example
     *      tc::shared_ptr<int> sp1 = tc::make_shared<int>(34);
     *      tc::shared_ptr<int> sp2 = sp1;
     *      tc::shared_ptr<int> sp3 = tc::make_shared<int>(100);
     *      
     *      assert(sp1 == sp2);
     *      assert(sp1 != sp3);
     */
    template<typename E>
    bool operator== (const shared_ptr<E>& ptr) const {
        return ptr.get() == get();
    }
};


    template<typename T>
    shared_ptr<T>::shared_ptr() : m_control(nullptr), m_obj(nullptr) {

    }
    
    #if 0
    template<typename T>
    template<typename DELETER>
    shared_ptr<T>::shared_ptr(T* obj, DELETER deleter, tca::allocator* alloc) : m_control(nullptr), m_obj(nullptr) {
        typedef internal::ptr_control_block<T, DELETER> control_block_type;
        void* mem = alloc->allocate_align(sizeof(control_block_type), alignof(control_block_type));
        if (!mem)
            throw_except<out_of_memory_error>("out of memory");
        try {
            m_control = new (mem) control_block_type(obj, std::move(deleter), alloc);
        } catch (...) {
            alloc->deallocate(mem);
            throw;
        }
    }
    #endif
    
    template<typename T>
    shared_ptr<T>::shared_ptr(T* obj, internal::control_block* block) : m_control(block), m_obj(obj) {

    }
    
    template<typename T>
    shared_ptr<T>::shared_ptr(const shared_ptr<T>& ptr) : m_control(ptr.m_control), m_obj(ptr.m_obj) {
        if (m_control)
        {
            m_control->inc_strong();
        }
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(const shared_ptr<E>& ptr) : m_control(ptr.m_control), m_obj(ptr.m_obj) {
        if (m_control)
        {
            m_control->inc_strong();
        }
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>::shared_ptr(shared_ptr<E>&& ptr) : m_control(ptr.m_control), m_obj(ptr.m_obj) {
        ptr.m_obj       = nullptr;
        ptr.m_control   = nullptr;
    }
    
    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (const shared_ptr<E>& ptr) {
        if (&ptr != this)
        {
            cleanup();
            m_control   = ptr.m_control;
            m_obj       = ptr.m_obj;
            if (m_control)
            {
                m_control->inc_strong();
            }
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& ptr) {
        if (&ptr != this)
        {
            cleanup();
            m_control   = ptr.m_control;
            m_obj       = ptr.m_obj;
            if (m_control)
            {
                m_control->inc_strong();
            }
        }
        return *this;
    }
    
    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>&& ptr) {
        if (&ptr != this)
        {
            std::swap(m_control, ptr.m_control);
            std::swap(m_obj,     ptr.m_obj);
        }
        return *this;
    }

    template<typename T>
    template<typename E, typename>
    shared_ptr<T>& shared_ptr<T>::operator= (shared_ptr<E>&& ptr) {
        if (ptr.m_obj != m_obj)
        {
            cleanup();
            m_control     = ptr.m_control;
            m_obj         = ptr.m_obj;
            ptr.m_control = nullptr;
            ptr.m_obj     = nullptr;
        }
        return *this;
    }

    template<typename T>
    void shared_ptr<T>::cleanup() {
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
        JSTD_DEBUG_CODE
        (
            check_access();
        )
        return m_obj;
    }

    template<typename T>
    T& shared_ptr<T>::operator*() const {
        JSTD_DEBUG_CODE
        (
            check_access();
        )
        return *m_obj;
    }
    
    template<typename T>
    T* shared_ptr<T>::operator->() const {
        JSTD_DEBUG_CODE
        (
            check_access();
        )
        return m_obj;
    }

    template<typename T>
    std::size_t shared_ptr<T>::use_count() const {
        if (m_control)
        {
            return m_control->get_strong();
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
        return weak_ptr<T>(m_obj, m_control);
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
        if (p.use_count() > 0)
        {
            p.m_control->inc_strong();
            return shared_ptr<A>(const_cast<A*>(p.m_obj), p.m_control);
        }
        return shared_ptr<A>();
    }
    
    template<typename A, typename B, typename = typename enable_if<is_related<A, B>::value && is_cv_castable<B, A>::value>::type>
    shared_ptr<A> dynamic_pointer_cast(const shared_ptr<B>& p) {
        B* const object = p.get();
        JSTD_DEBUG_CODE(check_non_null(object));
        if (!dynamic_cast<A*>(object))
            throw_except<class_cast_exception>("Where [To = %s, From = %s]", typeid(A).name(), typeid(*object).name());
        return shared_ptr<A>(
                                static_cast<A*>(p.m_obj), p.m_control
                            );
    }

    template<typename A, typename B, typename = typename enable_if<is_cv_castable<B, A>::value>::type>
    shared_ptr<A> reinterpret_pointer_cast(const shared_ptr<B>& p) {
        return shared_ptr<A>(
                                static_cast<A*>(p.m_obj), p.m_control
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
    weak_ptr<T>::weak_ptr(T* obj, internal::control_block* block) : m_control(block), m_obj(obj) {
        if (m_control)
        {
            m_control->inc_weak();
        }
    }
    
    template<typename T>
    void weak_ptr<T>::cleanup() {
        if (m_control)
        {
            if (m_control->dec_weak() == 0)
            {
                m_control->destroy_control_block();
            }
        }
    }

    template<typename T>
    weak_ptr<T>::weak_ptr() : m_control(nullptr), m_obj(nullptr) {

    }
    
    template<typename T>
    weak_ptr<T>::~weak_ptr() {
        cleanup();
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) : m_control(ptr.m_control), m_obj(ptr.m_obj) {
        if (m_control)
        {
            m_control->inc_weak();
        }
    }
    
    template<typename T>
    weak_ptr<T>::weak_ptr(weak_ptr<T>&& ptr) : m_control(ptr.m_control), m_obj(ptr.m_obj) {
        ptr.m_control = nullptr;
        ptr.m_obj     = nullptr;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<T>& ptr) {
        if (&ptr != this) 
        {
            cleanup();
            m_control = ptr.m_control;
            m_obj     = ptr.m_obj;
            if (m_control)
            {
                m_control->inc_weak();
            }
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<T>&& ptr) {
        if (&ptr != this)
        {
            std::swap(m_control, ptr.m_control);
            std::swap(m_obj,     ptr.m_obj);
        }
        return *this;
    }

    template<typename T>
    std::size_t weak_ptr<T>::use_count() const {
        if (m_control)
        {
            return m_control->get_strong();
        }
        return 0;
    }
    
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock() const {
        if (m_control)
        {
            if (m_control->try_inc_strong())
            {
                return shared_ptr<T>(m_obj, m_control);
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