#ifndef JSTD_CPP_LANG_HASH_MAP_H_
#define JSTD_CPP_LANG_HASH_MAP_H_

#include <internal/shared/map_utils.hpp>
#include <cpp/lang/utils/map_entry.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/pair.hpp>
#include <cpp/lang/array.hpp>
#include <initializer_list>
#include <cassert>

namespace tc
{

template<typename TKEY, typename TVALUE, typename THASHER = hash_for<TKEY>, typename TEQUALER = equal_to<TKEY>>
class hash_map {

    /**
     * 
     */
    typedef typename remove_cv<TKEY>::type Kvalue;
    
    /**
     * 
     */
    typedef typename remove_cv<TVALUE>::type Vvalue;

public:
    /**
    * The node type used to store hash_map<T> elements.
    *
    * The type is declared publicly so it can be used.
    * when creating specialized allocators, such as pool_allocator,
    * customized allocation of hash_map<T> nodes.
    */
    typedef map::entry<TKEY, TVALUE> entry;
private:
    /**
     * 
     */
    tca::allocator* const m_allocator;

    /**
     * 
     */
    array<entry*> m_buckets;

    /**
     * 
     */
    std::size_t m_size;

    /**
     * 
     */
    float m_load_factor;

    /**
     * 
     */
    template<typename TKEY_, typename TVALUE_>
    entry* alloc_entry(TKEY_&&, TVALUE_&&, std::size_t hashcode);
    
    /**
     * 
     */
    void free_entry(entry*);

    /**
     * 
     */
    void lazy_init();

    /**
     * 
     */
    void rehash();

    /**
     * 
     */
    float get_load_factor() const;

    /**
     * 
     */
    void ensure_capacity();
public:
    /**
     * 
     */
    hash_map(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * 
     */
    hash_map(std::size_t initial_capacity, float load_factor = 0.75f, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * 
     */
    hash_map(const std::initializer_list<const pair<TKEY, TVALUE>>& init_list, float load_factor = 0.75f, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * 
     */
    hash_map(const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * 
     */
    hash_map(hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * 
     */
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * 
     */
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * 
     */
    template<typename TKEY_, typename TVALUE_>
    bool put(TKEY_&& key, TVALUE_&& value);
    
    /**
     * 
     */
    template<typename TKEY_, typename TVALUE_>
    bool insert(TKEY_&& key, TVALUE_&& value);

    /**
     * @throws no_such_element_exception
     *      Если значения по переданному ключу не существует.
     */
    TVALUE& get(const TKEY& key);
    
    /**
     * @throws no_such_element_exception
     *      Если значения по переданному ключу не существует.
     */
    const TVALUE& get(const TKEY& key) const;
       
    /**
     * 
     */
    template<typename TVALUE_>
    bool replace(const TKEY& key, TVALUE_&& value);

    /**
     * 
     */
    bool contains_key(const TKEY& key) const;
    
    /**
     * 
     */
    template<typename TVALUE_EQUALER = equal_to<TVALUE>>
    bool contains_value(const TVALUE& value) const;

    /**
     * 
     */
    TVALUE& get_or_default(const TKEY& key, TVALUE& value);
    
    /**
     * 
     */
    bool remove(const TKEY& key);

    /**
     * 
     */
    const TVALUE& get_or_default(const TKEY& key, TVALUE& value) const;
    
    /**
     * 
     */
    tca::allocator* get_allocator() const;

    /**
     * 
     */
    std::size_t hashcode() const;
    
    /**
     * 
     */
    template<typename THasher>
    bool equals(const hash_map<TKEY, TVALUE, THasher, TEQUALER>& map) const;

    /**
     * @return
     *      Размер этой карты.
     */
    std::size_t size() const;

    /**
     * @return
     *      Является ли карта пустой.
     */
    bool is_empty() const;

    /**
     * 
     */
    void clear();

    /**
     * 
     */
    template<typename THASHER_, typename TEQUALER_>
    void put_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map);

    /**
     * 
     */
    template<typename THASHER_, typename TEQUALER_>
    bool contains_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const;

    /**
     * 
     */
    ~hash_map();

    /**
     * 
     */
    template<typename TENTRY>
    class iterator {
        /**
         * 
         */
        TENTRY* const* m_entries;

        /**
         * 
         */
        TENTRY* m_node;

        /**
         * 
         */
        std::size_t m_length;

        /**
         * 
         */
        std::size_t m_idx;
    public:
        /**
         * 
         */
        iterator(TENTRY* const* e, std::size_t length);
        
        /**
         * 
         */
        iterator(const iterator<TENTRY>&) = default;
        
        /**
         * 
         */
        iterator(iterator<TENTRY>&&) = default;
        
        /**
         * 
         */
        iterator<TENTRY>& operator= (const iterator<TENTRY>&) = default;
        
        /**
         * 
         */
        iterator<TENTRY>& operator= (iterator<TENTRY>&&) = default;
        
        /**
         * 
         */
        ~iterator() = default;

        /**
         * 
         */
        TENTRY& operator* () const;

        /**
         * 
         */
        bool operator!=(const iterator<TENTRY>&) const;
        
        /**
         * 
         */
        bool operator==(const iterator<TENTRY>&) const;
        
        /**
         * 
         */
        iterator<TENTRY>& operator++ ();
        
        /**
         * 
         */
        iterator<TENTRY> operator++ (int);
    };

    /**
     * 
     */
    iterator<entry> begin() {
        return iterator<entry>(m_buckets.data(), m_buckets.length);
    }
    
    /**
     * 
     */
    iterator<entry> end() {
        return iterator<entry>(nullptr, m_buckets.length);
    }
    
    /**
     * 
     */
    iterator<const entry> begin() const {
        return iterator<const entry>(m_buckets.data(), m_buckets.length);
    }
    
    /**
     * 
     */
    iterator<const entry> end() const {
        return iterator<const entry>(nullptr, m_buckets.length);
    }
};

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(tca::allocator* allocator) :
        hash_map<TKEY, TVALUE, THASHER, TEQUALER>(0, //< ёмкость нулевая, чтобы память в массиве выделилась не сразу, а только с первой вставкой в карту.
                                                            0.75f, allocator) {

    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(std::size_t initial_capacity, float load_factor, tca::allocator* allocator) :
        m_allocator(allocator),
        m_buckets(initial_capacity, allocator),
        m_size(0),
        m_load_factor(load_factor) {
        m_buckets.set(nullptr);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(
        const std::initializer_list<const pair<TKEY, TVALUE>>& init_list,
        float load_factor,
        tca::allocator* allocator
    ) : m_allocator(allocator), m_buckets(), m_size(0), m_load_factor(load_factor) {
        m_buckets.set(nullptr);
        try {
            for (const pair<TKEY, TVALUE>& p : init_list)
            {
                insert(p.first(), p.second());
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) :
        hash_map(map.get_allocator()) {
        
        try {
            for (const entry& e : map)
            {
                insert(e.get_key(), e.get_value());
            }
        } catch (...) {
            clear();
            throw;
        }
    } 

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map) :
        m_allocator(map.m_allocator),
        m_buckets(std::move(map.m_buckets)),
        m_size(map.m_size),
        m_load_factor(map.m_load_factor) {
        map.m_size      = 0;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) {
        if (&map != this)
        {
            hash_map<TKEY, TVALUE, THASHER, TEQUALER> tmp(m_allocator);
            tmp.put_all(map);
            *this = std::move(tmp);
        }
        return *this;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map) {
        if (&map == this)
            return *this;

        if (get_allocator() == map.get_allocator())
        {
            std::swap(m_buckets,     map.m_buckets);
            std::swap(m_size,        map.m_size);
            std::swap(m_load_factor, map.m_load_factor);
        }
        else
        {
            *this = map;
        }
        
        return *this;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::~hash_map() {
        clear();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    typename hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* hash_map<TKEY, TVALUE, THASHER, TEQUALER>::alloc_entry(TKEY_&& key, TVALUE_&& value, std::size_t hashcode) {
        void* mem = m_allocator->allocate_align(sizeof(entry), alignof(entry));
        if (!mem)
            throw_except<out_of_memory_error>("Out of memory!");
        entry* e = nullptr;
        try {
            e = new(mem) entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), hashcode);
        } catch (...) {
            m_allocator->deallocate(mem, sizeof(entry));
            throw;
        }
        return e;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::free_entry(entry* e) {
        assert(e != nullptr);
        e->~entry();
        m_allocator->deallocate(e, sizeof(entry));
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::lazy_init() {
        if (m_buckets.length == 0)
        {
            m_buckets = array<entry*>(16, m_allocator);
            m_buckets.set(nullptr);
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::rehash() {
        std::size_t oldcap = m_buckets.length;
        std::size_t newcap = oldcap + oldcap / 2;
        newcap = newcap >= 16 ? newcap : 16;

        array<entry*> _new(newcap);
        _new.set(nullptr);

        array<entry*> old   = std::move(m_buckets);
        m_buckets           = std::move(_new);
        
        for (std::size_t i = 0, len = old.length; i < len; ++i)
        {
            for (entry* e = old[i]; e != nullptr; )
            {
                entry* current = e;
                internal::map::append_entry(current->get_hash() % newcap, current, m_buckets);
                e = e->get_next();
            }
        }

    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::ensure_capacity() {
        lazy_init();
        if (get_load_factor() > m_load_factor)
        {
            rehash();
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put(TKEY_&& key, TVALUE_&& value) {
        
        ensure_capacity();

        entry* finded   = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            finded->set_value(std::forward<TVALUE_>(value));
            return true;
        }
        else
        {
            entry* e = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), internal::map::hash_key<THASHER>(key));
            
            std::size_t idx = internal::map::bucket_index<THASHER>(key, m_buckets);
            internal::map::append_entry(idx, e, m_buckets);
            
            ++m_size;
            return true;
        }
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::insert(TKEY_&& key, TVALUE_&& value) {
        ensure_capacity();

        entry* finded   = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
        {
            entry* e = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), internal::map::hash_key<THASHER>(key));
            
            std::size_t idx = internal::map::bucket_index<THASHER>(key, m_buckets);
            internal::map::append_entry(idx, e, m_buckets);
            
            ++m_size;
            return true;
        }
        
        return false;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::replace(const TKEY& key, TVALUE_&& value) {
        ensure_capacity();

        entry* finded   = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            finded->set_value(std::forward<TVALUE_>(value));
        }
        
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::remove(const TKEY& key) {
        if (is_empty())
            return false;
        
        std::size_t idx  = internal::map::bucket_index<THASHER>(key, m_buckets);
        
        TEQUALER equals;
        entry* prev = nullptr;
        for (entry* i = m_buckets[idx]; i != nullptr; prev = i, i = i->get_next()) {
            if (equals(i->get_key(), key)) {
                if (prev)
                {
                    prev->set_next(i->get_next());
                }
                else
                {
                    m_buckets[idx] = i->get_next();
                }
                free_entry(i);
                --m_size;
                return true;
            }
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
            throw make_except<no_such_element_exception>("No such element in map"); 
        return finded->get_value();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) const {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
            throw make_except<no_such_element_exception>("No such element in map"); 
        return finded->get_value();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, TVALUE& value) {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
            return finded->get_value();
        else
            return value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, TVALUE& value) const {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
            return finded->get_value();
        else
            return value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_key(const TKEY& key) const {
        return internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets) != nullptr;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_EQUALER>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_value(const TVALUE& value) const {
        TVALUE_EQUALER equals;
        for (const entry& e: *this) {
            if (equals(e.get_value(), e.get_value()))
                return true;
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    std::size_t hash_map<TKEY, TVALUE, THASHER, TEQUALER>::size() const {
        return m_size;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    std::size_t hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hashcode() const {
        if (size() > 0)
            return objects::hashcode(begin(), end(), hash_for<entry>());
        else
            return 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THasher>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::equals(const hash_map<TKEY, TVALUE, THasher, TEQUALER>& map) const {
        return objects::equals(begin(), end(), map.begin(), map.end(), equal_to<entry>());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    float hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_load_factor() const {
        return static_cast<float>(m_size) / static_cast<float>(m_buckets.length);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::is_empty() const {
        return m_size == 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    tca::allocator* hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_allocator() const {
        return m_allocator;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::clear() {
        for (std::size_t i = 0; i < m_buckets.length; ++i) {
            entry* e = m_buckets[i];
            while (e)
            {
                entry* current = e;
                e = e->get_next();
                free_entry(current);
            }
        }
        m_buckets.set(nullptr);
        m_size = 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) {
        for (const entry& e : map) {
            put(e.get_key(), e.get_value());
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const {
        for (const entry& e : map)
            if (!find_entry(e.get_key()))
                return false;
        return true;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::iterator(TENTRY* const* e, std::size_t length) :
        m_entries(e),
        m_node(nullptr),
        m_length(length),
        m_idx(0) {
            if (m_entries != nullptr)
                ++(*this);
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    TENTRY& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator* () const {
        JSTD_DEBUG_CODE(check_non_null(m_node));
        return *m_node;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator!=(const iterator<TENTRY>& it) const {
        return m_node != it.m_node;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator==(const iterator<TENTRY>& it) const {
        return m_node == it.m_node;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    typename hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator<TENTRY>& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator++ () {
        if (m_node == nullptr || m_node->get_next() == nullptr)
        {
            for (std::size_t i = m_idx; i < m_length; ++i)
            {
                if (m_entries[i])
                {
                    m_node  = m_entries[i];
                    m_idx   = i + 1;
                    return *this;
                }
            }
            m_node = nullptr;
        }
        else
        {
            m_node = m_node->get_next();
        }
        return *this;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    typename hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator<TENTRY> hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator++(int) {
        JSTD_DEBUG_CODE(check_non_null(m_node));
        iterator<TENTRY> it(m_entries);
        m_entries = m_entries->get_list_next();
        return it;
    }
}

#endif//JSTD_CPP_LANG_HASH_MAP_H_