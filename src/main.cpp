#include <iostream>
#include <cstdio>

#include <allocators/pool_allocator.hpp>
#include <cpp/lang/string.hpp>
#include <allocators/freelist_allocator.hpp>
#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/utils/array_list.hpp>

#if 0
int main() {
    tca::free_list_allocator alloc(tca::get_default_allocator());

    
    tc::array_list<void*> list;
    const std::size_t MAX_ALLOCS = 1024;


    const std::size_t ALIGN_ARRAY[] = {8, 16, 32, 64, 128, 256, 512};

    tc::random rnd;
    for (int i = 0; i < 1000; ++i)
    {
        void* p = alloc.allocate_align(rnd.next<std::size_t>(128), ALIGN_ARRAY[rnd.next<std::size_t>(sizeof(ALIGN_ARRAY) / sizeof(*ALIGN_ARRAY))]);
        assert(p != nullptr);

        if (list.size() > MAX_ALLOCS)
        {
            std::size_t idx = rnd.next<std::size_t>(list.size());
            alloc.deallocate(list.at(idx));
            list.fast_remove_at(idx);
        }
        
        list.add(p);
    }

    for (std::size_t i = 0; i < list.size(); ++i)
    {
        alloc.deallocate(list.at(i));
    }

    alloc.print_log();

    // alloc.print_log();
    
    // std::printf("\n\n");

    // alloc.deallocate(p);

    
}

#endif
int main() {
    tca::free_list_allocator alloc(tca::get_default_allocator());
    tc::string sb (&alloc);
    for (std::size_t i = 0; i < 1000; ++i)
    {
        sb.append("___iteration___ ").append(tc::to_string(i)).append("\n");
        sb.clear();
    }

    alloc.print_log();
}