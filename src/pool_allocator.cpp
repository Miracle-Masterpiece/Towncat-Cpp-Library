#include <allocators/pool_allocator.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>
#include <cstdint>
#include <utility>

namespace tca
{
    using pa = pool_allocator;

    pa:: pool_allocator() : allocator(),
    pages(nullptr),
    free_list(nullptr),
    align(0),
    bucket_size(0),
    cnt_buckets_per_page(0) {

    }

    pa::pool_allocator(std::size_t bucket_size, std::size_t align, allocator* alloc) :  allocator(alloc),
    pages(nullptr),
    free_list(nullptr),
    align( tc::math::max(align, alignof(internal::bucket)) ),
    bucket_size( align_up(bucket_size, this->align) ),
    cnt_buckets_per_page(64) {

    }

    pa:: pool_allocator(pool_allocator&& alloc) : allocator(std::move(alloc)) {
        std::swap(pages,        alloc.pages);
        std::swap(free_list,    alloc.free_list);
        std::swap(align,        alloc.align);
        std::swap(bucket_size,  alloc.bucket_size);
        std::swap(cnt_buckets_per_page, alloc.cnt_buckets_per_page);
    }

    pool_allocator& pa:: operator=(pool_allocator&& alloc) {
        if (&alloc != this)
        {
            allocator::operator=(std::move(alloc));
            std::swap(pages,        alloc.pages);
            std::swap(free_list,    alloc.free_list);
            std::swap(align,        alloc.align);
            std::swap(bucket_size,  alloc.bucket_size);
            std::swap(cnt_buckets_per_page, alloc.cnt_buckets_per_page);
        }
        return *this;
    }
    
    void pa:: allocate_page() {
        std::size_t header_sz = align_up(sizeof(internal::page_header), align);
        std::size_t block_sz  = bucket_size * cnt_buckets_per_page;
        std::size_t total_page_size = header_sz + block_sz;
        
        #if 0
        unsigned char* tmp = nullptr;
        std::printf("header     : 0x%p\n", tmp);
        std::printf("block      : 0x%p\n", tmp + header_sz);
        std::printf("header_sz  : %zu\n", header_sz);
        std::printf("block_sz   : %zu\n", block_sz);
        std::printf("total      : %zu\n", total_page_size);
        std::printf("bucket_sz  : %zu\n", bucket_size);
        std::printf("align      : %zu\n", align);
        #endif

        void* dat = m_parent->allocate_align(
                        total_page_size, tc::math::max(
                            align, alignof(internal::page_header) 
                        )
                    );
        
        if (!dat)
            return;

        #if 1
        {
            internal::page_header* hder = static_cast<internal::page_header*>(dat);
            
            assert((((std::uintptr_t) &hder->dat) % alignof(void*)) == 0);
            hder->dat    = static_cast<unsigned char*>(dat) + header_sz;
            
            assert((((std::uintptr_t) &hder->next) % alignof(void*)) == 0);
            hder->next = pages;
            
            assert((((std::uintptr_t) &hder->size) % alignof(std::size_t)) == 0);
            hder->size   = total_page_size;

            pages = hder;
        }

        {
            
            unsigned char* buckets = static_cast<unsigned char*>(dat) + header_sz;
            
            for (std::size_t i = 0; i < cnt_buckets_per_page; ++i)
            {
                // What the fuck
                internal::bucket* bucket = static_cast<internal::bucket*>(
                    static_cast<void*>(buckets + i * bucket_size)
                );
                
                assert((((std::uintptr_t) bucket) % align) == 0);
                
                bucket->next = free_list;
                free_list = bucket;
            }

        }
        #endif
    }

    void* pa:: allocate() {
        
        for (std::size_t i = 0; i < 2; ++i)
        {
            if (free_list != nullptr)
            {
                
                internal::bucket* node = free_list;
                free_list = free_list->next;
                
                assert((((std::uintptr_t) node) % align) == 0);
                
                return static_cast<void*>(node);
            }
    
            if (i == 0)
            {
                allocate_page();
            }
        }

        return nullptr;
    }
    
    void* pa:: allocate(std::size_t sz) {
        if (sz > bucket_size || alignof(std::max_align_t) > align)
            return nullptr;
        return allocate();
    }

    void* pa:: allocate_align(std::size_t sz, std::size_t align_) {
        if (sz > bucket_size || align_ > align)
            return nullptr;
        return allocate();
    }

    void pa ::deallocate(void* p) {
        if (p == nullptr)
            return;
        internal::bucket* node = static_cast<internal::bucket*>(p);
        node->next = free_list;
        free_list  = node;
    }

    pa:: ~pool_allocator() {
        for (internal::page_header* page = pages; page != nullptr; )
        {
            internal::page_header* next = page->next;
            m_parent->deallocate(static_cast<void*>(page), page->size);
            page = next;
        }
    }
}