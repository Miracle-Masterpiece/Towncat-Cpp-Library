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

typedef unsigned short  fallback_t;
typedef unsigned int    memsize_t;

struct memblock {
    struct memblock* left;
    struct memblock* right;
    memsize_t   key;
    signed char height;
    fallback_t  dummy;
};

} //namespace internal

class free_list_allocator : public allocator {
    
    enum {
        MIN_ALIGN       = alignof(internal::memblock),
        HDR_SIZE        = sizeof(internal::memblock),
        MIN_BLOCK_SIZE  = align_up<std::size_t>(HDR_SIZE + 16, MIN_ALIGN),
        PAGE_SIZE       = 1024 * 1024 * 16,
    };

    // 
    internal::page_header* page_list;
    
    // 
    tc::raw_binary_tree<internal::memsize_t, tc::compare_to<internal::memsize_t>, internal::memblock> tree;

    /**
     * 
     */
    free_list_allocator(const free_list_allocator&) = delete;
    
    /**
     * 
     */
    free_list_allocator& operator= (const free_list_allocator&)  = delete;

    /**
     * 
     */
    void allocate_page();

    void link(internal::memblock* h);
    void unlink(internal::memblock* h);
public:

    free_list_allocator();
    free_list_allocator(allocator* parent);

    void* allocate(std::size_t sz) override;
    void* allocate_align(std::size_t sz, std::size_t align) override;
    void  deallocate(void* p) override;

    void print_log() const;

    ~free_list_allocator();
};

} //namespace tca

#endif /* DF99106C_3530_4915_A05D_4A80EFF8CA01 */
