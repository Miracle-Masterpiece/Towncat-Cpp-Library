#ifndef JSTD_CPP_LANG_HASH_MAP_H_
#define JSTD_CPP_LANG_HASH_MAP_H_

#include <internal/shared/map_utils.hpp>
#include <cpp/lang/utils/map_entry.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/array.hpp>
#include <initializer_list>
#include <cassert>

namespace tc
{

/**
 * Hash table implementation.
 *
 * This implementation provides constant performance for the functions
 * get, insert, and put, provided that the key hashes are well distributed.
 *
 * The performance of this implementation is affected by two parameters: initial capacity and load factor.
 *
 * The initial capacity is simply the number of buckets in the hash table.
 * The load factor is the ratio of the number of elements to the number of buckets after which rehashing and increasing the buckets will occur.
 *
 * The default load factor is 0.75.
 *
 * This implementation is not thread-safe.
 *
 * By default, tc::hash_for<TKEY> is used for key hashing.
 * By default, tc::equal_to<TKEY> is used for key comparison.
 *
 * For iteration, the tc::pair<TKEY, TVALUE> class is used.
 *
 * @example
 *       tc::hash_map<int, tc::string> map = {
 *           {1, "one"},
 *           {2, "two"},
 *           {3, "three"},
 *           {4, "four"}
 *       };
 *
 *       for (tc::pair<int, tc::string>& p : map) {
 *           std::cout << p.first() << " = " << p.second() << "\n";
 *       } 
 * 
 * @example 
 *       const tc::hash_map<int, tc::string> map = { 
 *           {1, "one"}, 
 *           {2, "two"}, 
 *           {3, "three"}, 
 *           {4, "four"} 
 *       }; 
 * 
 *       for (const tc::pair<int, tc::string>& p : map) { 
 *           std::cout << p.first() << " = " << p.second() << "\n"; 
 *       } 
 */
template<typename TKEY, typename TVALUE, typename THASHER = hash_for<TKEY>, typename TEQUALER = equal_to<TKEY>>
class hash_map {
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
    * The memory management allocator is a hash map.
    */
    tca::allocator* const m_allocator;

   /**
    * An array of pointers to linked lists of nodes storing the hash map values.
    */
    array<entry*> m_buckets;

    /**
    * Number of elements stored in the hash map
    */
    std::size_t m_size;

    /**
     * 
     */
    float m_load_factor;

   /**
    * Increases the capacity of this hash map.
    * Rehash allocates a new, larger array,
    * and then appends all entries to it.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory
    */
    void rehash();

   /**
    * Returns the load factor of this hash map.
    * The load factor is the value of size() / m_buckets.length
    */
    float get_load_factor() const;

   /**
    * Attempts to increase the capacity of this hash map.
    *
    * First, checks that the bucket array length is not 0, then allocates a new array.
    * Otherwise, checks that the current load factor is not greater than m_load_factor,
    * in which case, reallocates memory for buckets.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    * 
    * @see rehash()
    * @see get_load_factor();
    */
    void ensure_capacity();

public:
   /**
    * Creates a hash map with the passed allocator.
    *
    * @param allocator
    *       The allocator responsible for allocating and deallocating memory for this hash map.
    */
    hash_map(tca::allocator* allocator = tca::get_default_allocator());
    
   /** 
    * Creates a hash map with the initial capacity. 
    * 
    * @param initial_capacity 
    *       Initial capacity of the hash map. 
    * 
    * @param load_factor 
    *       Hash map load factor. 
    *       The value above which memory will be redistributed and the number of buckets will increase. 
    * 
    * @param allocator 
    *       An allocator responsible for allocating and freeing memory for this hash map. 
    * 
    * @throws out_of_memory_error 
    *       If there is not enough memory. 
    */
    hash_map(std::size_t initial_capacity, float load_factor = 0.75f, tca::allocator* allocator = tca::get_default_allocator());
    
   /**
    * Creates a hash map and initializes it using std::initializer_list
    *
    * If duplicate elements appear in std::initializer_list, only the first entry will be added.
    *
    * @param load_factor
    *       The hash map load factor.
    *       The value above which memory will be reallocated and the number of buckets will be increased.
    *
    * @param allocator
    *       The allocator responsible for allocating and freeing memory for this hash map.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any element copy constructor exception.
    *
    * @example
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
    *       assert(map.size() == 2);
    */
    hash_map(const std::initializer_list<const pair<TKEY, TVALUE>>& init_list, float load_factor = 0.75f, tca::allocator* allocator = tca::get_default_allocator());

   /**
    * Copy constructor.
    *
    * Copies the values ​​of the passed hash map to this hash map.
    * The allocator of the copied hash map is used when constructing this hash map.
    *
    * @param map
    *       The hash map whose values ​​will be copied.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any element copy constructor exception.
    *
    * @example
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
    *       tc::hash_map<int, int> copied = map;
    *
    *       std::cout << copied.get(1) << "\n";
    *       std::cout << copied.get(2) << "\n"; 
    * 
    *       assert(copied.size() == map.size()); 
    *       assert(copied.contains_key(1)); 
    *       assert(copied.contains_key(2)); 
    *       assert(copied.get_allocator() == map.get_allocator()); 
    */
    hash_map(const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
   /**
    * Move constructor.
    *
    * Moves the resources associated with 'map' to this hashmap.
    * After moving, this hashmap will use the passed hashmap pointer.
    *
    * The allocator of 'map' is unchanged.
    * After moving, 'map' is left in a valid but unspecified state.
    *
    * @param map
    *       The hashmap whose resources will be moved to this hashmap.
    *
    * @example
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
    *       tc::hash_map<int, int> moved = std::move(map);
    *
    *       assert(moved.get_allocator() == map.get_allocator());
    *       assert(moved.size() == 2); 
    *       assert(moved.contains_key(1)); 
    *       assert(moved.contains_key(2)); 
    */
    hash_map(hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
   /**
    * Copy operator.
    *
    * Copies elements from the passed hash map to this hash map.
    *
    * This object's allocator is not modified during copying.
    *
    * Strong exception guarantee.
    * If an exception is thrown during copying, the objects are not modified.
    *
    * @param map
    *       The map from which the elements will be copied.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown by the element's copy constructor.
    *
    * @example
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}};
    *
    *       tca::malloc_free_allocator alloc;
    *       tc::hash_map<int, int> copied(&alloc); 
    *       
    *       copied = map; 
    *       
    *       assert(copied.size() == map.size()); 
    *       assert(copied.get_allocator() == &alloc); 
    *       assert(map.contains_key(1)); 
    *       assert(map.contains_key(2)); 
    */
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
   /**
    * Move operator.
    *
    * Moves elements from the passed hash map to this hash map.
    *
    * If the allocators are the same, ownership of the resource is simply transferred.
    * If the allocators are different, the elements are copied.
    *
    * Strong exception guarantee.
    * If an exception is thrown during copying, the objects are not modified.
    *
    * After moving, 'map' saves its allocator
    * and remains in a valid but unspecified state.
    *
    * @param map
    *       The map from which the elements will be moved.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory..
    *
    * @throws
    *       Any exception thrown by the element's copy constructor. * 
    * 
    * @example 
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}}; 
    *       
    *       tca::malloc_free_allocator alloc; 
    *       tc::hash_map<int, int> moved(&alloc); 
    *       
    *       moved = std::move(map); //Allocators are different, so this will result in a copy 
    *       assert(moved.get_allocator() = &alloc); 
    * 
    * @example 
    *       tc::hash_map<int, int> map = {{1, 1}, {2, 2}}; 
    *       tc::hash_map<int, int> moved; 
    *       
    *       moved = std::move(map); //Allocators are the same, fair move
    * 
    *       assert(moved.get_allocator() = map.get_allocator());
    *
    */
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
   /**
    * Inserts a key and value into this hash map.
    *
    * If the key is already mapped to a value, the value will be modified.
    *
    * Strong exception guarantee.
    * If the function throws an exception, the object will be preserved unchanged.
    *
    * @param key
    *       The key to be mapped to the value.
    *
    * @param value
    *       The value mapped to the key.
    *
    * @return
    *       If the key was successfully added to the hash map or the value was modified.
    *       Otherwise, false.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown from the constructor (copy/move) of the key or value.
    *
    *
    * @see insert()
    * @see size()
    * @see replace()
    */
    template<typename TKEY_, typename TVALUE_>
    bool put(TKEY_&& key, TVALUE_&& value);
    
   /**
    * Inserts a key and value into this hash map.
    *
    * If the key is already mapped to a value, do nothing.
    *
    * Strong exception guarantee.
    * If the function throws an exception, the object is preserved unchanged.
    *
    * @param key
    *       The key to be mapped to the value.
    *
    * @param value
    *       The value mapped to the key.
    *
    * @return
    *       If the key was successfully added to the hash map.
    *       If this key already exists in the hash map, return false.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    * Any exception thrown from the constructor (copy/move) of the key or value.
    *
    * @see put()
    * @see size()
    * @see replace()
    */
    template<typename TKEY_, typename TVALUE_>
    bool insert(TKEY_&& key, TVALUE_&& value);

   /**
    * Returns a reference to the value associated with the passed key.
    * If there is no equivalent key to the passed one, an exception is thrown.
    *
    * @throws no_such_element_exception
    *       If no value exists for the passed key.
    */
    TVALUE& get(const TKEY& key);
    
   /**
    * Returns a constant reference to the value associated with the passed key.
    * If there is no equivalent key to the passed one, an exception is thrown.
    *
    * @throws no_such_element_exception
    *       If no value exists for the passed key.
    */
    const TVALUE& get(const TKEY& key) const;
       
   /**
    * Replaces the value, given the key, with a new value.
    *
    * If an equivalent key does not exist in the hash map, the function does nothing.
    *
    * @return
    *       true if the value has been modified; otherwise, false.
    *
    * @throw
    *       Any exception thrown from the value's copy constructor.
    * 
    * @see put()
    * @see insert()
    */
    template<typename TVALUE_>
    bool replace(const TKEY& key, TVALUE_&& value);

   /**
    * Checks whether the hash map contains a key equivalent to the one passed.
    * 
    * @return
    *       true - only if the key is contained in the hash map.
    *       Otherwise, false.
    */
    bool contains_key(const TKEY& key) const;
    
   /**
    * Checks whether the hash map contains a value equivalent to the one passed.
    *
    * @return
    *       true - only if the value is contained in the hash map.
    *       Otherwise, false.
    */
    template<typename TVALUE_EQUALER = equal_to<TVALUE>>
    bool contains_value(const TVALUE& value) const;

   /**
    * Returns a reference to the value associated with the key.
    *
    * If the hashmap does not contain a key equivalent to the one passed in,
    * the default value passed in by the user is returned.
    *
    * @return
    *       A reference to the value from the hashmap, or a reference to the default value if the value does not exist in the hashmap.
    */
    template<typename DEFAULT_T, typename = typename enable_if<
                                                                is_same<
                                                                    typename remove_cv<DEFAULT_T>::type, typename remove_cv<TVALUE>::type
                                                                >::value
                                                                &&
                                                                is_cv_castable<TVALUE, DEFAULT_T>::value
                                                            >::type>
    DEFAULT_T& get_or_default(const TKEY& key, DEFAULT_T& default_value) {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
            return finded->get_value();
        return default_value;
    }
    
   /**
    * Returns a constant reference to the value associated with the key.
    *
    * If the hash map does not contain a key equivalent to the one passed in,
    * the default value passed in by the user is returned.
    *
    * @return
    *       A reference to the value from the hash map, or a reference to the default value if the value does not exist in the hash map.
    */
    template<typename DEFAULT_T, typename = typename enable_if<
                                                                is_same<
                                                                    typename remove_cv<DEFAULT_T>::type, typename remove_cv<TVALUE>::type
                                                                >::value
                                                            >::type>
    const DEFAULT_T& get_or_default(const TKEY& key, DEFAULT_T& default_value) const {
        const entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
            return finded->get_value();
        return default_value;
    }

   /**
    * Removes a key-value from the hash map.
    *
    * @return
    *       true if the deletion was successful.
    *       Otherwise, false.
    */
    bool remove(const TKEY& key);

   /**
    * Returns the allocator responsible for allocating
    * and deallocating memory for this hash map.
    */
    tca::allocator* get_allocator() const;

   /** 
    * Returns the hash code of this hash-map. 
    * The hash code is generated based on all stored elements. 
    * 
    * tca::hash_for<TKEY> and tca::hash_for<TVALUE> is used to generate the hash code of elements. 
    * 
    * @return 
    *       The hash code of this list. 
    */
    std::size_t hashcode() const;
    
   /**
    * Checks if this hash-map is equal to the passed 'map'.
    *
    * tc::equal_to<TKEY> and tc::equal_to<TVALUE> is used for comparison.
    *
    * @param map
    *       Another hash-map to check.
    *
    * @return
    *       true - only if the hash-map have the same sizes,
    *       and the same element contents. False - otherwise.
    *
    * @example
    *       tc::hash_map<int, int> ints_1 = {{1, 1}, {2, 2}};
    *       tc::hash_map<int, int> ints_2 = {{1, 1}, {2, 2}};
    *       tc::hash_map<int, int> ints_3 = {{1, 1}, {5, 5}};
    * 
    *       std::cout << ints_1.equals(ints_2) << "\n"; //output true 
    *       std::cout << ints_1.equals(ints_3) << "\n"; //output false 
    * 
    */
    template<typename THasher>
    bool equals(const hash_map<TKEY, TVALUE, THasher, TEQUALER>& map) const;

   /**
    * @return
    *       The number of elements this hash map stores.
    */
    std::size_t size() const;

   /**
    * @return
    *       Whether the map is empty.
    *       Returns true only if size() returns 0.
    *       Otherwise, returns false.
    */
    bool is_empty() const;

   /**
    * Clears this hash map.
    * After calling size(), size() will be 0.
    */
    void clear();

   /**
    * Inserts elements from the passed hash map into this hash map.
    * If the key already exists in this hash map, its value will be replaced.
    *
    * Basic exception safety guarantee.
    * If an exception occurs while adding one of the elements,
    * elements successfully added before the exception occurs remain in the map.
    *
    * @param map
    *       The hash map from which to insert values ​​into this hash map.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown by the constructor (copy or move) of the key or value.
    *
    */
    template<typename THASHER_, typename TEQUALER_>
    void put_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map);
    
    /**
    * Inserts elements from the passed hash map into this hash map.
    * If the key already exists in the map, this element is ignored.
    *
    * Basic exception safety guarantee.
    * If an exception occurs while adding one of the elements,
    * elements successfully added before the exception occurs remain in the map.
    *
    * @param map
    *       The hash map from which to insert values ​​into this hash map.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown by the constructor (copy or move) of the key or value.
    *
    */
    template<typename ITERATOR>
    void insert_all(ITERATOR begin, ITERATOR end);
    
   /**
    * Inserts elements from the passed iterator into this hash map.
    * If the key already exists in this hash map, its value will be replaced.
    *
    * The iterator must return a reference to a tc::pair<TKEY, TVALUE>
    *
    * Basic exception safety guarantee.
    * If an exception occurs while adding one of the elements,
    * elements successfully added before the exception occurs remain in the map.
    *
    * @param begin
    *       The iterator to the beginning.
    *
    * @param end
    *       The iterator to the end.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown by the constructor (copy or move) of the key or value.
    *
    */
    template<typename ITERATOR>
    void put_all(ITERATOR begin, ITERATOR end);
    
   /**
    * Checks whether the keys of the passed hash map are contained in this hash map.
    *
    * @param map
    *       The hash map whose keys will be checked.
    *
    * @return
    *       true if all keys of 'map' are contained in this hash map.
    *       Otherwise, false.
    */
    template<typename THASHER_, typename TEQUALER_>
    bool contains_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const;
    
   /**
    * Checks whether the keys from the passed iterator are contained in this hash map.
    *
    * The iterator must return a reference to a tc::pair<TKEY, TVALUE>
    *
    * @param begin
    *       The iterator to the beginning.
    *
    * @param end
    *       The iterator to the end.
    *
    * @return
    *       true if all keys of 'map' are contained in this hash map.
    *       Otherwise, false.
    */
    template<typename ITERATOR>
    bool contains_all(ITERATOR begin, ITERATOR end) const;

   /**
    * Clears the resources associated with this hashmap.
    */
    ~hash_map();

private:
    /**
     * 
     */
    template<typename TENTRY, typename PAIR_T>
    class iterator_impl {
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
        iterator_impl(TENTRY* const* e, std::size_t length);
        
        /**
         * 
         */
        iterator_impl(const iterator_impl<TENTRY, PAIR_T>&) = default;
        
        /**
         * 
         */
        iterator_impl(iterator_impl<TENTRY, PAIR_T>&&) = default;
        
        /**
         * 
         */
        iterator_impl<TENTRY, PAIR_T>& operator= (const iterator_impl<TENTRY, PAIR_T>&) = default;
        
        /**
         * 
         */
        iterator_impl<TENTRY, PAIR_T>& operator= (iterator_impl<TENTRY, PAIR_T>&&) = default;
        
        /**
         * 
         */
        ~iterator_impl() = default;

        /**
         * 
         */
        PAIR_T& operator* ();

        /**
         * 
         */
        bool operator!=(const iterator_impl<TENTRY, PAIR_T>&) const;
        
        /**
         * 
         */
        bool operator==(const iterator_impl<TENTRY, PAIR_T>&) const;
        
        /**
         * 
         */
        iterator_impl<TENTRY, PAIR_T>& operator++ ();
        
        /**
         * 
         */
        iterator_impl<TENTRY, PAIR_T> operator++ (int);
    };

public:
    /**
     * 
     */
    typedef iterator_impl<const entry, const pair<TKEY, TVALUE>> const_iterator;
    
    /**
     * 
     */
    typedef iterator_impl<entry, pair<TKEY, TVALUE>> iterator;

    /**
     * 
     */
    iterator begin() {
        return iterator(m_buckets.data(), m_buckets.length);
    }
    
    /**
     * 
     */
    iterator end() {
        return iterator(nullptr, m_buckets.length);
    }
    
    /**
     * 
     */
    const_iterator begin() const {
        return const_iterator(m_buckets.data(), m_buckets.length);
    }
    
    /**
     * 
     */
    const_iterator end() const {
        return const_iterator(nullptr, m_buckets.length);
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
            insert_all(init_list.begin(), init_list.end());
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hash_map(const hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) :
        hash_map(map.get_allocator()) {
        try {
            insert_all(map.begin(), map.end());
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
            hash_map<TKEY, TVALUE, THASHER, TEQUALER> tmp(0, m_load_factor, m_allocator);
            tmp.insert_all(map.begin(), map.end());
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
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::rehash() {
        std::size_t oldcap = m_buckets.length;
        std::size_t newcap = oldcap + oldcap / 2;
        newcap = newcap >= 16 ? newcap : 16;
        internal::map::rehash<THASHER, TEQUALER>(m_buckets, newcap);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::ensure_capacity() {
        if (m_buckets.length == 0)
        {
            m_buckets = array<entry*>(16, m_allocator);
            m_buckets.set(nullptr);
        }
        else if (get_load_factor() > m_load_factor)
        {
            rehash();
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put(TKEY_&& key, TVALUE_&& value) {
        
        entry* finded   = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            finded->set_value(std::forward<TVALUE_>(value));
            return true;
        }
        else
        {
            ensure_capacity();

            std::size_t idx     = internal::map::bucket_index<THASHER>(key, m_buckets);
            std::size_t hash    = internal::map::hash_key<THASHER>(key);

            entry* e = internal::map::alloc_entry<entry>(
                                                            std::forward<TKEY_>(key),
                                                            std::forward<TVALUE_>(value),
                                                            hash,
                                                            m_allocator
                                                        );
            
            internal::map::append_entry(idx, e, m_buckets);
            
            ++m_size;
            return true;
        }
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::insert(TKEY_&& key, TVALUE_&& value) {
        
        ensure_capacity();

        entry* added = internal::map::insert<THASHER, TEQUALER>(
                                                std::forward<TKEY_>(key),
                                                std::forward<TVALUE_>(value),
                                                m_buckets,
                                                m_allocator
                                            );

        if (added)
        {
            ++m_size;
            return true;
        }
        
        return false;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::replace(const TKEY& key, TVALUE_&& value) {

        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
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
                internal::map::free_entry(i, m_allocator);
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
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_key(const TKEY& key) const {
        return internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets) != nullptr;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_EQUALER>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_value(const TVALUE& value) const {
        TVALUE_EQUALER equals;
        for (const pair<TKEY, TVALUE>& e: *this) {
            if (equals(e.get_value(), value))
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
            return objects::hashcode(begin(), end(), hash_for<pair<TKEY, TVALUE>>());
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
        internal::map::clear(m_buckets, m_allocator);
        m_size = 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename ITERATOR>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(ITERATOR begin, ITERATOR end) {
        while (begin != end)
        {
            const pair<TKEY, TVALUE>& p = *begin;
            put(p.first(), p.second());
            ++begin;
        }
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename ITERATOR>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::insert_all(ITERATOR begin, ITERATOR end) {
        while (begin != end)
        {
            const pair<TKEY, TVALUE>& p = *begin;
            insert(p.first(), p.second());
            ++begin;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    void hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) {
        put_all(map.begin(), map.end());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename ITERATOR>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(ITERATOR begin, ITERATOR end) const {
        while (begin != end)
        {
            const pair<TKEY, TVALUE>& p = *begin;
            if (!internal::map::find_entry<THASHER, TEQUALER>(p.first(), m_buckets) )
                return false;
            ++begin;
        }
        return begin == end;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(const hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const {
        return contains_all(map.begin(), map.end());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::iterator_impl(TENTRY* const* e, std::size_t length) :
        m_entries(e),
        m_node(nullptr),
        m_length(length),
        m_idx(0) {
            if (m_entries != nullptr)
                ++(*this);
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    PAIR_T& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator* () {
        JSTD_DEBUG_CODE(check_non_null(m_node));
        return m_node->get_pair();
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator!=(const iterator_impl<TENTRY, PAIR_T>& it) const {
        return m_node != it.m_node;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    bool hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator==(const iterator_impl<TENTRY, PAIR_T>& it) const {
        return m_node == it.m_node;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    typename hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator_impl<TENTRY, PAIR_T>& hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator++ () {
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
    template<typename TENTRY, typename PAIR_T>
    typename hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator_impl<TENTRY, PAIR_T> hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator++(int) {
        JSTD_DEBUG_CODE(check_non_null(m_node));
        iterator_impl<TENTRY, PAIR_T> it = *this;
        ++(*this);
        return it;
    }
}

#endif//JSTD_CPP_LANG_HASH_MAP_H_