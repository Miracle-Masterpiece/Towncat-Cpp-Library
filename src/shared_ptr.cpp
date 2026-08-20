#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/math/math.hpp>
#include <allocators/Helpers.hpp>

namespace tc
{
namespace internal
{
    void* alloc_control_block(std::size_t t_size, std::size_t t_align, tca::allocator* alloc) {
        assert(alloc != nullptr);
        
        std::size_t max_align   = math::max(t_align, alignof(ctrl_block));
        std::size_t header_size = tca::align_up(sizeof(ctrl_block), max_align); 
        std::size_t block_size  = header_size + t_size;
        
        void* block = alloc->allocate_align(block_size, max_align);
        if (!block)
            return nullptr;

        std::uintptr_t start    = reinterpret_cast<std::uintptr_t>(block);
        std::uintptr_t user_ptr = start + header_size;
        toffset offset         = static_cast<toffset>(user_ptr - start);

        TC_ALIGN_ASSERT(start,      alignof(ctrl_block));
        TC_ALIGN_ASSERT(user_ptr,   t_align);

        ctrl_block* header = reinterpret_cast<ctrl_block*>(start);
        
        header->m_alloc     = alloc;
        header->m_strong    = 0;
        header->m_weak      = 0;

        toffset* offptr = reinterpret_cast<toffset*>(user_ptr - sizeof(toffset));
        TC_ALIGN_ASSERT(offptr, alignof(toffset));
        *offptr = offset;

        return reinterpret_cast<void*>(user_ptr);
    }

    ctrl_block* get_ctrlblock(void* p) {
        assert(p != nullptr);
        
        std::uintptr_t user_ptr = reinterpret_cast<std::uintptr_t>(p);
        
        toffset* offptr    = reinterpret_cast<toffset*>(user_ptr - sizeof(toffset));
        toffset offset     = *offptr;
        
        ctrl_block* header = reinterpret_cast<ctrl_block*>(user_ptr - offset);
        
        return header;
    }

    void free_ctrlblock(void* p) {
        assert(p != nullptr);
        ctrl_block* block = get_ctrlblock(p);
        tca::allocator* alloc = block->m_alloc;
        alloc->deallocate(block);
    }

}// namespace internal
}// namespace jstd