#ifndef DFFD7E3E_2691_41B5_B59C_0AB637563A13
#define DFFD7E3E_2691_41B5_B59C_0AB637563A13

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

namespace map
{
namespace internal
{

template<typename K, typename V>
class linked_entry : public map::entry<K, V> {
    /**
     * 
     */
    linked_entry<K, V>* m_list_next;
    
    /**
     * 
     */
    linked_entry<K, V>* m_list_prev;
public:
    /**
     * 
     */
    template<typename K_, typename V_>
    linked_entry(K_&&, V_&&, std::size_t hashcode);

    /**
     * 
     */
    linked_entry<K, V>* get_next();
    
    /**
     * 
     */
    const linked_entry<K, V>* get_next() const;
    
    /**
     * 
     */
    void set_next(linked_entry<K, V>* e);

    /**
     * 
     */
    linked_entry<K, V>* get_list_next();
    
    /**
     * 
     */
    linked_entry<K, V>* get_list_prev();

    /**
     * 
     */
    const linked_entry<K, V>* get_list_next() const;
    
    /**
     * 
     */
    const linked_entry<K, V>* get_list_prev() const;
    
    /**
     * 
     */
    void set_list_next(linked_entry<K, V>* e);
    
    /**
     * 
     */
    void set_list_prev(linked_entry<K, V>* e);
};

} //namespace map
} //namespace map

/**
 * Implementation of hash tables and linked lists with favorable iteration ordering.
 * This implementation differs from the 'linked_hash_map' theme in that it preserves the order of element addition.
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
 * @example
 *       tc::linked_hash_map<int, tc::string> map = {
 *           {1, "one"},
 *           {2, "two"},
 *           {3, "three"},
 *           {4, "four"}
 *       };
 *
 *       for (tc::pair<int, tc::string>& p : map) {
 *           std::cout << p.first() << " = " << p.second() << "\n";
 *       } //output:
 *          1 = one
 *          2 = two
 *          3 = three
 *          4 = four
 * 
 * @example 
 *       const tc::linked_hash_map<int, tc::string> map = { 
 *           {1, "one"}, 
 *           {2, "two"}, 
 *           {3, "three"}, 
 *           {4, "four"} 
 *       }; 
 * 
 *       for (const tc::pair<int, tc::string>& p : map) { 
 *           std::cout << p.first() << " = " << p.second() << "\n"; 
 *       } //output:
 *          1 = one
 *          2 = two
 *          3 = three
 *          4 = four
 */
template<typename TKEY, typename TVALUE, typename THASHER = hash_for<TKEY>, typename TEQUALER = equal_to<TKEY>>
class linked_hash_map {
    using entry = map::internal::linked_entry<TKEY, TVALUE>;
private:
    /**
     * The memory management allocator is a this map.
     */
    tca::allocator* const m_allocator;

    /**
     * An array of pointers to linked lists of nodes storing the map values.
     */
    array<entry*> m_buckets;

    /**
     * Pointer to the first entry in the list
     */
    entry* m_head;
    
    /**
     * Pointer to the last entry in the list
     */
    entry* m_tail;

    /**
    * Number of elements stored in the linked_hash_map
    */
    std::size_t m_size;

    /**
     * 
     */
    float m_load_factor;

    /**
     * Determines whether accessing an element will affect its position in the linked list.
     */
    bool m_access_order;

    /**
    * Allocates memory and initializes a new entry.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any copy or move constructor exception.
    */
    template<typename TKEY_, typename TVALUE_>
    entry* alloc_entry(TKEY_&&, TVALUE_&&, std::size_t hashcode);
    
    /**
    * Calls the destructor and frees the memory of the passed entry.
    */
    void free_entry(entry*);

    /**
     * Attempts to increase the capacity of this linked_hash_map.
     *
     * First, checks that the bucket array length is not 0, then allocates a new array.
     * Otherwise, checks that the current load factor is not greater than m_load_factor,
     * in which case, reallocates memory for buckets.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @see rehash()
     * @see get_load_factor();
     */
    void ensure_capacity();

    /**
     * Adds an entry to the beginning of the linked list.
     * If this entry is already in the list, it must be removed first.
     *
     * @param e
     *      A pointer to the entry to add to the beginning of the linked list.
     *      'e' must be != nullptr
     *
     * @see unlink()
     */
    void link_last(entry* e);
    
    /**
     * Adds an entry to the ending of the linked list.
     * If this entry is already in the list, it must be removed first.
     *
     * @param e
     *      A pointer to the entry to add to the ending of the linked list.
     *      'e' must be != nullptr
     *
     * @see unlink()
     */
    void link_first(entry* e);
    
    /**
     * 
     */
    void unlink(entry* e);

    /**
     * Increases the capacity of this linked_hash_map.
     * Rehash allocates a new, larger array,
     * and then appends all entries to it.
     *
     * @throws out_of_memory_error
     *       If there is not enough memory
     */
    void rehash();

    /**
     * Returns the load factor of this linked_hash_map.
     * The load factor is the value of size() / m_buckets.length
     */
    float get_load_factor() const;

public:
    /**
     * Creates a map with the passed allocator.
     *
     * @param allocator
     *       The allocator responsible for allocating and deallocating memory for this map.
     */
    linked_hash_map(tca::allocator* allocator = tca::get_default_allocator());
    
    /** 
     * Creates a linked linked_hash_map with the initial capacity. 
     * 
     * @param initial_capacity 
     *       Initial capacity of the linked_hash_map. 
     * 
     * @param load_factor 
     *       Map load factor. 
     *       The value above which memory will be redistributed and the number of buckets will increase. 
     * 
     * @param access_order
     *       Whether accessing an element moves it to the end of the map.
     *       If true, accessed elements are moved to the end as if they were newly added.
     * 
     * @param allocator 
     *       An allocator responsible for allocating and freeing memory for this map. 
     * 
     * @throws out_of_memory_error 
     *       If there is not enough memory. 
     */
    linked_hash_map(std::size_t initial_capacity, float load_factor = 0.75f, bool access_order = false, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Creates a linked_hash_map and initializes it using std::initializer_list
     *
     * If duplicate elements appear in std::initializer_list, only the first entry will be added.
     *
     * @param load_factor
     *       The linked_hash_map load factor.
     *       The value above which memory will be reallocated and the number of buckets will be increased.
     *
     * @param allocator
     *       The allocator responsible for allocating and freeing memory for this linked_hash_map.
     * 
     * @param access_order
     *       Whether accessing an element moves it to the end of the map.
     *       If true, accessed elements are moved to the end as if they were newly added.
     * 
     * @throws out_of_memory_error
     *       If there is not enough memory.
     *
     * @throws
     *       Any element copy constructor exception.
     *
     * @example
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
     *       assert(map.size() == 2);
     */
    linked_hash_map(const std::initializer_list<pair<TKEY, TVALUE>>& init_list, float load_factor = 0.75f, bool access_order = false, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Copy constructor.
     *
     * Copies the values ​​of the passed linked_hash_map to this linked_hash_map.
     * The allocator of the copied linked_hash_map is used when constructing this linked_hash_map.
     *
     * @param map
     *       The linked_hash_map whose values ​​will be copied.
     *
     * @throws out_of_memory_error
     *       If there is not enough memory.
     *
     * @throws
     *       Any element copy constructor exception.
     *
     * @example
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
     *       tc::linked_hash_map<int, int> copied = map;
     *
     *       std::cout << copied.get(1) << "\n";
     *       std::cout << copied.get(2) << "\n"; 
     * 
     *       assert(copied.size() == map.size()); 
     *       assert(copied.contains_key(1)); 
     *       assert(copied.contains_key(2)); 
     *       assert(copied.get_allocator() == map.get_allocator()); 
     */
    linked_hash_map(const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * Move constructor.
     *
     * Moves the resources associated with 'map' to this linked_hash_map.
     * After moving, this linked_hash_map will use the passed linked_hash_map pointer.
     *
     * The allocator of 'map' is unchanged.
     * After moving, 'map' is left in a valid but unspecified state.
     *
     * @param map
     *       The linked_hash_map whose resources will be moved to this linked_hash_map.
     *
     * @example
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
     *       tc::linked_hash_map<int, int> moved = std::move(map);
     *
     *       assert(moved.get_allocator() == map.get_allocator());
     *       assert(moved.size() == 2); 
     *       assert(moved.contains_key(1)); 
     *       assert(moved.contains_key(2)); 
     */
    linked_hash_map(linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * Copy operator.
     *
     * Copies elements from the passed linked_hash_map to this linked_hash_map.
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
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}};
     *
     *       tca::malloc_free_allocator alloc;
     *       tc::linked_hash_map<int, int> copied(&alloc); 
     *       
     *       copied = map; 
     *       
     *       assert(copied.size() == map.size()); 
     *       assert(copied.get_allocator() == &alloc); 
     *       assert(map.contains_key(1)); 
     *       assert(map.contains_key(2)); 
     */
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map);
    
    /**
     * Move operator.
     *
     * Moves elements from the passed linked_hash_map to this linked_hash_map.
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
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}}; 
     *       
     *       tca::malloc_free_allocator alloc; 
     *       tc::linked_hash_map<int, int> moved(&alloc); 
     *       
     *       moved = std::move(map); //Allocators are different, so this will result in a copy 
     *       assert(moved.get_allocator() = &alloc); 
     * 
     * @example 
     *       tc::linked_hash_map<int, int> map = {{1, 1}, {2, 2}}; 
     *       tc::linked_hash_map<int, int> moved; 
     *       
     *       moved = std::move(map); //Allocators are the same, fair move
     * 
     *       assert(moved.get_allocator() = map.get_allocator());
     *
     */
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& operator= (linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map);
    
    /**
     * Inserts a key and value into this linked_hash_map.
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
     *       If the key was successfully added to the linked_hash_map or the value was modified.
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
     * Inserts a key and value into this linked_hash_map.
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
     *       If the key was successfully added to the linked_hash_map.
     *       If this key already exists in the linked_hash_map, return false.
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
     * Replaces the value, given the key, with a new value.
     *
     * If an equivalent key does not exist in the linked_hash_map, the function does nothing.
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
     * Checks whether the linked_hash_map contains a key equivalent to the one passed.
     * 
     * @return
     *       true - only if the key is contained in the linked_hash_map.
     *       Otherwise, false.
     */
    bool contains_key(const TKEY& key) const;
    
    /**
     * Checks whether the linked_hash_map contains a value equivalent to the one passed.
     *
     * @return
     *       true - only if the value is contained in the linked_hash_map.
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
    DEFAULT_T& get_or_default(const TKEY& key, DEFAULT_T& default_value);
    
   /**
    * Returns a constant reference to the value associated with the key.
    *
    * If the linked_hash_map does not contain a key equivalent to the one passed in,
    * the default value passed in by the user is returned.
    *
    * @return
    *       A reference to the value from the linked_hash_map, or a reference to the default value if the value does not exist in the linked_hash_map.
    */
    template<typename DEFAULT_T, typename = typename enable_if<
                                                                is_same<
                                                                    typename remove_cv<DEFAULT_T>::type, typename remove_cv<TVALUE>::type
                                                                >::value
                                                            >::type>
    const DEFAULT_T& get_or_default(const TKEY& key, DEFAULT_T& default_value) const;
    
    /**
     * Removes a key-value from the linked_hash_map.
     *
     * @return
     *      true if the deletion was successful.
     *      Otherwise, false.
     */
    bool remove(const TKEY& key);
    
    /**
     * Returns the allocator responsible for allocating
     * and deallocating memory for this linked_hash_map.
     */
    tca::allocator* get_allocator() const;

    /**
     * @return
     *       The number of elements this linked_hash_map stores.
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
     *       tc::linked_hash_map<int, int> ints_1 = {{1, 1}, {2, 2}};
     *       tc::linked_hash_map<int, int> ints_2 = {{1, 1}, {2, 2}};
     *       tc::linked_hash_map<int, int> ints_3 = {{1, 1}, {5, 5}};
     * 
     *       std::cout << ints_1.equals(ints_2) << "\n"; //output true 
     *       std::cout << ints_1.equals(ints_3) << "\n"; //output false 
     * 
     */
    template<typename THasher>
    bool equals(const linked_hash_map<TKEY, TVALUE, THasher, TEQUALER>& map) const;

   /**
    * Clears this linked_hash_map.
    * After calling size(), size() will be 0.
    */
    void clear();

   /**
    * Inserts elements from the passed linked_hash_map into this linked_hash_map.
    * If the key already exists in this linked_hash_map, its value will be replaced.
    *
    * Basic exception safety guarantee.
    * If an exception occurs while adding one of the elements,
    * elements successfully added before the exception occurs remain in the map.
    *
    * @param map
    *       The linked_hash_map from which to insert values ​​into this linked_hash_map.
    *
    * @throws out_of_memory_error
    *       If there is not enough memory.
    *
    * @throws
    *       Any exception thrown by the constructor (copy or move) of the key or value.
    *
    */
    template<typename THASHER_, typename TEQUALER_>
    void put_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map);

    /**
    * Inserts elements from the passed iterator into this linked_hash_map.
    * If the key already exists in this linked_hash_map, its value will be replaced.
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
    * Checks whether the keys of the passed linked_hash_map are contained in this linked_hash_map.
    *
    * @param map
    *       The linked_hash_map whose keys will be checked.
    *
    * @return
    *       true if all keys of 'map' are contained in this linked_hash_map.
    *       Otherwise, false.
    */
    template<typename THASHER_, typename TEQUALER_>
    bool contains_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const;

   /**
    * Checks whether the keys from the passed iterator are contained in this linked_hash_map.
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
    *       true if all keys of 'map' are contained in this linked_hash_map.
    *       Otherwise, false.
    */
    template<typename ITERATOR>
    bool contains_all(ITERATOR begin, ITERATOR end) const;

private:
    /**
     * 
     */
    template<typename TENTRY, typename PAIR_T>
    class iterator_impl {
        /**
         * 
         */
        TENTRY* m_entry;
    
    public:
        /**
         * 
         */
        iterator_impl(TENTRY* e = nullptr);
        
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
    typedef iterator_impl<entry, pair<TKEY, TVALUE>> iterator;
    
    /**
     * 
     */
    typedef iterator_impl<entry, const pair<TKEY, TVALUE>> const_iterator;

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
    iterator begin() {
        return iterator(m_head);
    }
    
    /**
     * 
     */
    iterator end() {
        return iterator(nullptr);
    }
    
    /**
     * 
     */
    const_iterator begin() const {
        return const_iterator(m_head);
    }
    
    /**
     * 
     */
    const_iterator end() const {
        return const_iterator(nullptr);
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
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(
        const std::initializer_list<pair<TKEY, TVALUE>>& init_list,
        float load_factor,
        bool access_order,
        tca::allocator* allocator
    ) : m_allocator(allocator), m_buckets(), m_head(nullptr), m_tail(nullptr), m_size(0), m_load_factor(load_factor), m_access_order(access_order) {
        m_buckets.set(nullptr);
        try {
            insert_all(init_list.begin(), init_list.end());
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::linked_hash_map(const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) :
        linked_hash_map() {
        try {
            insert_all(map.begin(), map.end());
        } catch (...) {
            clear();
            throw;
        }
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
        map.m_head      = nullptr;
        map.m_tail      = nullptr;
        map.m_size      = 0;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (const linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& map) {
        if (&map != this)
        {
            linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER> tmp(0, m_load_factor, m_access_order, m_allocator);
            tmp.insert_all(map.begin(), map.end());
            *this = std::move(tmp);
        }
        return *this;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::operator= (linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>&& map) {
        if (&map != this) {
            
            if (get_allocator() == map.get_allocator())
            {
                std::swap(m_buckets,        map.m_buckets);
                std::swap(m_head,           map.m_head);
                std::swap(m_tail,           map.m_tail);
                std::swap(m_size,           map.m_size);
                std::swap(m_load_factor,    map.m_load_factor);
                std::swap(m_access_order,   map.m_access_order);
            }
            else
            {
                *this = map;
            }
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
        m_allocator->deallocate(e);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::ensure_capacity() {
        if (m_buckets.length == 0)
        {
            m_buckets = array<entry*>(16, m_allocator);
            m_buckets.set(nullptr);
        }
        else if (get_load_factor() >= m_load_factor)
        {
            rehash();
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
        
        ensure_capacity();

        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            finded->set_value(std::forward<TVALUE_>(value));
            unlink(finded);
            link_last(finded);
            return true;
        }
        else
        {
            entry* e = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), internal::map::hash_key<THASHER>(key));
            
            std::size_t idx = internal::map::bucket_index<THASHER>(key, m_buckets);
            internal::map::append_entry(idx, e, m_buckets);
            
            ++m_size;
            link_last(e);
            
            if (remove_eldest_entry(m_head))
            {
                remove(m_head->get_key());
            }

            return true;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TKEY_, typename TVALUE_>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::insert(TKEY_&& key, TVALUE_&& value) {
        
        ensure_capacity();

        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
        {
            entry* e = alloc_entry(std::forward<TKEY_>(key), std::forward<TVALUE_>(value), internal::map::hash_key<THASHER>(key));
            
            std::size_t idx = internal::map::bucket_index<THASHER>(key, m_buckets);
            internal::map::append_entry(idx, e, m_buckets);
            
            ++m_size;
            link_last(e);
            
            if (remove_eldest_entry(m_head))
            {
                remove(m_head->get_key());
            }

            return true;
        }

        return false;
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
    template<typename TVALUE_>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::replace(const TKEY& key, TVALUE_&& value) {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            finded->set_value(std::forward<TVALUE_>(value));
            return true;
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) {
        
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
           throw make_except<no_such_element_exception>("No such element in map"); 
        
        if (m_access_order)
        {
            unlink(finded);
            link_last(finded);
        }

        return finded->get_value();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    const TVALUE& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get(const TKEY& key) const {

        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (!finded)
           throw make_except<no_such_element_exception>("No such element in map"); 
        
        return finded->get_value();
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename DEFAULT_T, typename>
    DEFAULT_T& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, DEFAULT_T& default_value) {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        if (finded)
        {
            if (m_access_order)
            {
                unlink(finded);
                link_last(finded);
            }
            return finded->get_value();
        }
        return default_value;
    }


    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename DEFAULT_T, typename>
    const DEFAULT_T& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::get_or_default(const TKEY& key, DEFAULT_T& value) const {
        entry* finded = internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
        
        if (finded)
            return finded->get_value();
        
        return value;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_key(const TKEY& key) const {
        return internal::map::find_entry<THASHER, TEQUALER>(key, m_buckets);
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TVALUE_EQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_value(const TVALUE& value) const {
        TVALUE_EQUALER equals;
        for (const pair<TKEY, TVALUE>& e: *this) {
            if (equals(e.get_value(), value))
                return true;
        }
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::link_last(entry* e) {
        assert(e != nullptr);
        e->set_list_next(nullptr);
        e->set_list_prev(nullptr);
        if (m_tail == nullptr)
        {
            m_head = m_tail = e;
        }
        else
        {
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
        if (m_head == nullptr)
        {
            m_head = m_tail = e;
        }
        else
        {
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
        return static_cast<float>(m_size) / static_cast<float>(m_buckets.length);
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
    std::size_t linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::hashcode() const {
        if (size() > 0)
            return objects::hashcode(begin(), end(), hash_for<pair<TKEY, TVALUE>>());
        else
            return 0;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THasher>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::equals(const linked_hash_map<TKEY, TVALUE, THasher, TEQUALER>& map) const {
        return objects::equals(begin(), end(), map.begin(), map.end(), equal_to<pair<TKEY, TVALUE>>());
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
    template<typename ITERATOR>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(ITERATOR begin, ITERATOR end) {
        while (begin != end)
        {
            const pair<TKEY, TVALUE>& p = *begin;
            put(p.first(), p.second());
            ++begin;
        }
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename ITERATOR>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::insert_all(ITERATOR begin, ITERATOR end) {
        while (begin != end)
        {
            const pair<TKEY, TVALUE>& p = *begin;
            insert(p.first(), p.second());
            ++begin;
        }
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename THASHER_, typename TEQUALER_>
    void linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::put_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) {
        put_all(map.begin(), map.end());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename ITERATOR>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(ITERATOR begin, ITERATOR end) const {
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
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::contains_all(const linked_hash_map<TKEY, TVALUE, THASHER_, TEQUALER_>& map) const {
        return contains_all(map.begin(), map.end());
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::remove_eldest_entry(entry* eldest) {
        return false;
    }

    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::iterator_impl(TENTRY* e) :
        m_entry(e) {

    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    PAIR_T& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator* () {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        return m_entry->get_pair();
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator!=(const iterator_impl<TENTRY, PAIR_T>& it) const {
        return m_entry != it.m_entry;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    bool linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator==(const iterator_impl<TENTRY, PAIR_T>& it) const {
        return m_entry == it.m_entry;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator_impl<TENTRY, PAIR_T>& linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator++ () {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        m_entry = m_entry->get_list_next();
        return *this;
    }
    
    template<typename TKEY, typename TVALUE, typename THASHER, typename TEQUALER>
    template<typename TENTRY, typename PAIR_T>
    typename linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::template iterator_impl<TENTRY, PAIR_T> linked_hash_map<TKEY, TVALUE, THASHER, TEQUALER>::iterator_impl<TENTRY, PAIR_T>::operator++(int) {
        JSTD_DEBUG_CODE(check_non_null(m_entry));
        iterator_impl<TENTRY, PAIR_T> it(m_entry);
        m_entry = m_entry->get_list_next();
        return it;
    }



namespace map
{
namespace internal
{

    template<typename K, typename V>
    template<typename K_, typename V_>
    linked_entry<K, V>::linked_entry(K_&& key, V_&& value, std::size_t hashcode) :
    entry<K, V>(std::forward<K_>(key), std::forward<V_>(value), hashcode) {
        m_list_next = nullptr;
        m_list_prev = nullptr;
    }
    
    template<typename K, typename V>
    linked_entry<K, V>* linked_entry<K, V>::get_next() {
        return static_cast<linked_entry<K, V>*>(entry<K, V>::get_next());
    }
    
    template<typename K, typename V>
    const linked_entry<K, V>* linked_entry<K, V>::get_next() const {
        return static_cast<linked_entry<K, V>*>(entry<K, V>::get_next());
    }
        
    template<typename K, typename V>
    void linked_entry<K, V>::set_next(linked_entry<K, V>* e) {
        entry<K, V>::set_next(e);
    }

    template<typename K, typename V>
    linked_entry<K, V>* linked_entry<K, V>::get_list_next() {
        return m_list_next;
    }
    
    template<typename K, typename V>
    linked_entry<K, V>* linked_entry<K, V>::get_list_prev() {
        return m_list_prev;
    }
    
    template<typename K, typename V>
    const linked_entry<K, V>* linked_entry<K, V>::get_list_next() const {
        return m_list_next;
    }
    
    template<typename K, typename V>
    const linked_entry<K, V>* linked_entry<K, V>::get_list_prev() const {
        return m_list_prev;
    }
    
    template<typename K, typename V>
    void linked_entry<K, V>::set_list_next(linked_entry<K, V>* e) {
        m_list_next = e;
    }
    
    template<typename K, typename V>
    void linked_entry<K, V>::set_list_prev(linked_entry<K, V>* e) {
        m_list_prev = e;
    }

} //namespace internal
} //namespace map

} //namespace tc

#endif /* DFFD7E3E_2691_41B5_B59C_0AB637563A13 */
