#ifndef D4616906_478F_4D6C_A433_B2CC4DC47873
#define D4616906_478F_4D6C_A433_B2CC4DC47873

#include <allocators/allocator.hpp>
#include <cpp/lang/utils/raw_binary_tree.hpp>
#include <cstddef>
#include <cassert>

namespace tca
{

class arena_free_list_allocator : public allocator {
    /**
     * 
     */
    struct memblock;
    
    /**
     * 
     */
    std::size_t m_alignas;

    /**
     * 
     */
    std::size_t m_header_size;

    /**
     * 
     */
    std::size_t m_min_block_size;

    /**
     * 
     */
    void* m_data;

    /**
     * 
     */
    std::size_t m_length;

    /**
     * 
     */
    void* m_start;

    /**
     * 
     */
    std::size_t m_free_size;

    /**
     * 
     */
    tc::raw_binary_tree<std::size_t, tc::compare_to<std::size_t>, memblock> m_tree;

    /**
     * 
     */
    void link(memblock* block);
    
    /**
     * 
     */
    void unlink(memblock* block);

    /**
     * 
     */
    void split(memblock*, std::size_t piece);

    /**
     * 
     */
    bool can_split(const memblock*, std::size_t piece) const;

    /**
     * 
     */
    void cleanup();

    /**
     * 
     */
    void init(void* data, std::size_t length, std::size_t align);

public:
    /**
     * 
     */
    arena_free_list_allocator();

    /**
     * 
     */
    arena_free_list_allocator(void* data, std::size_t length, std::size_t align = alignof(std::max_align_t));
    
    /**
     * 
     */
    arena_free_list_allocator(std::size_t length, std::size_t align = alignof(std::max_align_t), tca::base_allocator* allocator = tca::get_scoped_or_default());

    /**
     * 
     */
    arena_free_list_allocator(arena_free_list_allocator&&);
    
    /**
     * 
     */
    arena_free_list_allocator& operator= (arena_free_list_allocator&&);

    /**
     * 
     */
    ~arena_free_list_allocator();

    /**
     * 
     */
    void* allocate(std::size_t) override;
    
    /**
     * 
     */
    void* allocate_align(std::size_t, std::size_t) override;
    
    /**
     * 
     */
    void deallocate(void*) override;

    /**
     * 
     */
    void print_log() const;

    /**
     * 
     */
    void merge_all();
};

}// namespace tca

#endif /* D4616906_478F_4D6C_A433_B2CC4DC47873 */
