#ifndef DF99106C_3530_4915_A05D_4A80EFF8CA01
#define DF99106C_3530_4915_A05D_4A80EFF8CA01

#include <allocators/allocator.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/utils/raw_binary_tree.hpp>
#include <climits>

namespace tca
{

namespace internal
{

struct page_header;

// Type used for storing fallback offset.
typedef unsigned short  fallback_t;

// Type used for storing block sizes.
typedef unsigned int    memsize_t;

struct memblock {
    memsize_t        key;
    signed char      height;
    fallback_t       dummy;
    struct memblock* left;
    struct memblock* right;
};

struct large_memblock {
    struct memblock base;
    std::size_t     large_size;
    fallback_t      dummy;
};

} //namespace internal


/**
 * Variable-size free list allocator.
 * 
 * A general-purpose allocator that manages a free list of variable-size
 * memory blocks using a binary search tree keyed by block size.
 * 
 * The allocator uses a threshold-based approach:
 * - Small allocations: served from the free list (pages from parent)
 * - Large allocations: directly allocated from parent allocator (mmap-like)
 * 
 * This approach reduces fragmentation and overhead for large allocations
 * while maintaining good performance for small-to-medium allocations.
 * 
 * @note
 *      The default page size is 128KB.
 *      The threshold for large allocations is page_size - HDR_SIZE.
 * 
 * @example
 *      free_list_allocator alloc;
 *      int* arr = static_cast<int*>(alloc.allocate(100 * sizeof(int)));
 *      // Use arr...
 *      alloc.deallocate(arr);
 */
class free_list_allocator : public allocator {

    enum
    {
        // Minimum alignment.
        MIN_ALIGN       = alignof(internal::memblock),
        
        // Header size before user data.
        HDR_SIZE        = offsetof(internal::memblock, left),
        
        // Total memblock size (with tree ptrs).
        NODE_SIZE       = sizeof(internal::memblock),
        
        // Size of tree pointers.
        HDR_DIFF        = NODE_SIZE - HDR_SIZE,
        
        // Minimum usable block.
        MIN_BLOCK_SIZE  = align_up<std::size_t>(NODE_SIZE, MIN_ALIGN),
        
        // Default page size (128KB).
        PAGE_SIZE       = 1024 * 128,

        // Size of large memblock
        LARGE_BLOCK_HEADER_SIZE = sizeof(internal::large_memblock),
    };

    // Linked list of memory pages.
    internal::page_header* page_list;
    
    // 
    tc::raw_binary_tree<internal::memsize_t, tc::compare_to<internal::memsize_t>, internal::memblock> tree;

    // Size of each page.
    internal::memsize_t page_size;

    /**
     * 
     */
    free_list_allocator(const free_list_allocator&) = delete;
    
    /**
     * 
     */
    free_list_allocator& operator= (const free_list_allocator&)  = delete;

    /**
     * Allocates a new memory page.
     * 
     * Creates a new page and adds it to the free list as a single block.
     */
    void allocate_page();
    
    /**
     * Inserts a block into the free tree.
     * 
     * @param h
     *      The block to insert.
     */
    void link(internal::memblock* h);
    
    /**
     * Removes a block from the free tree.
     * 
     * @param h
     *      The block to remove.
     */
    void unlink(internal::memblock* h);
    
    /**
     * Returns the threshold for large allocations.
     * 
     * @return
     *      The maximum size that can be served from the free list.
     */
    std::size_t mmap_threshold() const {
        return page_size - HDR_SIZE;
    }
public:
    
/**
     * Allocates a large block directly from the parent allocator.
     * 
     * @param sz
     *      The requested size.
     * 
     * @param align
     *      The requested alignment.
     * 
     * @return
     *      Pointer to allocated memory, or nullptr on failure.
     * 
     * @note
     *      The actual allocation size is increased to accommodate the header
     *      and alignment padding.
     */
    void* alloc_from_parent(std::size_t sz, std::size_t align);
    
    /**
     * Frees a large block to the parent allocator.
     * 
     * @param p
     *      Pointer to the block to free.
     */
    void  free_from_parent(void* p);
    
    free_list_allocator();
    
    /**
     * Constructs with a parent allocator.
     * 
     * @param parent
     *      The parent allocator used for page allocation.
     * 
     * @param page_size
     *      The size of each memory page. Defaults to 128KB.
     *      This also determines the mmap threshold.
     */
    free_list_allocator(allocator* parent, std::size_t page_size = static_cast<std::size_t>(PAGE_SIZE));
    
    /**
     * Move constructor.
     */
    free_list_allocator(free_list_allocator&&);
    
    /**
     * Move operator.
     */
    free_list_allocator& operator= (free_list_allocator&&);
    
    /**
     * Destructor.
     * 
     * Deallocates all pages managed by this allocator.
     */
    ~free_list_allocator();

    /**
     * Allocates memory of the specified size.
     * 
     * @param sz
     *      Size in bytes to allocate.
     * 
     * @return
     *      Pointer to allocated memory, or nullptr on failure.
     * 
     * @note
     *      Calls allocate_align() with default alignment (std::max_align_t).
     */
    void* allocate(std::size_t sz) override;
    
    /**
     * Allocates aligned memory of the specified size.
     * 
     * Allocation strategy:
     * 1. If the request exceeds mmap_threshold(), delegate to parent
     * 2. Otherwise, find the smallest free block that fits
     * 3. If none exists, try compact(), then allocate a new page
     * 4. Split the block if there's enough space for a new free block
     * 5. Return the aligned user pointer
     * 
     * @param sz
     *      Size in bytes to allocate.
     * 
     * @param align
     *      Required alignment (must be a power of two).
     * 
     * @return
     *      Pointer to aligned allocated memory, or nullptr on failure.
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;
    
    /**
     * Deallocates previously allocated memory.
     * 
     * @param p
     *      Pointer to memory allocated by this allocator.
     * 
     * @note
     *      If the block is a large allocation, it's returned to the parent.
     *      Otherwise, it's added back to the free list.
     */
    void deallocate(void* p) override;
    
    /**
     * Prints debug information about the allocator state.
     * 
     * Dumps information about all pages and their blocks.
     * 
     * @note
     *      For debugging purposes only.
     */
    void print_log() const;
    
    /**
     * Compacts the memory by coalescing adjacent free blocks.
     * 
     * Scans all pages and merges adjacent free blocks into larger blocks
     * to reduce fragmentation.
     * 
     * @note
     *      Called automatically when an allocation cannot be satisfied.
     */
    void compact();
};

} //namespace tca

#endif /* DF99106C_3530_4915_A05D_4A80EFF8CA01 */
