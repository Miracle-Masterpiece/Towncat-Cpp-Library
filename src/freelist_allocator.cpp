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

    inline static memsize_t memblock_get_size(const memblock* b) {
        return b->key;
    }

    inline static void memblock_set_size(memblock* b, memsize_t sz) {
        b->key = sz;
    }

    inline static bool memblock_is_free(const memblock* b) {
        return b->height != CHAR_MAX;
    }

    inline static void memblock_set_non_free(memblock* b) {
        b->height = CHAR_MAX;
    }

} //namespace internal
} //namespace tca

namespace tca
{
    using fla = free_list_allocator;
    
    fla:: free_list_allocator() : page_list(nullptr), tree() {

    }

    fla:: free_list_allocator(allocator* parent) : allocator(parent), page_list(nullptr), tree() {

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
        internal::page_header* newpage = internal::page_new(page_list, PAGE_SIZE, MIN_ALIGN, m_parent);
        if (!newpage)
            return;
        page_list = newpage;
        
        void* block = newpage->dat;        
        
        internal::memblock* header = static_cast<internal::memblock*>(newpage->dat);
        
        internal::memblock_set_size(header, PAGE_SIZE);

        // std::printf("free space: %zu\n", internal::memblock_get_size(header));

        link(header);
    }

    void* fla:: allocate_align(std::size_t sz, std::size_t align) {
        using internal::memsize_t;
        using internal::memblock;
        using internal::memblock_set_size;
        using internal::memblock_get_size;
        using internal::memblock_set_non_free;
        using internal::fallback_t;

        align   = tc::math::max( align, static_cast<std::size_t>(MIN_ALIGN) );
        sz      = align_up( sz, static_cast<std::size_t>(MIN_ALIGN) );

        memsize_t min_need_size = static_cast<memsize_t>(sz + HDR_SIZE + align - 1);

        for (std::size_t i = 0; i < 2; ++i)
        {
            memblock* block = tree.ceil_entry(min_need_size);
            if (!block)
            {
                allocate_page();
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

            /*
                [[HDR]............................................]
                ^    ^     ^
                0    16    64
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

                memblock_set_size(current, block_size);

                memblock_set_non_free(current);

                // link(current);
            }
            
            // std::printf("align      %zu\n", align);
            // std::printf("start:     %zu\n", (std::uintptr_t) start_block);
            // std::printf("user:      %zu\n", (std::uintptr_t) user_ptr);
            // std::printf("HDR_SIZE   %zu\n", (std::size_t) HDR_SIZE);
            // std::printf("hdr_size   %zu\n", hdr_with_padd);
            // std::printf("padding    %zu\n", padding);

            
            TC_ALIGN_ASSERT(user_ptr, align);

            return reinterpret_cast<void*>(user_ptr);
        }

        return nullptr;
    }
    
    void fla:: deallocate(void* p) {
        using internal::memblock;
        using internal::fallback_t;
        
        std::uintptr_t  user_ptr     = reinterpret_cast<std::uintptr_t>(p);
        std::uintptr_t  fallback_ptr = user_ptr - sizeof(fallback_t);
        fallback_t      fallback     = *reinterpret_cast<fallback_t*>(fallback_ptr);

        memblock* block = reinterpret_cast<memblock*>(user_ptr - fallback);
        link(block);
    }

    void fla:: print_log() const {
        const internal::page_header* page = page_list;
        
        std::size_t cnt_pages = 0;
        
        while (page)
        {
            std::size_t total_size = 0;
            using internal::memblock;
            using internal::memblock_is_free;
            std::printf("############### PAGE %zu ###############\n", cnt_pages++);

            std::uintptr_t start = reinterpret_cast<std::uintptr_t>(page->dat);
            std::size_t i        = 0;
            while (i < page->user_size)
            {
                const memblock* block = reinterpret_cast<memblock*>(start + i);
                std::size_t block_size = internal::memblock_get_size(block);
                total_size += block_size;
                assert(block_size >= HDR_SIZE);
                printf("[t: %zu, u: %zu, f: %s]\n", block_size, block_size - HDR_SIZE, memblock_is_free(block) ? "true" : "false");
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