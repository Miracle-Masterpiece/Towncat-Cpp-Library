#include <allocators/arena_free_list_allocator.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/system.hpp>
#include <utility>


namespace tca
{

    /**
    * 
    */
    struct arena_free_list_allocator::memblock {
        memblock*   left;
        memblock*   right;
        union {
            std::size_t key;
            std::size_t m_size;
        };
        signed char height;
        bool m_free;
    };

    arena_free_list_allocator::arena_free_list_allocator() : 
        allocator(nullptr),
        m_alignas(0),
        m_header_size(0),
        m_min_block_size(0),
        m_data(nullptr),
        m_length(0),
        m_start(nullptr),
        m_free_size(0),
        m_tree() {

    }

    void arena_free_list_allocator::init(void* data, std::size_t length, std::size_t align) {
        m_length            = length;
        m_header_size       = align_up(sizeof(memblock), align);
        m_min_block_size    = m_header_size + m_alignas;
        m_alignas           = align;
        m_data              = data;
        {//выравнивание адреса
            std::uintptr_t padding = calc_padding_for((uintptr_t) m_data, (uintptr_t) m_alignas);
            m_start = static_cast<void*>(static_cast<unsigned char*>(m_data) + padding);
            TC_ALIGN_ASSERT(m_start, m_alignas);
            m_free_size = length - padding;
        }
        memblock* block = reinterpret_cast<memblock*>(m_start);
        block->m_size   = m_free_size - m_header_size;
        link(block);
    }

    arena_free_list_allocator::arena_free_list_allocator(std::size_t length, std::size_t align, tca::base_allocator* allocator) : 
        arena_free_list_allocator() {
        void* data = allocator->allocate_align(length, align);
        if (!data) 
            return;
        m_parent = allocator;
        init(data, length, align);
    }

    arena_free_list_allocator::arena_free_list_allocator(void* data, std::size_t length, std::size_t align) :
        allocator(nullptr),
        m_alignas(align),
        m_header_size(align_up(sizeof(memblock), m_alignas)),
        m_min_block_size(m_header_size + m_alignas),
        m_data(data),
        m_length(length),
        m_start(nullptr),
        m_free_size(0),
        m_tree() {
        init(data, length, align);
    }

    arena_free_list_allocator::arena_free_list_allocator(arena_free_list_allocator&& alloc) : 
        allocator(std::move(alloc)),
        m_alignas(alloc.m_alignas),
        m_header_size(alloc.m_header_size),
        m_min_block_size(alloc.m_min_block_size),
        m_data(alloc.m_data),
        m_length(alloc.m_length),
        m_start(alloc.m_start),
        m_free_size(alloc.m_free_size),
        m_tree(std::move(alloc.m_tree)) {
        alloc.m_alignas           = 0;
        alloc.m_header_size       = 0;
        alloc.m_min_block_size    = 0;
        alloc.m_data              = nullptr;
        alloc.m_length            = 0;
        alloc.m_start             = nullptr;
        alloc.m_free_size         = 0;
    }

    void arena_free_list_allocator::cleanup() {
        if (m_parent != nullptr && m_data != nullptr) {
            m_parent->deallocate(m_data, m_length);
            m_data   = nullptr;
            m_parent = nullptr;
        }
    }

    arena_free_list_allocator& arena_free_list_allocator::operator= (arena_free_list_allocator&& alloc) {
        if (&alloc != this) {
            cleanup();
            allocator::operator=(std::move(alloc));
            m_alignas           = alloc.m_alignas;
            m_header_size       = alloc.m_header_size;
            m_min_block_size    = alloc.m_min_block_size;
            m_data              = alloc.m_data;
            m_length            = alloc.m_length;
            m_start             = alloc.m_start;
            m_free_size         = alloc.m_free_size;
            m_tree              = std::move(alloc.m_tree);
            alloc.m_alignas           = 0;
            alloc.m_header_size       = 0;
            alloc.m_min_block_size    = 0;
            alloc.m_data              = nullptr;
            alloc.m_length            = 0;
            alloc.m_start             = nullptr;
            alloc.m_free_size         = 0;
        }
        return *this;
    }

    void arena_free_list_allocator::link(memblock* block) {
        assert(block != nullptr);
        block->left     = nullptr;
        block->right    = nullptr;
        block->height   = 0;
        block->m_free   = true;
        m_tree.insert_entry(block);
    }
    
    void arena_free_list_allocator::unlink(memblock* block) {
        assert(block != nullptr);
        block->m_free = false;
        memblock* removed = m_tree.remove_entry(block);
        assert(removed == block);
    }

    arena_free_list_allocator::~arena_free_list_allocator() {
        cleanup();
    }

    void* arena_free_list_allocator::allocate(std::size_t sz) {
        void* const p = allocate_align(sz, m_alignas);
        assert(((std::uintptr_t) p % m_alignas) == 0);
        return p;
    }
        
    void* arena_free_list_allocator::allocate_align(std::size_t sz, std::size_t) {
        if (!m_data)
            return nullptr;
        sz = align_up(sz, m_alignas);
        
        memblock* block = m_tree.ceil_entry(sz);
        if (!block) {
            merge_all();
            block = m_tree.ceil_entry(sz);
            if (!block)
                return nullptr;
        }

        if (can_split(block, sz))
            split(block, sz);

        unlink(block);

        return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(block) + m_header_size);
    }

    bool arena_free_list_allocator::can_split(const memblock* block, std::size_t piece) const {
        assert(block != nullptr);
        return (block->m_size > piece) && (block->m_size - piece >= m_min_block_size);
    }

    void arena_free_list_allocator::split(memblock* block, std::size_t piece) {
        assert(block != nullptr);
        assert(can_split(block, piece));
        assert((piece % m_alignas) == 0);
        
        memblock* next  = reinterpret_cast<memblock*>(reinterpret_cast<unsigned char*>(block) + (m_header_size + piece));
        next->m_size    = block->m_size - piece - m_header_size;
        block->m_size   = piece;
        
        link(next);
    }

    void arena_free_list_allocator::merge_all() {
        typedef unsigned char uchar;
        typedef memblock mb;
        for (std::size_t off = 0; off < m_free_size; ) {
            mb* block = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + off);
            if (block->m_free) {                
                const std::size_t offset_to_next_block = off + m_header_size + block->m_size;
                if (offset_to_next_block < m_free_size) {
                    mb* next = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + offset_to_next_block);
                    if (next->m_free) {
                        memblock* deleted = m_tree.remove_entry(next);
                        assert(deleted != nullptr);
                        assert(deleted == next);
                        block->m_size += m_header_size + next->m_size;
                        continue;
                    }
                }
            }
            off += (m_header_size + block->m_size);
        }
        print_log();
    }

    void arena_free_list_allocator::deallocate(void* p) {
        if (p == nullptr) 
            return;
        link(reinterpret_cast<memblock*>(reinterpret_cast<unsigned char*>(p) - m_header_size));
    }

    void arena_free_list_allocator::print_log() const {
        typedef unsigned char uchar;
        typedef memblock mb;
        for (std::size_t off = 0; off < m_free_size; ) {
            mb* block = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + off);
            tc::system::tsprintf("[p: %p, is_free: %s, sz: %zu]\n", reinterpret_cast<void*>(block), (block->m_free ? "true" : "false"), block->m_size);
            off += (m_header_size + block->m_size);
        }
    }
}