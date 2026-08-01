#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/math/math.hpp>
#include <allocators/Helpers.hpp>

namespace tc
{
namespace internal
{
namespace sptr 
{
    std::size_t calc_ctr_block_total_size(std::size_t data_type_sizeof, std::size_t data_type_alignof, std::size_t* offset_to_object) {
        const std::size_t ctrl_block_sz            = sizeof(shared_control_block);
        const std::size_t padding                  = tca::align_up(ctrl_block_sz, data_type_alignof);
        const std::size_t ctrl_block_total_size    = ctrl_block_sz + padding + data_type_sizeof;
        if (offset_to_object != nullptr)
            (*offset_to_object) = (ctrl_block_sz + padding);
        return ctrl_block_total_size;
    }

    shared_control_block* alloc_memory_to_control_block(tca::allocator* allocator, std::size_t object_size, std::size_t object_align, std::size_t n_objects) {        
        std::size_t ctrl_block_size     = sizeof(shared_control_block);
        std::size_t padding             = tca::align_up(ctrl_block_size, object_align);
        std::size_t offset_to_object    = ctrl_block_size + padding;
        std::size_t objects_size        = object_size * n_objects;
        std::size_t total_size          = ctrl_block_size + padding + objects_size;
        void* p                         = allocator->allocate_align(total_size, math::max(alignof(shared_control_block), object_align));         
        if (!p)
            return nullptr;
        using byte_t = unsigned char;
        byte_t* block = reinterpret_cast<byte_t*>(p);
        shared_control_block* ctrl_block = reinterpret_cast<shared_control_block*>(block); {
            ctrl_block->m_allocator     = allocator;
            ctrl_block->m_blocksize     = total_size;
            ctrl_block->m_object        = block + offset_to_object;
            ctrl_block->m_strong_refs   = 0;
            ctrl_block->m_weak_refs     = 0;
        }
        JSTD_DEBUG_CODE(
            assert(((uintptr_t) ctrl_block->m_object) % (uintptr_t) object_align == 0);
        );
        return ctrl_block;
    }

}// namespace sptr 
}// namespace internal
}// namespace jstd