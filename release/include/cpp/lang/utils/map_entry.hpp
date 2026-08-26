#ifndef C151DFEA_DFA0_44BB_9C18_756422D43DDA
#define C151DFEA_DFA0_44BB_9C18_756422D43DDA

#include <cpp/lang/traits/sfinae.hpp>
#include <cpp/lang/traits/relatoship_traits.hpp>
#include <cpp/lang/utils/pair.hpp>
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
    pair<TKEY, TVAL> m_pair;
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
    pair<TKEY, TVAL>& get_pair();
    
    /**
     * 
     */
    const pair<TKEY, TVAL>& get_pair() const;

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
        m_pair(std::forward<TKEY_>(key), std::forward<TVAL_>(value)) {

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
        return m_pair.second();
    }

    template<typename TKEY, typename TVAL>
    pair<TKEY, TVAL>& entry<TKEY, TVAL>::get_pair() {
        return m_pair;
    }
    
    template<typename TKEY, typename TVAL>
    const pair<TKEY, TVAL>& entry<TKEY, TVAL>::get_pair() const {
        return m_pair;
    }
    
    template<typename TKEY, typename TVAL>
    const TVAL& entry<TKEY, TVAL>::get_value() const {
        return m_pair.second();
    }

    template<typename TKEY, typename TVAL>
    const TKEY& entry<TKEY, TVAL>::get_key() const {
        return m_pair.first();
    }
    
    template<typename TKEY, typename TVAL>
    template<typename TVAL_>
    void entry<TKEY, TVAL>::set_value(TVAL_&& value) {
        m_pair.second() = std::forward<TVAL_>(value);
    }

    template<typename TKEY, typename TVAL>
    std::size_t entry<TKEY, TVAL>::get_hash() const {
        return m_hash;
    }

} //namespace map

} //namespace tc

#endif /* C151DFEA_DFA0_44BB_9C18_756422D43DDA */
