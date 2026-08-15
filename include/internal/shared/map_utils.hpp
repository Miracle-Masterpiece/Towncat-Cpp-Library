#ifndef DEEA2B2C_5A02_4FCC_B680_462D65028132
#define DEEA2B2C_5A02_4FCC_B680_462D65028132

#include <cpp/lang/array.hpp>
#include <cassert>

namespace tc
{
namespace internal
{
namespace map
{

    /**
    * Allocates memory and initializes a new entry.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any copy or move constructor exception.
    */
    template<typename ENTRY, typename KEY_, typename VALUE_>
    ENTRY* alloc_entry(KEY_&& key, VALUE_&& value, std::size_t hashcode, tca::allocator* alloc) {
        void* mem = alloc->allocate_align(sizeof(ENTRY), alignof(ENTRY));
        if (!mem)
            throw_except<out_of_memory_error>("Out of memory!");
        
        ENTRY* e = nullptr;
        try {
            e = new(mem) ENTRY(std::forward<KEY_>(key), std::forward<VALUE_>(value), hashcode);
        } catch (...) {
            alloc->deallocate(mem);
            throw;
        }
        
        return e;
    }

    /**
    * Calls the destructor and frees the memory of the passed entry.
    * 
    * @param e
    *       Pointer to a map entry
    * 
    * @param alloc
    *       A pointer to the allocator that allocated the entry
    */
    template<typename ENTRY>
    void free_entry(ENTRY* e, tca::allocator* alloc) {
        assert(e != nullptr);
        e->~ENTRY();
        alloc->deallocate(e);
    }
    
    template<typename ENTRY_T>
    void append_entry(std::size_t idx, ENTRY_T* e, tc::array<ENTRY_T*>& buckets) {
        assert(buckets.length > 0);
        assert(idx < buckets.length);
        if (buckets[idx])
        {
            e->set_next(buckets[idx]);
            buckets[idx] = e;
        }
        else
        {
            buckets[idx] = e;
        }
    }

    template<typename THASHER, typename KEY, typename ENTRY_T>
    std::size_t bucket_index(const KEY& k, const array<ENTRY_T*>& buckets) {
        assert(buckets.length != 0);
        return THASHER()(k) % buckets.length;
    }

    template<typename THASHER, typename TEQUALER, typename ENTRY_T, typename TKEY>
    ENTRY_T* find_entry(const TKEY& k, const array<ENTRY_T*>& buckets) {
        if (buckets.length == 0)
            return nullptr;
        
        std::size_t idx  = bucket_index<THASHER>(k, buckets);

        assert(buckets.length > 0);
        TEQUALER equals_to;
        for (ENTRY_T* e = buckets[idx]; e != nullptr; e = e->get_next())
            if (equals_to(k, e->get_key()))
                return e;
        
        return nullptr;
    }

    template<typename THASHER, typename TKEY>
    std::size_t hash_key(const TKEY& k) {
        return THASHER()(k);
    }

    template<typename THASHER, typename TEQUALER, typename TKEY, typename TVALUE, typename ENTRY>
    ENTRY* insert(TKEY&& key, TVALUE&& value, array<ENTRY*>& buckets, tca::allocator* alloc) {
    
        ENTRY* finded   = find_entry<THASHER, TEQUALER>(key, buckets);
        if (!finded)
        {
            std::size_t hashcode = hash_key<THASHER>(key);
            ENTRY* e = alloc_entry<ENTRY>(
                                                std::forward<TKEY>(key),
                                                std::forward<TVALUE>(value),
                                                hashcode,
                                                alloc
                                            );
            
            std::size_t idx = internal::map::bucket_index<THASHER>(key, buckets);
            internal::map::append_entry(idx, e, buckets);
            
            return e;
        }
        
        return nullptr;
    }

    template<typename ENTRY>
    void clear(array<ENTRY*>& buckets, tca::allocator* alloc) {
        for (std::size_t i = 0; i < buckets.length; ++i) {
            ENTRY* e = buckets[i];
            while (e)
            {
                ENTRY* current = e;
                e = e->get_next();
                free_entry(current, alloc);
            }
        }
        buckets.set(nullptr);
    }

} //namespace tc
} //namespace internal
} //namespace tc


#endif /* DEEA2B2C_5A02_4FCC_B680_462D65028132 */
