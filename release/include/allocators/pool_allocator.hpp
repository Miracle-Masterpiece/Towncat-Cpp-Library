#ifndef ADADCDE2_F2B2_4A6A_9ED4_DFBA695B431C
#define ADADCDE2_F2B2_4A6A_9ED4_DFBA695B431C

#include <allocators/allocator.hpp>
#include <cstddef>
#include <internal/memory.hpp>

namespace tca
{

namespace internal
{

/**
 * Free list node.
 * 
 * Each free block in the pool is represented as a bucket node.
 * The node stores a pointer to the next free block, forming a
 * singly linked list of available blocks.
 * 
 * @note
 *      The bucket node is stored directly in the free block memory,
 *      so no additional memory is wasted for free list management.
 */
struct bucket {
    struct bucket* next;
};

}

/**
 * Fixed-size block pool allocator.
 * 
 * A memory pool allocator that manages a fixed-size block pool.
 * All allocations return blocks of a predetermined size (bucket_size).
 * 
 * The allocator uses a page-based allocation strategy:
 * 1. When the free list is empty, a new page is allocated
 * 2. The page is divided into equal-sized blocks (buckets)
 * 3. All blocks are linked into the free list
 * 4. Allocations pop from the free list
 * 5. Deallocations push back to the free list
 * 
 * Memory layout of a page:
 * 
 * --------------------
 * | page_header      |
 * | - next           |
 * | - size           |
 * | - dat            |
 * |------------------| <- aligned to max(align, alignof(page_header))
 * | bucket[0]        |
 * |------------------|
 * | bucket[1]        |
 * |------------------|
 * | bucket[2]        |
 * |------------------|
 * | ...              |
 * |------------------|
 * | bucket[N-1]      |
 * |------------------|
 * 
 * @note
 *      This allocator is not thread-safe by default.
 *      For thread-safe usage, wrap it with a mutex.
 * 
 * @see
 *      allocator
 *      page_header
 *      bucket
 */
class pool_allocator : public allocator {

    internal::page_header*  pages;
    internal::bucket*       free_list;
    std::size_t             align;
    std::size_t             bucket_size;
    std::size_t             cnt_buckets_per_page;

    /**
     * Allocates a new memory page.
     * 
     * Allocates a page from the parent allocator, divides it into
     * equal-sized blocks, and adds all blocks to the free list.
     * 
     * The page size is calculated as:
     * 
     * header_size  = align_up(sizeof(page_header), align)
     * page_size    = header_size + bucket_size * cnt_buckets_per_page
     * 
     * @note
     *      The page header is placed at the start of the page.
     *      The data blocks follow immediately after the header.
     * 
     * @warning
     *      If allocation fails, the free list remains unchanged.
     */
    void allocate_page();
    
    /**
     * 
     */
    pool_allocator(const pool_allocator&) = delete;
    
    /**
     * 
     */
    pool_allocator& operator=(const pool_allocator&) = delete;

public:
    using allocator::deallocate;

    /**
     * Default constructor.
     * 
     * Constructs an empty pool allocator in a moved-from state.
     * The allocator is not usable until assigned from a valid pool_allocator.
     * 
     * @note
     *      This constructor exists primarily for move semantics support.
     *      A default-constructed pool_allocator cannot allocate memory.
     */
    pool_allocator();

    /**
     * Move constructor.
     * 
     * Transfers ownership of resources from another pool_allocator.
     * The source allocator is left in a valid but unspecified state.
     * 
     * @param alloc
     *      The pool_allocator to move from.
     * 
     * @note
     *      After move, the source allocator can be destroyed or assigned to.
     *      Using it for allocations before assignment is undefined behavior.
     */
    pool_allocator(pool_allocator&&);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of resources from another pool_allocator.
     * Any existing resources are released before acquiring new ones.
     * 
     * @param alloc
     *      The pool_allocator to move from.
     * 
     * @return
     *      Reference to this allocator.
     * 
     * @note
     *      Self-assignment is handled safely.
     *      The source allocator is left in a valid but unspecified state.
     */
    pool_allocator& operator=(pool_allocator&&);

    /**
     * Constructs a pool allocator.
     * 
     * @param bucket_size
     *      Size of each block in bytes. The actual block size is
     *      aligned up to the specified alignment.
     * 
     * @param count_buckets
     *      Count buckets of each block.
     * 
     * @param align
     *      Alignment requirement for blocks.
     *      Defaults to alignof(std::max_align_t).
     *      Must be a power of two.
     * 
     * @param alloc
     *      Parent allocator used to allocate memory pages.
     *      Defaults to the global default allocator.
     * 
     * @note
     *      The bucket_size is rounded up to satisfy the alignment
     *      requirement.
     * 
     * @example
     *      // Pool for 64-byte aligned objects
     *      pool_allocator pool(64, alignof(int));
     */
    pool_allocator(std::size_t bucket_size, std::size_t count_buckets = 64, std::size_t align = alignof(std::max_align_t), allocator* alloc = tca::get_default_allocator());
    
    /**
     * Allocates a single block from the pool.
     * 
     * @return
     *      Pointer to the allocated block, or nullptr if allocation fails.
     * 
     * @note
     *      If the free list is empty, a new page is allocated.
     *      If allocation of a new page fails, returns nullptr.
     */
    void* allocate();
    
    /**
     * Deallocates a block back to the pool.
     * 
     * @param p
     *      Pointer to the block to deallocate.
     *      Must be a valid pointer previously returned by allocate()
     *      from this pool allocator.
     * 
     * @note
     *      The pointer is added to the free list and becomes
     *      available for future allocations.
     * 
     * @warning
     *      Passing an invalid pointer (not from this pool) results
     *      in undefined behavior.
     */
    void  deallocate(void* p) override;
    
    /**
     * Allocates memory of the specified size.
     * 
     * Allocates a block from the pool. Memory is guaranteed to be aligned
     * to at least alignof(std::max_align_t) as required by the allocator interface.
     * 
     * @param sz
     *      Size in bytes to allocate.
     * 
     * @return
     *      Pointer to allocated memory, or nullptr if:
     *      - sz > bucket_size (block too large)
     *      - align < alignof(std::max_align_t) (insufficient alignment)
     *      - Allocation fails
     * 
     * @note
     *      Only supports allocations up to bucket_size.
     */
    void* allocate(std::size_t sz) override;

    /**
     * Allocates aligned memory of the specified size.
     * 
     * This implementation only allocates blocks of size bucket_size
     * with alignment align_.
     * 
     * @param sz
     *      Size in bytes to allocate.
     * 
     * @param align_
     *      Required alignment (must be <= pool_allocator::align).
     * 
     * @return
     *      Pointer to allocated memory, or nullptr if:
     *      - sz > bucket_size
     *      - align_ > pool_allocator::align
     *      - Allocation fails
     * 
     * @note
     *      This function is provided for allocator interface compliance.
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;

    /**
     * 
     */
    ~pool_allocator();
};

    /**
     * Creates a pool allocator for a specific type.
     * 
     * Convenience function that creates a pool_allocator configured for
     * objects of type T. The bucket size is automatically set to sizeof(T)
     * and alignment to alignof(T).
     * 
     * @tparam T
     *      The type for which the pool allocator is created.
     * 
     * @param count
     *      Number of objects per page. Defaults to 128.
     *      This determines how many T objects each page will hold.
     * 
     * @param alloc
     *      Parent allocator used for page allocation.
     *      Defaults to the global default allocator.
     * 
     * @return
     *      A pool_allocator configured for type T.
     * 
     * @example
     *      // Create a pool for 128 MyClass objects per page
     *      tca::pool_allocator pool = tca::make_pool<MyClass>();
     *      
     *      // Allocate a MyClass object
     *      MyClass* obj = static_cast<MyClass*>(pool.allocate());
     * 
     *      // Create a pool with 256 objects per page
     *      tca::pool_allocator large_pool = make_pool<MyClass>(256);
     */
    template<typename T>
    pool_allocator make_pool(std::size_t count = 128, allocator* alloc = get_default_allocator()) {
        return pool_allocator(sizeof(T), count, alignof(T), alloc);
    }

} //namespace tca

#endif /* ADADCDE2_F2B2_4A6A_9ED4_DFBA695B431C */
