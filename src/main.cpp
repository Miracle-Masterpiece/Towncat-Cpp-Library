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
 #define LINKED_HASH_MAP_TEST
 #define STRING_TEST
 #define BYTEBUF_TEST

void assert_fail(bool expr, const char* msg, const char* func, int line) {
    if (!expr)
    {
        std::printf("%s %s:%d\n", msg, func, line);
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
#include <allocators/malloc_free_allocator.hpp>
namespace hash_map_test
{
    void copy_test() {
        tc::hash_map<int, int> map     = {{1, 1}, {2, 2}};
        tc::hash_map<int, int> copied  = map;

        TC_ASSERT(copied.size() == map.size());
        TC_ASSERT(copied.contains_key(1));
        TC_ASSERT(copied.contains_key(2));
        TC_ASSERT(copied.get_allocator() == map.get_allocator());
    }
    
    void move_test() {
        tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
        tc::hash_map<int, int> moved = std::move(map);
        
        TC_ASSERT(moved.get_allocator() == map.get_allocator());
        TC_ASSERT(moved.size() == 2);
        TC_ASSERT(moved.contains_key(1));
        TC_ASSERT(moved.contains_key(2));
    }

    void assign_copy_test() {
        tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
        
        tca::malloc_free_allocator alloc;
        tc::hash_map<int, int> copied(&alloc);
        
        copied = map;
        
        TC_ASSERT(copied.size() == map.size());
        TC_ASSERT(copied.get_allocator() == &alloc);
        TC_ASSERT(map.contains_key(1));
        TC_ASSERT(map.contains_key(2));
    }

    void move_assign_test() {
        tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
        
        tca::malloc_free_allocator alloc;
        tc::hash_map<int, int> moved(&alloc);
        
        moved = std::move(map);
        
        TC_ASSERT(moved.get_allocator() == &alloc);
        TC_ASSERT(map.contains_key(1));
        TC_ASSERT(map.contains_key(2));
    }
}
#endif

#ifdef LINKED_HASH_MAP_TEST
#include <cpp/lang/utils/linked_hash_map.hpp>
#include <allocators/malloc_free_allocator.hpp>
namespace linked_hash_map_test
{
    void copy_test() {
        tc::linked_hash_map<int, int> map     = {{1, 1}, {2, 2}};
        tc::linked_hash_map<int, int> copied  = map;

        TC_ASSERT(copied.size() == map.size());
        TC_ASSERT(copied.contains_key(1));
        TC_ASSERT(copied.contains_key(2));
        TC_ASSERT(copied.get_allocator() == map.get_allocator());
    }
    
    void move_test() {
        tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
        tc::linked_hash_map<int, int> moved = std::move(map);
        
        TC_ASSERT(moved.get_allocator() == map.get_allocator());
        TC_ASSERT(moved.size() == 2);
        TC_ASSERT(moved.contains_key(1));
        TC_ASSERT(moved.contains_key(2));
    }

    void assign_copy_test() {
        tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
        
        tca::malloc_free_allocator alloc;
        tc::linked_hash_map<int, int> copied(&alloc);
        
        copied = map;
        
        TC_ASSERT(copied.size() == map.size());
        TC_ASSERT(copied.get_allocator() == &alloc);
        TC_ASSERT(map.contains_key(1));
        TC_ASSERT(map.contains_key(2));
    }

    void move_assign_test() {
        tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
        
        tca::malloc_free_allocator alloc;
        tc::linked_hash_map<int, int> moved(&alloc);
        
        moved = std::move(map);
        
        TC_ASSERT(moved.get_allocator() == &alloc);
        TC_ASSERT(map.contains_key(1));
        TC_ASSERT(map.contains_key(2));
    }
}
#endif

#ifdef STRING_TEST
#include <cpp/lang/string.hpp>
#include <allocators/malloc_free_allocator.hpp>
namespace string_test
{
    void copy_test() {
        tc::string str = "Hello, World!";
        tc::string s = str;
        TC_ASSERT(s.get_allocator() == str.get_allocator());
    }
    
    void move_test() {
        tc::string str = "Hello, World!";
        tc::string s = std::move(str);
        TC_ASSERT(s.get_allocator() == str.get_allocator());
    }

    void assign_copy_test() {
        tc::string str  = "Hello, World!";
        
        tca::malloc_free_allocator alloc;
        tc::string s("sdadad", &alloc);
        
        s = str;

        TC_ASSERT(s.get_allocator() == &alloc);
    }

    void move_assign_test() {
        tc::string str  = "Hello, World!";
        
        tca::malloc_free_allocator alloc;
        tc::string s("sdadad", &alloc);
        
        s = std::move(str);

        TC_ASSERT(s.get_allocator() == &alloc);

        // std::cout << s << std::endl;
    }
}
#endif

#ifdef BYTEBUF_TEST
#include <cpp/lang/io/bytebuf.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <cpp/lang/net/inetaddr.hpp>
#include <iostream>

namespace bytebuf_test
{
    void copy_test() {
        tc::bytebuf buf0(16);
        tc::bytebuf buf1 = buf0;
        TC_ASSERT(buf1.get_allocator() == buf0.get_allocator());
        TC_ASSERT(buf1.remaining()     == buf0.remaining());
        TC_ASSERT(buf1.capacity()      == buf0.capacity());
        TC_ASSERT(buf1.limit()         == buf0.limit());
    }
    
    void move_test() {
        tc::bytebuf buf0(16);
        tc::bytebuf buf1 = std::move(buf0);
        TC_ASSERT(buf1.get_allocator() == buf0.get_allocator());
    }

    void assign_copy_test() {
        tc::bytebuf buf0(16);
        
        tca::malloc_free_allocator alloc;
        tc::bytebuf buf1(4, &alloc);
        
        buf1 = buf0;

        TC_ASSERT(buf1.get_allocator() == &alloc);
        TC_ASSERT(buf1.remaining()     == buf0.remaining());
        TC_ASSERT(buf1.capacity()      == buf0.capacity());
        TC_ASSERT(buf1.limit()         == buf0.limit());
    }

    void move_assign_test() {
        tc::bytebuf buf0(16);
        
        tca::malloc_free_allocator alloc;
        tc::bytebuf buf1(4, &alloc);
        
        buf1 = std::move(buf0);

        TC_ASSERT(buf1.get_allocator() == &alloc);
    }
}
#endif

template class tc::array<int>;
template class tc::array_list<int>;
template class tc::linked_list<int>;
template class tc::hash_map<int, int>;
template class tc::hash_map<int, const tc::string>;
template class tc::hash_map<int, tc::string>;
template class tc::linked_hash_map<int, const tc::string>;
template class tc::linked_hash_map<int, tc::string>;


#if 0
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

    #ifdef LINKED_HASH_MAP_TEST
    {
        using namespace hash_map_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif
    
    #ifdef HASH_MAP_TEST
    {
        using namespace linked_hash_map_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif
    
    #ifdef STRING_TEST
    {
        using namespace string_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif
    
    #ifdef BYTEBUF_TEST
    {
        using namespace bytebuf_test;
        copy_test();
        move_test();
        assign_copy_test();
        move_assign_test();
    }
    #endif
}
#endif

#if 0

#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/utils/images/image_packer.hpp>
#include <cpp/lang/utils/arrays.hpp>

class png_filter : public tc::file_filter {
public:
    bool apply(const char* name, std::size_t len) const {
        return tc::string(name).ends_with(".png");
    }
};

template<>
struct tc::compare_to<tc::image> {
    int operator()(const tc::image& a, const tc::image& b) {
        int a_area = a.get_width() * a.get_height();
        int b_area = b.get_width() * b.get_height();
        return b_area - a_area;
    }
};

tc::array_list<tc::image> load_all_image(const char* images) {
    
    tc::file dir = images;
    tc::array<tc::file> files = dir.list_files(png_filter());
    tc::array_list<tc::image> list(files.length);

    for (std::size_t i = 0; i < files.length; ++i)
    {
        list.add(
            tc::imageio::load_image(files[i])
        );
    }

    tc::quick_sort(list.data(), list.size());

    return list;
}

int main() {
    const char* image_folder = "images";
    tca::free_list_allocator free_list(tca::get_default_allocator(), 1 << 24);
    tca::set_default_allocator(&free_list);
    try {
        tc::array_list<tc::image> images = load_all_image(image_folder);
        tc::image_packer packer(images.data(), images.size(), 1024, 1024);
        tc::image atlas = packer.pack(4);
        tc::imageio::write_image(tc::file("build/atlas.png"), atlas, "png");
    } catch (const tc::throwable& t) {
        std::cout << t.cause() << "\n";
        t.print_stack_trace();
    }
    free_list.print_log();
}
#endif

#include <tc/unique_ptr.hpp>
#include <tc/shared_ptr.hpp>

struct animal {
    double x, y, z;
    float xo, yo, zo;
    bool on_ground;

    virtual void say() const = 0;
    virtual ~animal() {
        std::cout << "~animal()\n";
    }
};

struct cat : animal {
    void say() const {
        std::cout << "meow\n";
    }
    ~cat() {
        std::cout << "~cat()\n";
    }
};

#include <cpp/lang/math.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>
#include <cpp/lang/traits/pure_traits.hpp>
#include <tc/unique_ptr.hpp>

#include <memory>

template class tc::shared_ptr<cat>;

struct test {
    test() { std::cout << "test()\n"; }
    test(const test&) { std::cout << "test(const test&)\n"; }
    test(test&&) { std::cout << "test(test&&)\n"; }
    test& operator= (const test&) { std::cout << "test& operator= (const test&)\n"; return *this;}
    test& operator= (test&&) { std::cout << "test& operator= (test&&)\n"; return *this;}
    ~test() {std::cout << "~test()\n";}
};

#include <tc/date.hpp>









template<typename T>
struct PTR_DELETER {
    void operator()(T* p)
    {
        delete p;
    }
};

struct point {
    int x, y;
};

template class tc::unique_ptr<tc::string>;




#if 0
int main() {
    using namespace tc::internal;
    using namespace tc;
    tca::free_list_allocator alloc(tca::get_default_allocator());

    // {
    //     tc::array_list<tc::polymorph::unique_ptr<animal>> animals(&alloc);
    
    //     for (std::size_t i = 0; i < 1000; ++i)
    //     {
    //         animals.add(
    //             tc::polymorph::allocate_unique<cat>(&alloc)
    //         );
    //     }
        
    //     for (std::size_t i = 0; i < 10; ++i)
    //     {
    //         animals.at(i)->say();
    //     }
    
    // }
    alloc.print_log();
}
#endif

#include <tc/io/ifstream.hpp>
#include <tc/io/rafstream.hpp>

int main() {
    try {
        
        
        tc::random_access_file file("build/text.txt", "r");

        {
            tc::string s = "Hello, World";
            file.write_string(s);

            char c;
            std::cout << "before force\n";
            std::cin >> c;
            file.force(false);
            std::cout << "after force\n";
            std::cin >> c;
        }
        
        file.seek(0);

        {
            tc::string s = file.read_string();
            // std::cout << s << std::endl;
        }

    } catch (const tc::throwable& e) {
        std::cout << e.cause() << "\n";
    }
}