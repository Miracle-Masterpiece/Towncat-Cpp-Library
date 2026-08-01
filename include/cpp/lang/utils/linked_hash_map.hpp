#ifndef DFFD7E3E_2691_41B5_B59C_0AB637563A13
#define DFFD7E3E_2691_41B5_B59C_0AB637563A13

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/array.hpp>
#include <cassert>

namespace tc
{

template<typename TKEY, typename TVALUE, typename THASHER = hash_for<TKEY>, typename TEQUALER = equal_to<TKEY>>
class linked_hash_map {
public:
    class entry {
        /**
         * 
         */
        entry* m_next;
        
        /**
         * 
         */
        entry* m_list_next;
        
        /**
         * 
         */
        entry* m_list_prev;
        
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
        TVALUE m_value;
    
    public:
        /**
         * 
         */
        template<typename TKEY_, typename TVALUE_>
        entry(TKEY_&&, TVALUE_&&, std::size_t hashcode);

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
        TKEY& get_key();
        
        /**
         * 
         */
        TVALUE& get_value();
        
        /**
         * 
         */
        const TKEY& get_key() const;
        
        /**
         * 
         */
        const TVALUE& get_value() const;

        /**
         * 
         */
        entry* get_list_next();
        
        /**
         * 
         */
        entry* get_list_prev();

        /**
         * 
         */
        const entry* get_list_next() const;
        
        /**
         * 
         */
        const entry* get_list_prev() const;
        
        /**
         * 
         */
        void set_list_next(entry* e);
        
        /**
         * 
         */
        void set_list_prev(entry* e);

        /**
         * 
         */
        template<typename TVALUE_>
        void set_value(TVALUE_&&);
    };

private:
    /**
     * 
     */
    tca::allocator* m_allocator;

    /**
     * 
     */
    array<entry*> m_buckets;

    /**
     * 
     */
    entry* m_head;
    
    /**
     * 
     */
    entry* m_tail;

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
    bool m_access_order;

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
    void link_last(entry* e);
    
    /**
     * 
     */
    void link_first(entry* e);
    
    /**
     * 
     */
    void unlink(entry* e);

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
    TVALUE* get0(const TKEY& key);
    
    /**
     * 
     */
    const TVALUE* get0(const TKEY& key) const;

public:
    /**
     * 
     */
    linked_hash_map(tca::allocator* allocator = tca::get_scoped_or_default());
    
    /**
     * 
     */
    linked_hash_map(std::size_t initial_capacity, float load_factor = 0.75f, bool access_order = false, tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * 
     */
    linked_hash_map(const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * 
     */
    linked_hash_map(linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * 
     */
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * 
     */
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * 
     */
    template<typename TKEY_, typename TVALUE_>
    bool put(TKEY_&& key, TVALUE_&& value);

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
    const TVALUE& get_or_default(const TKEY& key, const TVALUE& value) const;
    
    /**
     * 
     */
    tca::allocator* get_allocator() const;

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
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER> clone(tca::allocator* allocator = nullptr) const;

    /**
     * 
     */
    template<typename THASHER_, typename TEQUALER_>
    void put_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map);

    /**
     * 
     */
    template<typename THASHER_, typename TEQUALER_>
    bool contains_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const;
    
    /**
     * 
     */
    template<typename TENTRY>
    class iterator {
        /**
         * 
         */
        TENTRY* m_entry;
    
    public:
        /**
         * 
         */
        iterator(TENTRY* e = nullptr);
        
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
        iterator<TENTRY>& operator++ ();
        
        /**
         * 
         */
        iterator<TENTRY> operator++ (int);
    };

    /**
     * 
     */
    virtual ~linked_hash_map();

    /**
     * 
     */
    virtual bool remove_eldest_entry(entry* eldest);

    /**
     * 
     */
    iterator<entry> begin() {
        return iterator<entry>(m_head);
    }
    
    /**
     * 
     */
    iterator<entry> end() {
        return iterator<entry>(nullptr);
    }
    
    /**
     * 
     */
    iterator<const entry> begin() const {
        return iterator<const entry>(m_head);
    }
    
    /**
     * 
     */
    iterator<const entry> end() const {
        return iterator<const entry>(nullptr);
    }
};

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(tca::allocator* allocator) :
        linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>(0, //< ёмкость нулевая, чтобы память в массиве выделилась не сразу, а только с первой вставкой в карту.
                                                            0.75f, false, allocator) {

    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(std::size_t initial_capacity, float load_factor, bool access_order, tca::allocator* allocator) :
        m_allocator(allocator),
        m_buckets(initial_capacity, allocator),
        m_head(nullptr),
        m_tail(nullptr),
        m_size(0),
        m_load_factor(load_factor),
        m_access_order(access_order) {
        m_buckets.set(nullptr);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) :
        linked_hash_map() {
        (*this) = std::move(map.clone());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map) :
        m_allocator(map.m_allocator),
        m_buckets(std::move(map.m_buckets)),
        m_head(map.m_head),
        m_tail(map.m_tail),
        m_size(map.m_size),
        m_load_factor(map.m_load_factor),
        m_access_order(map.m_access_order) {
        map.m_allocator = nullptr;
        map.m_head      = nullptr;
        map.m_tail      = nullptr;
        map.m_size      = 0;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) {
        if (&map != this) {
            linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER> tmp = map.clone(m_allocator);
            clear();
            (*this) = std::move(tmp);
        }
        return *this;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map) {
        if (&map != this) {
            clear();
            m_allocator = map.m_allocator;
            m_buckets   = std::move(map.m_buckets);
            m_head      = map.m_head;
            m_tail      = map.m_tail;
            m_size          = map.m_size;
            m_load_factor   = map.m_load_factor;
            m_access_order  = map.m_access_order;

            map.m_allocator = nullptr;
            map.m_head      = nullptr;
            map.m_tail      = nullptr;
            map.m_size      = 0;
        }
        return *this;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::~linked_hash_map() {
        clear();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::alloc_entry(TKEY_&& key, TVALUE_&& value, std::size_t hashcode) {
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
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::free_entry(entry* e) {
        assert(e != nullptr);
        e->~entry();
        m_allocator->deallocate(e, sizeof(entry));
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::lazy_init() {
        if (m_buckets.length == 0) {
            m_buckets = array<entry*>(16, m_allocator);
            m_buckets.set(nullptr);
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::rehash() {
        array<entry*> _new((std::size_t) (m_buckets.length + (m_buckets.length >> 1)));
        _new.set(nullptr);
        array<entry*> old = std::move(m_buckets);
        m_buckets = std::move(_new);
        
        THASHER hashcode;
        for (std::size_t i = 0, len = old.length; i < len; ++i) {
            for (entry* e = old[i]; e != nullptr; ) {
                entry* current = e;
                e = e->get_next();
                std::size_t hash   = hashcode(current->get_key());
                std::size_t idx    = hash % m_buckets.length;
                
                current->set_next(nullptr);
                if (!m_buckets[idx])
                {
                    m_buckets[idx] = current;
                }
                else
                {
                    entry* entr = m_buckets[idx];
                    while (entr->get_next())
                        entr = entr->get_next();
                    entr->set_next(current);
                }
            }
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put(TKEY_&& key, TVALUE_&& value) {
        lazy_init();

        if (get_load_factor() > m_load_factor) 
            rehash();

        THASHER hashcode;
        std::size_t hash   = hashcode(key);
        std::size_t idx    = hash % m_buckets.length;

        if (!m_buckets[idx])
        {
            m_buckets[idx] = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), hash);
            
            link_last(m_buckets[idx]);
            ++m_size;
            
            assert(m_head != nullptr);
            if (remove_eldest_entry(m_head))
                remove(m_head->get_key());

            return true;
        }
        else
        {
            TEQUALER equals;
            entry* prev = nullptr;
            for (entry* i = m_buckets[idx]; i != nullptr; prev = i, i = i->get_next()) {
                if (equals(i->get_key(), key)) {
                    i->set_value(std::forward<TVALUE_>(value));
                    return false;
                }
            }
            assert(prev != nullptr);
            
            entry* _new = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), hash);
            prev->set_next(_new);
            
            link_last(_new);
            ++m_size;

            assert(m_head != nullptr);
            if (remove_eldest_entry(m_head))
                remove(m_head->get_key());

            return true;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::remove(const TKEY& key) {
        if (is_empty())
            return false;
        THASHER hashcode;
        std::size_t hash   = hashcode(key);
        std::size_t idx    = hash % m_buckets.length;
        
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
                unlink(i);
                free_entry(i);
                --m_size;
                return true;
            }
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get0(const TKEY& key) {
        THASHER hashcode;
        std::size_t hash   = hashcode(key);
        std::size_t idx    = hash % m_buckets.length;
        TEQUALER equals;
        for (entry* i = m_buckets[idx]; i != nullptr; i = i->get_next()) {
            if (equals(i->get_key(), key)) {
                if (m_access_order) {
                    unlink(i);
                    link_last(i);
                }
                return &i->get_value();
            }
        }
        return nullptr;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get0(const TKEY& key) const {
        THASHER hashcode;
        std::size_t hash   = hashcode(key);
        std::size_t idx    = hash % m_buckets.length;
        TEQUALER equals;
        for (entry* i = m_buckets[idx]; i != nullptr; i = i->get_next()) {
            if (equals(i->get_key(), key)) {
                return &i->get_value();
            }
        }
        return nullptr;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::replace(const TKEY& key, TVALUE_&& value) {
        THASHER hashcode;
        std::size_t hash   = hashcode(key);
        std::size_t idx    = hash % m_buckets.length;
        TEQUALER equals;
        for (entry* i = m_buckets[idx]; i != nullptr; i = i->get_next()) {
            if (equals(i->get_key(), key)) {
                i->set_value(std::forward<TVALUE_>(value));
                return true;
            }
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) {
        TVALUE* val = get0(key);
        if (val)
            return *val;
        else
            throw_except<no_such_element_exception>("No such element in map"); 
        throw 0; //[-Wreturn-type]
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) const {
        const TVALUE* val = get0(key);
        if (val)
            return *val;
        else
            throw_except<no_such_element_exception>("No such element in map"); 
        throw 0; //[-Wreturn-type]
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, TVALUE& value) {
        TVALUE* val = get0(key);
        if (val)
            return *val;
        else
            return value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, const TVALUE& value) const {
        const TVALUE* val = get0(key);
        if (val)
            return *val;
        else
            return value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_key(const TKEY& key) const {
        return get0(key) != nullptr;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_EQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_value(const TVALUE& value) const {
        TVALUE_EQUALER equals;
        for (const entry& e: *this) {
            if (equals(e.get_value(), e.get_value()))
                return true;
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::link_last(entry* e) {
        assert(e != nullptr);
        e->set_list_next(nullptr);
        e->set_list_prev(nullptr);
        if (m_tail == nullptr) {
            m_head = m_tail = e;
        } else {
            m_tail->set_list_next(e);
            e->set_list_prev(m_tail);
            m_tail = e;
        }
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::link_first(entry* e) {
        assert(e != nullptr);
        e->set_list_next(nullptr);
        e->set_list_prev(nullptr);
        if (m_head == nullptr) {
            m_head = m_tail = e;
        } else {
            e->set_list_next(m_head);
            m_head->set_list_prev(e);
            m_head = e;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::unlink(entry* e) {
        assert(e != nullptr);
        entry* const prev = e->get_list_prev();
        entry* const next = e->get_list_next();
        if (prev == nullptr)
        {
            m_head = e->get_list_next();
            if (m_head == nullptr) {
                m_head = m_tail = nullptr;
            } 
            
            else {
                m_head->set_list_prev(nullptr);
            }
        } 
        else if (next == nullptr)
        {
            m_tail = e->get_list_prev();
            if (m_tail != nullptr)
            {
                m_tail->set_list_next(nullptr);
            }
            else
            {
                m_head = m_tail = nullptr;
            }
        }
        else
        {
            prev->set_list_next(next);
            next->set_list_prev(prev);
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    std::size_t linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::size() const {
        return m_size;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    float linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_load_factor() const {
        return m_size / (float) m_buckets.length;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::is_empty() const {
        return m_size == 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    tca::allocator* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_allocator() const {
        return m_allocator;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::clear() {
        while (m_head)
        {
            entry* for_removal = m_head;
            m_head = m_head->get_list_next();
            free_entry(for_removal);
        }
        m_buckets.set(nullptr);
        m_head = m_tail = nullptr;
        m_size = 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER> linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::clone(tca::allocator* allocator) const {
        if (allocator == nullptr) {
            if (m_allocator == nullptr)
                return linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>();
            allocator = m_allocator;
        }
        linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER> result(m_buckets.length, m_load_factor, m_access_order, allocator);
        for (const entry& e : *this)
            result.put(e.get_key(), e.get_value());
        return linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>(std::move(result));
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) {
        for (const entry& e : map)
            put(e.get_key(), e.get_value());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const {
        for (const entry& e : map)
            if (!get0(e.get_key()))
                return false;
        return true;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::remove_eldest_entry(entry* eldest) {
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::entry(TKEY_&& key, TVALUE_&& value, std::size_t hashcode) :
        m_next(nullptr),
        m_list_next(nullptr),
        m_list_prev(nullptr),
        m_hash(hashcode),
        m_key(std::forward<TKEY_>(key)),
        m_value(std::forward<TVALUE_>(value)) {

    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_next() {
        return m_next;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_next() const {
        return m_next;
    }
        
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::set_next(entry* e) {
        m_next = e;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TKEY& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_key() {
        return m_key;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_value() {
        return m_value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TKEY& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_key() const {
        return m_key;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_value() const {
        return m_value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::set_value(TVALUE_&& value) {
        m_value = std::forward<TVALUE_>(value);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_list_next() {
        return m_list_next;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_list_prev() {
        return m_list_prev;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_list_next() const {
        return m_list_next;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry* linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::get_list_prev() const {
        return m_list_prev;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::set_list_next(entry* e) {
        m_list_next = e;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::entry::set_list_prev(entry* e) {
        m_list_prev = e;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::iterator(TENTRY* e) :
        m_entry(e) {

    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    TENTRY& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator* () const {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        return *m_entry;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator!=(const iterator<TENTRY>& it) const {
        return m_entry != it.m_entry;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator<TENTRY>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator++ () {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        m_entry = m_entry->get_list_next();
        return *this;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator<TENTRY> linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator<TENTRY>::operator++(int) {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        iterator<TENTRY> it(m_entry);
        m_entry = m_entry->get_list_next();
        return it;
    }
}

#endif /* DFFD7E3E_2691_41B5_B59C_0AB637563A13 */
