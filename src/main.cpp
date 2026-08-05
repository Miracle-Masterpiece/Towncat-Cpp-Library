#include <iostream>
#include <cstdio>

#include <allocators/pool_allocator.hpp>
#include <cpp/lang/string.hpp>

int main() {
    
    tca::pool_allocator __pool(sizeof(tc::string), alignof(std::max_align_t));
    tca::pool_allocator pool = std::move(__pool);

    for (std::size_t i = 0; i < 10400; ++i)
    {
        void* p = pool.allocate();
        tc::string* str = new(p) tc::string();
        std::cout << str->append(tc::to_string(i)) << std::endl;
        str->~tstring();
    }
}