#include <iostream>
#include <cstdio>
#include <allocators/pool_allocator.hpp>
#include <cpp/lang/string.hpp>
#include <allocators/freelist_allocator.hpp>
#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <internal/img_utils.hpp>

#if 0
int main() {
    tca::free_list_allocator alloc(tca::get_default_allocator());

    
    tc::array_list<void*> list;
    const std::size_t MAX_ALLOCS = 1000;

    const std::size_t ALIGN_ARRAY[] = {8, 16, 32, 64, 128, 256, 512};

    tc::random rnd;
    for (int i = 0; i < 10000; ++i)
    {
        std::size_t sz = rnd.next<std::size_t>(128);
        void* p = alloc.allocate_align(sz, ALIGN_ARRAY[rnd.next<std::size_t>(sizeof(ALIGN_ARRAY) / sizeof(*ALIGN_ARRAY))]);
        assert(p != nullptr);

        std::memset(p, 0, sz);

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
    std::printf("\n\n");
    alloc.compact();
    alloc.print_log();

    // alloc.print_log();
    

    // alloc.deallocate(p);

    
}

#endif

#if 0
int main() {
    tca::free_list_allocator alloc(tca::get_default_allocator());

    void* p = alloc.allocate_align(sizeof(int), alignof(int));
    alloc.print_log();
    alloc.deallocate(p);
    alloc.print_log();
    
}
#endif

#if 0
int main() {
    tca::free_list_allocator alloc(tca::get_default_allocator());

    std::size_t CNT = 1014 * 1024 * 3;

    void* p = alloc.allocate_align(1024 * 1024, alignof(short));
    std::memset(p, 0, CNT);

    alloc.deallocate(p);
    alloc.print_log();
}
#endif


#if 0
#include <cpp/lang/string.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/utils/images/imageio.hpp>
int main() {
    tca::free_list_allocator allocator(tca::get_default_allocator());
    tca::set_default_allocator(&allocator);

    using tc::internal::resize_image_alpha;
    using tc::internal::resize_image;
    try {
        const char* __img = "build/bloodynight_ft.png";
        // const char* __img = "build/stone.png";
        
        tc::image img = tc::imageio::load_image(__img);
        std::cout << img.to_string() << std::endl;
        {      
            tc::image resized(img.get_width() / 4, img.get_height() / 4, img.get_channels());                
            resize_image(
            img.pixels(), img.get_width(), img.get_height(), 
            resized.pixels(), resized.get_width(), resized.get_height(), resized.get_channels()
            );
            tc::imageio::write_image(tc::file("build/my_impl.png"), &resized, "png");
        }
        {
            tc::image resized = img.resize(img.get_width() / 4, img.get_height() / 4);
            tc::imageio::write_image(tc::file("build/stb.png"), &resized, "png");
        }
        

    } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
    }

    allocator.print_log();
    allocator.compact();
    allocator.print_log();
}
#endif












































#if 1
#include <cpp/lang/array.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <cpp/lang/utils/linked_list.hpp>
#include <cpp/lang/utils/hash_map.hpp>
#include <cpp/lang/utils/linked_hash_map.hpp>


#include <allocators/malloc_free_allocator.hpp>
#include <cstdio>

namespace tc_test
{
    bool fail(bool expr, const char* msg, const char* func, int line) {
        if (!expr)
        {
            std::printf("fail: %s at %s:%d\n", msg, func, line);
            return false;
        }
        return true;
    }
};

#define assert_(expr) tc_test::fail(expr, #expr, __func__, __LINE__)

struct test {
    int v;
    static int count;
    test(int x) : v(x) {
        // if (v == 7)
        // {
        //     tc::throw_except<tc::invalid_data_format_exception>("err: %d", v);
        // }
    }
    test() : v(0) {
        // if (v == 7)
        // {
        //     tc::throw_except<tc::invalid_data_format_exception>("err: %d", v);
        // }
    }
    test(const test& t) : v(t.v) {
        // if (v == 7)
        // {
        //     tc::throw_except<tc::invalid_data_format_exception>("err: %d", v);
        // }
    }
    
    bool operator==(const test& t) const {
        return v == t.v;
    }

    test& operator=(const test& t) {
        v = t.v;
        return *this;
    }
    
    test& operator=(test&& t) {
        v = t.v;
        return *this;
    }
    
    ~test() {

    }
};

int test::count = 0;

namespace array_test
{
    void copy_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array<test> arr1(45, &a1);
            tc::array<test> arr0 = arr1;
            
            assert_(arr0.get_allocator() == &a1);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void move_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array<test> arr1(45, &a1);
            tc::array<test> arr0 = std::move(arr1);
            
            assert_(arr0.get_allocator() == &a1);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }

    void copy_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array<test> arr0(10, &a0);
            tc::array<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            
            arr0 = arr1;
    
            assert_(arr0.get_allocator() == &a0);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
    
    void move_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            
            tc::array<test> arr0(10, &a0);
            tc::array<test> arr1;
            
            arr1 = std::move(arr0);
    
            assert_(arr1.get_allocator() == &a0);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
};

namespace array_list_test
{
    void copy_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            tc::array_list<test> arr0 = arr1;
            
            assert_(arr0.get_allocator() == &a1);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void move_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            tc::array_list<test> arr0 = std::move(arr1);
            
            assert_(arr0.get_allocator() == &a1);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }

    void copy_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::array_list<test> arr0(10, &a0);
            tc::array_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            
            arr0 = arr1;
    
            assert_(arr0.get_allocator() == &a0);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
    
    void move_assign_test() {
        try {
            tca::malloc_free_allocator a0;
            
            tc::array_list<test> arr0({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a0);
            tc::array_list<test> arr1;
            
            arr1 = std::move(arr0);
    
            assert_(arr1.get_allocator() == &a0);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
};

namespace linked_list_test
{
    void copy_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            tc::linked_list<test> arr0 = arr1;
            
            assert_(arr0.get_allocator() == &a1);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void move_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            tc::linked_list<test> arr0 = std::move(arr1);
            
            assert_(arr0.get_allocator() == &a1);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }

    void copy_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_list<test> arr0(10, &a0);
            tc::linked_list<test> arr1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a1);
            
            arr0 = arr1;
    
            assert_(arr0.get_allocator() == &a0);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
    
    void move_assign_test() {
        try {
            tca::malloc_free_allocator a0;
            
            tc::array_list<test> arr0({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, &a0);
            tc::array_list<test> arr1;
            
            arr1 = std::move(arr0);
    
            assert_(arr1.get_allocator() == &a0);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
};


template class tc::hash_map<tc::string, tc::string>;
template class tc::linked_hash_map<tc::string, tc::string>;

namespace hash_map_test
{
    void copy_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, &a1);
            tc::hash_map<test, test> arr0 = arr1;
            
            assert_(arr0.get_allocator() == &a1);
            assert_(arr1.get_allocator() == &a1);
            
            assert_(arr0.equals(arr1));
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void move_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, &a1);
            tc::hash_map<test, test> arr0 = std::move(arr1);
            
            assert_(arr0.get_allocator() == &a1);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }

    void copy_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::hash_map<test, test> arr0(&a0);
            tc::hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, &a1);
            
            arr0 = arr1;
    
            assert_(arr0.get_allocator() == &a0);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
    
    void move_assign_test() {
        try {
            tca::malloc_free_allocator a0;
            
            tc::hash_map<test, test> arr0({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, &a0);
            tc::hash_map<test, test> arr1;
            
            arr1 = std::move(arr0);
    
            assert_(arr1.get_allocator() == &a0);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
};

namespace linked_hash_map_test
{
    void copy_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, true, &a1);
            tc::linked_hash_map<test, test> arr0 = arr1;
            
            assert_(arr0.get_allocator() == &a1);
            assert_(arr1.get_allocator() == &a1);
            
            assert_(arr0.equals(arr1));
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void move_construct_test() {
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, true, &a1);
            tc::linked_hash_map<test, test> arr0 = std::move(arr1);
            
            assert_(arr0.get_allocator() == &a1);
        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }

    void copy_assign_test() {
        
        try {
            tca::malloc_free_allocator a0;
            tca::malloc_free_allocator a1;
            
            tc::linked_hash_map<test, test> arr0(&a0);
            tc::linked_hash_map<test, test> arr1({{test(1), test(2)}, {test(5), test(8)}}, 0.75f, true, &a1);
            
            arr0 = arr1;
    
            assert_(arr0.get_allocator() == &a0);
            assert_(arr1.get_allocator() == &a1);
            assert_(arr0.equals(arr1));

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
    
    void move_assign_test() {
        try {
            tca::malloc_free_allocator a0;
            
            tc::linked_hash_map<test, test> arr0({{test(1057), test(8)}, {test(5), test(7657)}}, 0.75f, true, &a0);
            tc::linked_hash_map<test, test> arr1;
            
            arr1 = std::move(arr0);
    
            assert_(arr1.get_allocator() == &a0);

            std::cout << arr1.hashcode() << std::endl;

            for (const tc::map::entry<test, test>& e : arr1)
            {
                std::cout << e.get_key().v << " == " << e.get_value().v << std::endl;
            }

        } catch (const tc::throwable& e) {
            std::cout << e.cause() << std::endl;
        }
        
    }
};

template<>
struct tc::equal_to<test> {
    bool operator () (const test& a, const test& b) const {
        // std::cout << a.v << " == " << b.v << "\n";
        return a.v == b.v;
    }
};

template<>
struct tc::hash_for<test> {
    std::size_t operator () (const test& a) const {
        return static_cast<std::size_t>(a.v);
    }
};

int main(int argc, char const *argv[]) {
    // while(1)
    
    // tc::map::entry<int, tc::string> entry(45, tc::string("hello"), 453543);
    // std::cout << tc::hash_for<tc::map::entry<int, tc::string>>()(entry) << std::endl;

    {

       {
            using namespace array_test;
            copy_construct_test();
            move_construct_test();
            copy_assign_test();
            move_assign_test();
        }
        
        {
            using namespace array_list_test;
            copy_construct_test();
            move_construct_test();
            copy_assign_test();
            move_assign_test();
        }
        
        {
            using namespace linked_list_test;
            copy_construct_test();
            move_construct_test();
            copy_assign_test();
            move_assign_test();
        }
        
        {
            using namespace hash_map_test;
            copy_construct_test();
            move_construct_test();
            copy_assign_test();
            move_assign_test();
        }
        
        {
            using namespace linked_hash_map_test;
            copy_construct_test();
            move_construct_test();
            copy_assign_test();
            move_assign_test();
        }

        // {
        //     tc::hash_map<int, int> map = {{1, 2}, {1, 2}, {3, 4}};
        //     std::cout << map.hashcode() << std::endl;
        // }
        // {
        //     tc::hash_map<int, int> map = {{8, 2}, {3, 5}, {7, 4}};
        //     std::cout << map.hashcode() << std::endl;
        // }

    }
}

#endif
