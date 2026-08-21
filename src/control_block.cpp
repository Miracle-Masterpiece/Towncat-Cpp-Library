#include <internal/smart_ptrs/control_block.hpp>

namespace tc
{
namespace internal
{
    control_block::~control_block() {
        
    }

    void control_block::destroy_control_block() {
        tca::allocator* alloc = m_allocator;
        
        control_block* self = this;
        self->~control_block();
        
        alloc->deallocate(static_cast<void*>(this));
    }
} //namespace internal
} //namespace tc