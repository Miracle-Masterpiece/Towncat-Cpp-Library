#ifndef EF98B38A_B022_4DAF_88E7_E592F3B5A998
#define EF98B38A_B022_4DAF_88E7_E592F3B5A998

#include <allocators/allocator.hpp>
#include <cpp/lang/concurrency/mutex.hpp>

namespace tca
{

/**
 * Provides thread-safe allocation and deallocation by wrapping any allocator
 * with a mutex. All allocation and deallocation operations are synchronized
 * using a mutex, making the wrapper safe for concurrent use from multiple threads.
 */
class synchronized_allocator : public allocator {
    allocator*  m_alloc;
    tc::mutex   m_mutex;
public:

    /**
     * Constructor from an allocator pointer.
     * 
     * Wraps the provided allocator with thread-safety guarantees.
     * 
     * @param alloc
     *      Pointer to the allocator to wrap.
     */
    synchronized_allocator(allocator* alloc);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the wrapped allocator pointer from another
     * synchronized_allocator object. The mutex of the source object is not
     * transferred; the new object gets a new mutex.
     * 
     * @param a
     *      The synchronized_allocator object to move from.
     */
    synchronized_allocator(synchronized_allocator&&);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the wrapped allocator pointer from another
     * thread_safe_wrapper object.
     * 
     * @param a
     *      The thread_safe_wrapper object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    synchronized_allocator& operator= (synchronized_allocator&&);
    
    /**
     * Allocates memory in a thread-safe manner.
     * 
     * Locks the mutex, delegates the allocation to the wrapped allocator,
     * and unlocks the mutex before returning.
     * 
     * @param sz
     *      Number of bytes to allocate.
     * @return
     *      Pointer to the allocated memory, or nullptr if allocation fails.
     */
    void* allocate(std::size_t) override;

    /**
     * Allocates aligned memory in a thread-safe manner.
     * 
     * Locks the mutex, delegates the aligned allocation to the wrapped allocator,
     * and unlocks the mutex before returning.
     * 
     * @param sz
     *      Number of bytes to allocate.
     * 
     * @param align
     *      Alignment requirement (must be a power of two).
     * 
     * @return
     *      Pointer to the allocated aligned memory, or nullptr if allocation fails.
     */
    void* allocate_align(std::size_t, std::size_t) override;

    /**
     * Deallocates memory in a thread-safe manner.
     * 
     * Locks the mutex, delegates the deallocation to the wrapped allocator,
     * and unlocks the mutex before returning.
     * 
     * @param p
     *      Pointer to the memory to deallocate.
     */
    void deallocate(void*) override;

    /**
     * Deallocates memory with size information in a thread-safe manner.
     * 
     * Locks the mutex, delegates the deallocation to the wrapped allocator,
     * and unlocks the mutex before returning.
     * 
     * @param p
     *      Pointer to the memory to deallocate.
     * 
     * @param sz
     *      Size of the memory block (optional, may be ignored).
     */
    void deallocate(void*, std::size_t) override;
private:
    synchronized_allocator(const synchronized_allocator&) = delete;
    synchronized_allocator& operator= (const synchronized_allocator&) = delete;
};

}

#endif /* EF98B38A_B022_4DAF_88E7_E592F3B5A998 */
