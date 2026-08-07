#include <internal/memory.hpp>
#include <allocators/base_allocator.hpp>
#include <cpp/lang/math.hpp>
#include <allocators/helpers.hpp>
#include <cassert>
#include <cstdint>

namespace tca
{
namespace internal
{

    page_header* page_new(page_header* next, std::size_t page_size, std::size_t align, tca::base_allocator* alloc) {
        /*
            Увеличение размера заголовка, чтобы блок, начинающийся после него, 
            имел выравнивание не меньше, чем @param align
         */
        std::size_t header_size_with_padding = align_up(sizeof(internal::page_header), tc::math::max(alignof(internal::page_header), align));

        /*
            Общий размер страницы вместе с заголовком. 
        */
        std::size_t total_page_size = header_size_with_padding + page_size;

        void* block = alloc->allocate_align(total_page_size, tc::math::max(align, alignof(internal::page_header)));
        if (!block)
            return nullptr;
        
        assert((((std::uintptr_t) block) % align) == 0);
        
        internal::page_header* hder = reinterpret_cast<internal::page_header*>(block);
        
        assert((((std::uintptr_t) &hder->dat) % alignof(void*)) == 0);
        hder->dat    = static_cast<unsigned char*>(block) + header_size_with_padding;
        
        assert((((std::uintptr_t) &hder->next) % alignof(void*)) == 0);
        hder->next = next;
        
        assert((((std::uintptr_t) &hder->size) % alignof(std::size_t)) == 0);
        hder->size   = total_page_size;

        assert((((std::uintptr_t) &hder->user_size) % alignof(std::size_t)) == 0);
        hder->user_size = page_size;

        return hder;
    }

    void page_delete(internal::page_header* page, tca::base_allocator* alloc) {
        alloc->deallocate(page, page->size);
    }


} //namespace internal
} //namespace tca