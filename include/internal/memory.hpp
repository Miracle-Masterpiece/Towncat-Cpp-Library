#ifndef FD3886F5_C3F2_43D5_B795_2668A12DE7EF
#define FD3886F5_C3F2_43D5_B795_2668A12DE7EF

#include <cstddef>
#define TC_ALIGN_ASSERT(ptr, align) assert(((std::uintptr_t) ptr % align) == 0)

namespace tca
{
namespace internal
{

struct page_header {
    struct 
    page_header* next;
    void*        dat;
    std::size_t  size;
    std::size_t  user_size;
};

    /**
     * Allocates and initializes a new memory page.
     * 
     * Creates a new page with a header followed by a data block of the specified size.
     * The page is aligned according to the specified alignment requirements.
     * 
     * The page layout:
     * |---------------------------|  <- block (aligned to max(align, alignof(page_header)))
     * | page_header               |
     * | - next                    |
     * | - dat  -------------------|---
     * | - size                    |  |
     * | - user_size               |  |
     * |---------------------------|  |
     * | ... (padding)             |  |
     * |---------------------------|  |
     * | user data (page_size)     |  |
     * |                           |  |
     * |---------------------------|  |
     * ^                              |
     * hder->dat  ---------------------
     * 
     * @param next
     *      Pointer to the next page in the linked list.
     * 
     * @param page_size
     *      Size of the user data block in bytes.
     * 
     * @param align
     *      Alignment requirement for the user data block.
     *      Must be a power of two.
     * 
     * @param alloc
     *      Allocator used to allocate the page memory.
     * 
     * @return
     *      Pointer to the initialized page_header, or nullptr if allocation fails.
     * 
     * @note
     *      The header size is padded to ensure the user data block starts
     *      at the required alignment boundary.
     * 
     * @warning
     *      The returned page_header contains a pointer to the user data block
     *      (hder->dat) which is correctly aligned to the requested alignment.
     */
    page_header* page_new(page_header* next, std::size_t page_size, std::size_t align, class base_allocator* alloc);

    /**
     * Deallocates a memory page.
     * 
     * Frees the page memory using the provided allocator.
     * 
     * @param page
     *      Pointer to the page_header to deallocate.
     *      Must be a valid page previously returned by page_new().
     * 
     * @param alloc
     *      Allocator used to deallocate the page memory.
     * 
     * @note
     *      The page->size field is used to determine the total memory
     *      to deallocate.
     */
    void page_delete(page_header* page, class base_allocator* alloc);

} //namespace internal
} //namespace tca

#endif /* FD3886F5_C3F2_43D5_B795_2668A12DE7EF */
