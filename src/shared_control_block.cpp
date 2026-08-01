#include <cpp/lang/utils/shared_ptr.hpp>
#include <cassert>

namespace tc
{

namespace internal
{

namespace sptr
{

        shared_control_block::shared_control_block() : 
        m_allocator(nullptr), m_object(nullptr), m_strong_refs(0), m_weak_refs(0) {

        }

        shared_control_block::shared_control_block(tca::allocator* allocator, void* object, std::size_t blocksize) :
        m_allocator(allocator), 
        m_object(object), 
        m_blocksize(blocksize),  
        m_strong_refs(0), 
        m_weak_refs(0) {

        }

        shared_control_block::~shared_control_block() {

        }

        void shared_control_block::inc_strong_ref() {
            ++m_strong_refs;
        }

        void shared_control_block::dec_strong_ref() {
            assert(m_strong_refs > 0);
            --m_strong_refs;
        }

        void shared_control_block::inc_weak_ref() {
            ++m_weak_refs;
        }
        
        void shared_control_block::dec_weak_ref() {
            assert(m_weak_refs > 0);
            --m_weak_refs;
        }

        std::size_t shared_control_block::strong_count() const {
            return m_strong_refs;
        }
        
        std::size_t shared_control_block::weak_count() const {
            return m_weak_refs;
        }
        
}// namespace sptr 

}// namespace internal 

}// namespace jstd