#include <iostream>
#include <cstdio>
#include <allocators/pool_allocator.hpp>
#include <cpp/lang/string.hpp>
#include <allocators/freelist_allocator.hpp>
#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <internal/img_utils.hpp>

#define TC_TESTS
#ifdef TC_TESTS
 #define ARRAY_TEST
 #define ARRAY_LIST_TEST
 #define LINKED_LIST_TEST
 #define HASH_MAP_TEST

void assert_fail(bool expr, const char* msg, const char* func, int line) {
    if (!expr)
    {
        std::printf("%s\n", msg);
    }
}

#define TC_ASSERT(expr) assert_fail(expr, #expr, __func__, __LINE__)

#endif

#include <allocators/malloc_free_allocator.hpp>
#ifdef ARRAY_TEST
#include <cpp/lang/array.hpp>
namespace array_test
{
    void copy_test() {
        tc::array<int> ints     = {1, 2, 3, 4, 5};
        tc::array<int> copied   = ints;
        
        TC_ASSERT(copied.length == ints.length);
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == ints.get_allocator());
    }
    
    void move_test() {
        tc::array<int> ints    = {1, 2, 3, 4, 5};
        tc::array<int> moved   = std::move(ints);
        
        TC_ASSERT(moved.length == 5);
        TC_ASSERT(moved.get_allocator() == ints.get_allocator());
    }

    void assign_copy_test() {
        tc::array<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::array<int> copied(&alloc);
        
        copied = ints;

        TC_ASSERT(copied.length == ints.length);
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == &alloc);
    }

    void move_assign_test() {
        tc::array<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::array<int> moved(&alloc);

        moved = std::move(ints);

        TC_ASSERT(moved.length == 5);
        TC_ASSERT(moved.get_allocator() == &alloc);
    }
}
#endif

#ifdef ARRAY_LIST_TEST
#include <cpp/lang/utils/array_list.hpp>
namespace array_list_test
{
    void copy_test() {
        tc::array_list<int> ints     = {1, 2, 3, 4, 5};
        tc::array_list<int> copied   = ints;
        
        TC_ASSERT(copied.size() == ints.size());
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == ints.get_allocator());
    }
    
    void move_test() {
        tc::array_list<int> ints    = {1, 2, 3, 4, 5};
        tc::array_list<int> moved   = std::move(ints);
        
        TC_ASSERT(moved.size() == 5);
        TC_ASSERT(moved.get_allocator() == ints.get_allocator());
    }

    void assign_copy_test() {
        tc::array_list<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::array_list<int> copied(&alloc);
        
        copied = ints;

        TC_ASSERT(copied.size() == ints.size());
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == &alloc);
    }

    void move_assign_test() {
        tc::array_list<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::array_list<int> moved(&alloc);

        moved = std::move(ints);

        TC_ASSERT(moved.size() == 5);
        TC_ASSERT(moved.get_allocator() == &alloc);
    }
}
#endif

#ifdef LINKED_LIST_TEST
#include <cpp/lang/utils/linked_list.hpp>
namespace linked_list_test
{
    void copy_test() {
        tc::linked_list<int> ints     = {1, 2, 3, 4, 5};
        tc::linked_list<int> copied   = ints;
        
        TC_ASSERT(copied.size() == ints.size());
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == ints.get_allocator());
    }
    
    void move_test() {
        tc::linked_list<int> ints    = {1, 2, 3, 4, 5};
        tc::linked_list<int> moved   = std::move(ints);
        
        TC_ASSERT(moved.size() == 5);
        TC_ASSERT(moved.get_allocator() == ints.get_allocator());
    }

    void assign_copy_test() {
        tc::linked_list<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::linked_list<int> copied(&alloc);
        
        copied = ints;

        TC_ASSERT(copied.size() == ints.size());
        TC_ASSERT(copied.equals(ints));
        TC_ASSERT(copied.get_allocator() == &alloc);
    }

    void move_assign_test() {
        tc::linked_list<int> ints     = {1, 2, 3, 4, 5};
        
        tca::malloc_free_allocator alloc;
        tc::linked_list<int> moved(&alloc);

        moved = std::move(ints);

        TC_ASSERT(moved.size() == 5);
        TC_ASSERT(moved.get_allocator() == &alloc);
    }
}
#endif

#ifdef HASH_MAP_TEST
#include <cpp/lang/utils/hash_map.hpp>
#endif

template class tc::array<int>;
template class tc::array_list<int>;
template class tc::linked_list<int>;
template class tc::hash_map<int, int>;

#include <unordered_map>

int main() {
    
    #ifdef ARRAY_TEST
    {
        using namespace array_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif

    #ifdef ARRAY_LIST_TEST
    {
        using namespace array_list_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif

    #ifdef LINKED_LIST_TEST
    {
        using namespace linked_list_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif
}