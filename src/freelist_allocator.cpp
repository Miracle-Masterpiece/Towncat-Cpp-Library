#include <allocators/freelist_allocator.hpp>
#include <cpp/lang/math.hpp>
#include <internal/memory.hpp>
#include <cstddef>

#include <iostream>
#include <cstdio>


namespace tca
{
namespace internal
{

    /**
     * Flags for memblock key field.
     * 
     * The key field stores both the block size and flags:
     * USE_BIT:     set if block is currently allocated
     * LARGE_BIT:   set if block is a large allocation from parent
     * 
     * Block size is stored in the lower bits with flags masked out.
     */

    // Bit indicating block is allocated.
    const memsize_t USE_BIT    = 0x01u;
    
    // Bit indicating large allocation.
    const memsize_t LARGE_BIT  = 0x02u;
    
    // Mask to clear flag bits.
    const memsize_t CLEAR_BITS = ~0x03u;

    /**
     * Sets or clears a flag in the block key.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param flag
     *      The flag bit to modify.
     * 
     * @param use
     *      true to set the flag, false to clear it.
     */
    inline static void memblock_set_flag(memblock* b, memsize_t flag, bool use) {
        memsize_t bit = use ? flag : 0;
        b->key &= ~flag;
        b->key |= bit;
    }
    
    /**
     * Checks if a flag is set in the block key.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param flag
     *      The flag bit to check.
     * 
     * @return
     *      true if the flag is set, false otherwise.
     */
    inline static bool memblock_get_flag(const memblock* b, memsize_t flag) {
        return b->key & flag;
    }

    /**
     * Sets or clears the LARGE_BIT flag.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param use
     *      true for large allocation, false for normal block.
     */
    inline static void memblock_setlarge(memblock* b, bool use) {
        memblock_set_flag(b, LARGE_BIT, use);
    }

    /**
     * Checks if the block is a large allocation.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      true if the block is a large allocation from parent.
     */
    inline static bool memblock_is_large(const memblock* b) {
        return memblock_get_flag(b, LARGE_BIT);
    }

    /**
     * Retrieves the block size (without flags).
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      The block size in bytes.
     */
    inline static memsize_t memblock_get_size(const memblock* b) {
        return b->key & CLEAR_BITS;
    }

    /**
     * Sets the block size (clears flags).
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param sz
     *      The block size in bytes.
     */
    inline static void memblock_set_size(memblock* b, memsize_t sz) {
        b->key = sz;
    }
    
    /**
     * Retrieves the size of a large allocation.
     * 
     * @param b
     *      Pointer to the block (must be marked as large).
     * 
     * @return
     *      The large allocation size in bytes.
     */
    inline static std::size_t memblock_get_largesize(const memblock* b) {
        assert(memblock_is_large(b));
        return b->large_size;
    }

    /**
     * Marks a block as a large allocation and stores its size.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param sz
     *      The large allocation size in bytes.
     */
    inline static void memblock_set_largesize(memblock* b, std::size_t sz) {
        memblock_set_size(b, 0);
        memblock_setlarge(b, true);
        b->large_size = sz;
    }

    inline static void memblock_setuse(memblock* b, bool use) {
        memblock_set_flag(b, USE_BIT, use);
    }

    /**
     * Checks if the block is currently allocated.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      true if allocated, false if free.
     */
    inline static bool memblock_is_use(const memblock* b) {
        return memblock_get_flag(b, USE_BIT);
    }

} //namespace internal
} //namespace tca

namespace tca
{
    using fla = free_list_allocator;
    
    fla:: free_list_allocator() : allocator(nullptr), page_list(nullptr), tree(), page_size(0) {

    }

    fla:: free_list_allocator(allocator* parent, std::size_t page_size) :
    allocator(parent),
    page_list(nullptr),
    tree(),
    page_size(static_cast<internal::memsize_t>(page_size)) {

    }

    fla:: free_list_allocator(free_list_allocator&& alloc) :
    allocator(std::move(alloc)),
    page_list(alloc.page_list),
    tree(std::move(alloc.tree)),
    page_size(alloc.page_size) {
        alloc.page_list = nullptr;
        alloc.page_size = 0;
    }
    
    free_list_allocator& fla:: operator= (free_list_allocator&& alloc) {
        if (&alloc == this)
            return *this;
        std::swap(page_list, alloc.page_list);
        std::swap(tree,      alloc.tree);
        std::swap(page_size, alloc.page_size);
        return *this;
    }

    
    void* fla:: allocate(std::size_t sz) {
        return allocate_align(sz, alignof(std::max_align_t));
    }
    
    void fla:: link(internal::memblock* h) {
        h->left     = nullptr;
        h->right    = nullptr;
        h->height   = 0;
        tree.insert_entry(h);
    }
    
    void fla:: unlink(internal::memblock* h) {
        internal::memblock* unlinked = tree.remove_entry(h);
        assert(h == unlinked);
    }

    void fla:: allocate_page() {
        internal::page_header* newpage = internal::page_new(page_list, page_size, MIN_ALIGN, m_parent);
        if (!newpage)
            return;
        page_list = newpage;
        
        internal::memblock* header = static_cast<internal::memblock*>(newpage->dat);
        internal::memblock_set_size(header, page_size);

        link(header);
    }

    void* fla:: alloc_from_parent(std::size_t sz, std::size_t align) {
        using internal::memsize_t;
        using internal::memblock;
        using internal::memblock_setuse;
        using internal::memblock_set_largesize;
        using internal::fallback_t;
        
        sz = NODE_SIZE + (align - 1) + sz;
        void* memory = m_parent->allocate_align(sz, alignof(memblock));
        
        std::uintptr_t  hdr      = reinterpret_cast<std::uintptr_t>(memory);
        std::uintptr_t  user_ptr = align_up(hdr + NODE_SIZE, align);
        
        fallback_t*     fallback_ptr = reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback_t));
        
        TC_ALIGN_ASSERT(fallback_ptr, alignof(fallback_t));
        *fallback_ptr = static_cast<fallback_t>(user_ptr - hdr);

        memblock* block = reinterpret_cast<memblock*>(hdr);
        memblock_set_largesize(block, sz);

        TC_ALIGN_ASSERT(user_ptr, align);
        return reinterpret_cast<void*>(user_ptr);
    }
    
    void  fla:: free_from_parent(void* p) {
        using internal::memblock;
        using internal::memblock_get_largesize;
        using internal::fallback_t;
        
        std::uintptr_t user_ptr = reinterpret_cast<std::uintptr_t>(p);
        fallback_t fallback     = *reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback));
        memblock* block         = reinterpret_cast<memblock*>(user_ptr - fallback);

        m_parent->deallocate(block, block->large_size);
    }

    void* fla:: allocate_align(std::size_t sz, std::size_t align) {
        assert(m_parent != nullptr);
        
        using internal::memsize_t;
        using internal::memblock;
        using internal::memblock_set_size;
        using internal::memblock_get_size;
        using internal::memblock_setuse;
        using internal::fallback_t;

        align   = tc::math::max( align, static_cast<std::size_t>(MIN_ALIGN) );
        sz      = tc::math::max(sz, static_cast<std::size_t>(HDR_DIFF));
        sz      = align_up( sz, static_cast<std::size_t>(MIN_ALIGN) );
        memsize_t min_need_size = static_cast<memsize_t>(HDR_SIZE + sz + align - 1);

        if (min_need_size >= mmap_threshold())
        {
            return alloc_from_parent(sz, align);
        }

        for (std::size_t i = 0; i < 3; ++i)
        {
            memblock* block = tree.ceil_entry(min_need_size);
            if (!block)
            {
                if (i == 0)
                {
                    compact();
                } else if (i == 1)
                {
                    allocate_page();
                }
                continue;
            }
            unlink(block);

            // Указатель на начало блока (первый байт заголовка)
            std::uintptr_t  start_block  = reinterpret_cast<std::uintptr_t>(block);
            
            // Указатель на пользовательский заголовок
            std::uintptr_t  user_ptr     = align_up<std::uintptr_t>(start_block + HDR_SIZE, align);
            
            // Размер заголовка (вместе с padding)
            memsize_t     hdr_with_padd  = static_cast<memsize_t>(user_ptr - start_block);
            
            // Размер padding
            memsize_t     padding        = hdr_with_padd - HDR_SIZE;

            // Общий размер блока (Включая заголовок)
            memsize_t block_size = memblock_get_size(block);

            // std::printf("align          %zu\n", align);
            // std::printf("start:         %zu\n", (std::uintptr_t) start_block);
            // std::printf("user:          %zu\n", (std::uintptr_t) user_ptr);
            // std::printf("HDR_SIZE       %zu\n", (std::size_t) HDR_SIZE);
            // std::printf("hdr_with_padd  %zu\n", hdr_with_padd);
            // std::printf("padding        %zu\n", padding);

            /*
                [[HDR]..............................................]
                ^    ^    ^          ^
                0    8    24         64
                
                [[HDR]....]......[HDR]....]...................................]
                ^    ^    ^          ^
                0    8    24         128
            */

            // Разделение, если паддинг слишком большой, и туда можно поместить блок памяти
            if (padding >= MIN_BLOCK_SIZE)
            {
                memblock* front  = reinterpret_cast<memblock*>(start_block);
                memblock_set_size(front, padding);
                block_size      -= padding;
                start_block     += padding;
                link(front);
            }
            
            // Количество байт, занимаемое этим блоком. В целом header + padding + usersize
            memsize_t used = static_cast<memsize_t>(user_ptr + sz - start_block);

            assert(block_size >= used);

            // Сколько свободных байт остаётся впереди
            memsize_t rem  = block_size - used;

            // Разделение, если размер текущего блока слишком большой
            if (rem >= MIN_BLOCK_SIZE)
            {
                memblock* next  = reinterpret_cast<memblock*>(user_ptr + sz);
                TC_ALIGN_ASSERT(next, alignof(memblock));
                
                memblock_set_size(next, rem);
                block_size      -= rem;
                link(next);
            }

            {
                memsize_t fallback = static_cast<memsize_t>(user_ptr - start_block);
                
                /*
                    Записываем в user_ptr - sizeof(fallback_t) значение (user_ptr - start_block), 
                    сколько нужно сдвинуться назад, чтобы получить фактический указатель на заголовок
                */
                
                *reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback_t)) = static_cast<fallback_t>(fallback);

                memblock* current = reinterpret_cast<memblock*>(start_block);
                TC_ALIGN_ASSERT(current, alignof(memblock));

                memblock_set_size(current,  block_size);
                memblock_setuse(current,    true);

                // link(current);
            }
            
        
            TC_ALIGN_ASSERT(user_ptr, align);
            return reinterpret_cast<void*>(user_ptr);

            // return nullptr;
        }

        return nullptr;
    }
    
    void fla:: deallocate(void* p) {
        if (!p)
            return;

        using internal::memblock;
        using internal::fallback_t;
        using internal::memblock_setuse;
        using internal::memblock_is_large;
        
        std::uintptr_t  user_ptr     = reinterpret_cast<std::uintptr_t>(p);
        std::uintptr_t  fallback_ptr = user_ptr - sizeof(fallback_t);
        fallback_t      fallback     = *reinterpret_cast<fallback_t*>(fallback_ptr);

        memblock* block = reinterpret_cast<memblock*>(user_ptr - fallback);

        if (memblock_is_large(block))
        {
            free_from_parent(p);
            return;
        }

        memblock_setuse(block, false);
        link(block);
    }

    void fla:: compact() {
        
        using internal::memblock;
        using internal::memblock_is_use;
        using internal::memblock_get_size;
        using internal::memblock_set_size;
        using internal::memsize_t;
        
        const internal::page_header* page = page_list;
        
        while (page)
        {

            std::uintptr_t start = reinterpret_cast<std::uintptr_t>(page->dat);
            std::size_t i        = 0;
            while (i < page->user_size)
            {
                memblock* block      = reinterpret_cast<memblock*>(start + i);
                memsize_t block_size = internal::memblock_get_size(block);
                
                if (i + block_size >= page->user_size)
                {
                    break;
                }

                if (!memblock_is_use(block))
                {
                    memblock* next  = reinterpret_cast<memblock*>(start + i + block_size);
                    if (!memblock_is_use(next))
                    {
                        unlink(block);
                        unlink(next);
                        block_size += memblock_get_size(next);
                        memblock_set_size(block, block_size);
                        link(block);
                        continue;
                    }
                }
                
                i += block_size;
            }

            page = page->next;
        }
    }

    void fla:: print_log() const {
        const internal::page_header* page = page_list;
        
        std::size_t cnt_pages = 0;
        
        while (page)
        {
            std::size_t total_size = 0;
            using internal::memblock;
            using internal::memblock_is_use;
            std::printf("############### PAGE %zu ###############\n", cnt_pages++);

            std::uintptr_t start = reinterpret_cast<std::uintptr_t>(page->dat);
            std::size_t i        = 0;
            while (i < page->user_size)
            {
                const memblock* block = reinterpret_cast<memblock*>(start + i);
                std::size_t block_size = internal::memblock_get_size(block);
                total_size += block_size;
                // assert(block_size >= HDR_SIZE);
                printf("[t: %zu, u: %zu, f: %s]\n", block_size, block_size - HDR_SIZE, memblock_is_use(block) ? "false" : "true");
                i += internal::memblock_get_size(block);
            }

            assert(total_size == page->user_size);
            
            page = page->next;
        }

    }

    fla:: ~free_list_allocator() {
        using internal::page_header;
        using internal::page_delete;
        for (page_header* page = page_list; page != nullptr; )
        {
            page_header* next = page->next;
            page_delete(page, m_parent);
            page = next;
        }
    }
}