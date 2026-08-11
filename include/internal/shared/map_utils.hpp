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

} //namespace tc
} //namespace internal
} //namespace tc


#endif /* DEEA2B2C_5A02_4FCC_B680_462D65028132 */
