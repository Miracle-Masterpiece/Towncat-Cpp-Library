#ifndef AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF
#define AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF

#include <allocators/base_allocator.hpp>

namespace tca
{

/**
 * Abstract base class for memory allocators.
 * 
 * Provides the core allocation interface with support for:
 *  Custom allocation with alignment
 *  eallocation with size hints
 *  Scoped allocator overrides
 *  Move semantics
 */
class allocator : public tca::base_allocator {
    
    /**
     * 
     */
    allocator(const allocator&);
    
    /**
     * 
     */
    allocator& operator= (const allocator&);

public:
    /**
     * Default constructor.
     * 
     * Initializes the allocator with no parent.
     */
    allocator();

    /**
     * Constructs with a parent allocator.
     * 
     * @param parent
     *      The parent allocator to use for fallback allocations.
     */
    allocator(base_allocator* parent);

    /**
     * Move constructor.
     * 
     * @param other The allocator to move from.
     */
    allocator(allocator&&);
    
    /**
     * Move assignment operator.
     * 
     * @param other
     *      The allocator to move from.
     * 
     * @return Reference to this allocator.
     */
    allocator& operator= (allocator&&);
        
    /**
     * Deallocates memory.
     * 
     * Pure virtual function that must be implemented by derived classes.
     * 
     * @param p
     *      Pointer to memory previously allocated by this allocator.
     * 
     * @warning
     *      Passing an invalid pointer results in undefined behavior.
     */
    virtual void deallocate(void*) = 0;

    /**
     * Deallocates memory with a size hint.
     * 
     * Default implementation calls deallocate(p) ignoring the size hint.
     * Derived classes may override this for optimization.
     * 
     * @param p
     *      Pointer to memory previously allocated by this allocator.
     * 
     * @param sz
     *      Size of the allocated block (may be ignored by some allocators).
     */
    void deallocate(void*, std::size_t) override;

    /**
     * Virtual destructor.
     */
    virtual ~allocator();
};

}

namespace tca
{

namespace internal 
{
    extern thread_local allocator* scoped_allocator;
}

/**
     * Returns the global default allocator.
     * 
     * The default allocator is used when no specific allocator is provided.
     * Initially, this points to the malloc/free allocator.
     * 
     * @return
     *      Pointer to the global default allocator.
     * 
     * @note
     *      The default allocator can be changed using set_default_allocator().
     * 
     * @see
     *      set_default_allocator()
     */
    allocator* get_default_allocator();

    /**
     * Sets the global default allocator.
     * 
     * @param allocator
     *      Pointer to the new default allocator.
     *      If nullptr, resets to the malloc/free allocator.
     * 
     * @note
     *      The allocator must remain valid for the duration of its use.
     * 
     * @see
     *      get_default_allocator()
     */
    void set_default_allocator(allocator* allocator);

    /**
     * Returns the scoped allocator if set, otherwise the default.
     * 
     * This function is used by allocation functions to determine which
     * allocator to use. It checks the thread-local scoped allocator first,
     * falling back to the global default.
     * 
     * @return
     *      Pointer to the active allocator.
     * 
     * @note
     *      The scoped allocator is set via scope_allocator RAII class.
     * 
     * @see
     *      scope_allocator
     *      get_default_allocator()
     */
    allocator* get_scoped_or_default();

    /**
     * Returns the allocator used for exception handling.
     * 
     * This function is called when exceptions are thrown and memory
     * allocation is needed for exception objects.
     * 
     * @return
     *      Pointer to the exception allocator.
     * 
     * @note
     *      By default, this returns the global default allocator.
     */
    allocator* get_exception_allocator();

/**
 * RAII class for temporarily setting a scoped allocator.
 * 
 * The scope_allocator class provides a mechanism to temporarily override
 * the default allocator for the current scope. When an instance is created,
 * it sets the thread-local scoped allocator. When the instance is destroyed,
 * the previous allocator is restored.
 * 
 * Usage pattern:
 * {
 *     scope_allocator scoped(my_custom_allocator);
 *     // All allocations within this scope use my_custom_allocator
 *     allocate_something();  // Uses scoped allocator
 * }
 * // Restores previous allocator
 * 
 * @note
 *      This class is non-copyable and non-movable.
 * 
 * @warning
 *      The allocator must outlive the scope_allocator object.
 * 
 * @see
 *      get_scoped_or_default()
 */
class scope_allocator {
    /**
     * Указатель на предыдущий распределитель.
     */
    allocator* m_prev;
    
    /**
     * 
     */
    scope_allocator(const scope_allocator&)             = delete;
    
    /**
     * 
     */
    scope_allocator& operator= (const scope_allocator&) = delete;
    
    /**
     * 
     */
    scope_allocator(scope_allocator&&)                  = delete;
    
    /**
     * 
     */
    scope_allocator& operator= (scope_allocator&&)      = delete;
    
public:
    /**
     * Constructor.
     * 
     * Sets the scoped allocator for the current thread.
     * 
     * @param allocator
     *      Pointer to the allocator to use for the current scope.
     *      Must be a valid pointer.
     * 
     * @example
     *      void my_function() {
     *          scope_allocator scoped(my_pool);
     *          // All allocations use my_pool
     *      }
     */
    scope_allocator(allocator* allocator);
    
    /**
     * Destructor.
     * 
     * Restores the previous scoped allocator.
     */
    ~scope_allocator();
    
    /**
     * Returns the previous scoped allocator.
     * 
     * @return
     *      Pointer to the allocator that was active before this scope.
     * 
     * @note
     *      Useful for debugging or for temporarily switching allocators
     *      within the same scope.
     */
    allocator* get_prev() const;
};

}

#endif /* AF0BC140_8E70_4FF6_AB4C_FE8A95461BEF */
