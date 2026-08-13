#ifndef C151DFEA_DFA0_44BB_9C18_756422D43DDA
#define C151DFEA_DFA0_44BB_9C18_756422D43DDA

#include <cpp/lang/traits/SFINAE.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cstddef>
#include <utility>

namespace tc
{
namespace map
{

template<typename TKEY, typename TVAL>
class entry {
    /**
     * 
     */
    entry* m_next;

    /**
     * 
     */
    std::size_t m_hash;

    /**
     * 
     */
    TKEY m_key;

    /**
     * 
     */
    TVAL m_value;

public:
    /**
     * 
     */
    template<typename TKEY_, typename TVAL_>
    entry(TKEY_&&, TVAL_&&, std::size_t hashcode);

    /**
     * 
     */
    entry* get_next();

    /**
     * 
     */
    const entry* get_next() const;

    /**
     * 
     */
    void set_next(entry* e);

    /**
     * 
     */
    const TKEY& get_key() const;

    /**
     * 
     */
    TVAL& get_value();

    /**
     * 
     */
    const TVAL& get_value() const;

    /**
     * 
     */
    template<typename TVAL_>
    void set_value(TVAL_&&);

    /**
     * 
     */
    std::size_t get_hash() const;
};
} //namespace map

namespace map
{

    template<typename TKEY, typename TVAL>
    template<typename TKEY_, typename TVAL_>
    entry<TKEY, TVAL>::entry(TKEY_&& key, TVAL_&& value, std::size_t hashcode) :
        m_next(nullptr),
        m_hash(hashcode),
        m_key(std::forward<TKEY_>(key)),
        m_value(std::forward<TVAL_>(value)) {

    }
    
    template<typename TKEY, typename TVAL>
    entry<TKEY, TVAL>* entry<TKEY, TVAL>::entry::get_next() {
        return m_next;
    }
    
    template<typename TKEY, typename TVAL>
    const entry<TKEY, TVAL>* entry<TKEY, TVAL>::get_next() const {
        return m_next;
    }
        
    template<typename TKEY, typename TVAL>
    void entry<TKEY, TVAL>::set_next(entry<TKEY, TVAL>* e) {
        m_next = e;
    }
    
    template<typename TKEY, typename TVAL>
    TVAL& entry<TKEY, TVAL>::get_value() {
        return m_value;
    }

    template<typename TKEY, typename TVAL>
    const TKEY& entry<TKEY, TVAL>::get_key() const {
        return m_key;
    }
    
    template<typename TKEY, typename TVAL>
    const TVAL& entry<TKEY, TVAL>::get_value() const {
        return m_value;
    }

    template<typename TKEY, typename TVAL>
    template<typename TVAL_>
    void entry<TKEY, TVAL>::set_value(TVAL_&& value) {
        m_value = std::forward<TVAL_>(value);
    }

    template<typename TKEY, typename TVAL>
    std::size_t entry<TKEY, TVAL>::get_hash() const {
        return m_hash;
    }

} //namespace map

template<typename K, typename V>
struct hash_for<map::entry<K, V>> {
    std::size_t operator() (const map::entry<K, V>& e) const {
        hash_for<K> khash;
        hash_for<V> vhash;
        return (khash(e.get_key()) * 17) ^ (vhash(e.get_value()) >> 4);
    }
};

template<typename K, typename V>
struct equal_to<map::entry<K, V>> {
    std::size_t operator() (const map::entry<K, V>& a, const map::entry<K, V>& b) const {
        equal_to<K> kequal;
        equal_to<V> vequal;
        return kequal(a.get_key(), b.get_key()) && vequal(a.get_value(), b.get_value());
    }
};

} //namespace tc

#endif /* C151DFEA_DFA0_44BB_9C18_756422D43DDA */
