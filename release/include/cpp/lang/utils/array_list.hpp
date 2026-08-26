#ifndef JSTD_CPP_LANG_UTILS_ARRAY_LIST_H
#define JSTD_CPP_LANG_UTILS_ARRAY_LIST_H

#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/common.hpp>
#include <cstdint>
#include <utility>
#include <initializer_list>
#include <cassert>

namespace tc
{

namespace list
{

/**
 * Bounds-checked pointer iterator wrapper.
 * 
 * A lightweight iterator class that wraps a raw pointer with bounds checking
 * in debug builds. Provides iterator semantics for pointer-based ranges
 * with additional safety checks.
 * 
 * @tparam E
 *      The element type pointed to by the iterator.
 * 
 * @note
 *      In debug builds, dereferencing or incrementing past the end
 *      throws index_out_of_bound_exception.
 *      In release builds, these checks are omitted for performance.
 * 
 * @note
 *      The iterator stores both the current pointer and the end pointer
 *      for bounds checking in debug builds.
 * 
 * @example
 *      int arr[] = {1, 2, 3, 4, 5};
 *      ptr_iterator<int> begin(arr, arr + 5);
 *      ptr_iterator<int> end(arr + 5, arr + 5);
 *      
 *      for (auto it = begin; it != end; ++it) {
 *          std::cout << *it << " ";  // 1 2 3 4 5
 *      }
 *      // In debug builds, *end would throw an exception
 * 
 * @warning
 *      In release builds, no bounds checking is performed.
 *      Dereferencing past-the-end is undefined behavior.
 * 
 * @see
 *      array::begin()
 *      array::end()
 */
template<typename E>
class ptr_iterator {
    E* ptr;
    
    JSTD_DEBUG_CODE(
        E* end;
        /**
         * Checks if dereference is valid.
         * 
         * @throws index_out_of_bound_exception
         *      if ptr is null or out of bounds.
         */
        void check_deref() {
            check_non_null(ptr, "Iterator is null");
            if (ptr >= end)
                throw_except<index_out_of_bound_exception>("Iterator out of bound");
        }

        /**
         * Checks if increment is valid.
         * 
         * @throws index_out_of_bound_exception
         *      if increment would go past end.
         */
        void can_increment() {    
            if (ptr >= end)
                throw_except<index_out_of_bound_exception>("Iterator out of bound");
        }
    );
    
public:
    /**
     * Constructs an iterator from a pointer with bounds.
     * 
     * @param ptr
     *      Current pointer.
     * 
     * @param end
     *      End pointer (one past the last valid element).
     * 
     * @example
     *      int arr[] = {1, 2, 3};
     *      ptr_iterator<int> it(arr, arr + 3);
     */
    ptr_iterator(E* ptr, E* end) : ptr(ptr)
#ifndef NDEBUG
    , end(end)
#endif
    {}

    /**
     * Dereferences the iterator.
     * 
     * @return
     *      Reference to the pointed-to element.
     * 
     * @throws index_out_of_bound_exception in debug builds 
     *      If ptr is nullptr
     *      If ptr is out of bounds (ptr >= end)
     * 
     * @warning
     *      In release builds, no bounds checking is performed.
     *      Dereferencing an invalid iterator is undefined behavior.
     * 
     * @example
     *      ptr_iterator<int> it(arr, arr + 3);
     *      int x = *it;  // arr[0]
     */
    E& operator* () {
        JSTD_DEBUG_CODE(check_deref());
        return *ptr;
    }
    
    /**
     * Compares two iterators for equality.
     * 
     * @param it
     *      The other iterator to compare with.
     * 
     * @return
     *      true if both iterators point to the same address.
     */
    bool operator== (const ptr_iterator<E>& it) {
        return ptr == it.ptr;
    }
    
    /**
     * Compares two iterators for inequality.
     * 
     * @param it
     *      The other iterator to compare with.
     * 
     * @return
     *      true if iterators point to different addresses.
     */
    bool operator!= (const ptr_iterator<E>& it) {
        return ptr != it.ptr;
    }
    
    /**
     * Pre-increment operator.
     * 
     * Advances the iterator to the next element.
     * 
     * @return
     *      Reference to this iterator after increment.
     * 
     * @throws index_out_of_bound_exception in debug builds
     *      If ptr is out of bounds (ptr >= end)
     */
    ptr_iterator<E>& operator++ () {
        JSTD_DEBUG_CODE(can_increment());
        ++ptr;
        return *this;
    }
    
    /**
     * Post-increment operator.
     * 
     * Advances the iterator to the next element and returns
     * the previous value.
     * 
     * @return
     *      A copy of the iterator before increment.
     * 
     * @throws index_out_of_bound_exception in debug builds:
     *      If ptr is out of bounds (ptr >= end)
     */
    ptr_iterator<E> operator++ (int) {
        JSTD_DEBUG_CODE(can_increment());
        ptr_iterator<E> tmp = *this;
        ++ptr;
        return tmp;
    }
};

};

/**
 * Dynamic array-based list container with allocator support.
 * 
 * A dynamically resizable array list that provides O(1) random access,
 * O(n) insertion/removal at arbitrary positions, and O(1) amortized
 * insertion at the end.
 * 
 * @tparam E
 *      The type of elements stored in the list.
 * 
 * @note
 *      The list grows automatically when capacity is exceeded.
 *      Growth factor is 1.5x (capacity + capacity/2).
 * 
 * @warning
 *      Iterator invalidation: any modification to the list
 *      invalidates all iterators.
 * 
 * @example
 *      // Create list and add elements
 *      array_list<int> list;
 *      list.add(10);
 *      list.add(20);
 *      list.add(5);
 *      
 *      // Access elements
 *      int first = list.at(0);  // 10
 *      list[0] = 100;
 *      
 *      // Iterate
 *      for (int& i : list) {
 *          std::cout << i << " ";
 *      }
 */
template<typename E>
class array_list {
    
    /**
     * Iterator type alias.
     */
    typedef list::ptr_iterator<E> iterator;

    /**
     * Underlying type without cv-qualifiers.
     */
    typedef typename remove_cv<E>::type Evalue;
    
    /**
     * Default initial capacity.
     */
    static const int DEFAULT_CAPACITY = 10;    

    /**
     *  Allocator used for memory management.
     */
    tca::allocator* const m_allocator;
    
    /**
     * Pointer to element storage.
     */
    Evalue* m_data;
    
    /**
     * Current allocated capacity.
     */
    std::size_t m_capacity;
    
    /**
     * Current number of elements.
     */
    std::size_t m_size;
    
    /**
     * Frees all resources.
     * 
     * Destroys all elements and deallocates memory.
     */
    void cleanup();
    
    /**
     * Grows the capacity.
     * 
     * Increases capacity by 1.5x (capacity + capacity/2).
     * If capacity is 0, sets to DEFAULT_CAPACITY.
     * 
     * @throws out_of_memory_error
     *      if allocation fails.
     */
    void grow();

public:

    /**
     * Default constructor.
     * 
     * @param allocator
     *      Allocator to use. Uses default if not provided.
     */
    array_list(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Constructs with initial capacity.
     * 
     * @param init_capacity
     *      Initial capacity.
     * 
     * @param allocator
     *      Allocator to use.
     * 
     * @throws out_of_memory_error
     *      if allocation fails.
     */
    explicit array_list(std::size_t init_capacity, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Constructs an array list from an initializer list.
     * 
     * Creates a new array list containing the elements from the initializer list.
     * The capacity is pre-allocated to exactly match the number of elements.
     * 
     * @param init_list
     *      Initializer list of elements to populate the list with.
     * 
     * @param allocator
     *      Allocator to use for memory management.
     *      If not provided, uses the global default allocator.
     * 
     * @throws out_of_memory_error
     *      If memory allocation fails.
     * 
     * @throws
     *      Any exception thrown by E's copy constructor.
     * 
     * @note
     *      The list reserves exactly init_list.size() capacity upfront,
     *      avoiding reallocations during construction.
     * 
     * @note
     *      Elements are copied from the initializer list into the list.
     *      The initializer list remains unchanged.
     * 
     * @warning
     *      If an exception is thrown during construction, all resources
     *      are properly cleaned up (strong exception guarantee).
     * 
     * @example
     *      // Create list with integers
     *      array_list<int> list = {1, 2, 3, 4, 5};
     *      assert(list.size() == 5);
     *      assert(list[0] == 1);
     *      assert(list[4] == 5);
     * 
     *      // Create list with strings using custom allocator
     *      tca::allocator* custom = get_custom_allocator();
     *      array_list<std::string> names = {"Alice", "Bob", "Charlie", custom};
     * 
     *      // Empty initializer list creates an empty list
     *      array_list<double> empty = {};
     *      assert(empty.is_empty());
     */
    array_list(std::initializer_list<E> init_list, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Copy constructor.
     * Creates a copy of this array_list.
     * 
     * When creating an array_list, the allocator of the copied object is used and saved.
     * 
     * @throws out_of_memory_error
     * If there is not enough memory.
     * 
     * @throws
     * Any exception thrown by the element's copy constructor.
     * 
     * @example 
     *      tc::array_list<int> ints = {1, 2, 3, 4};
     *      tc::array_list<int> copied = ints;
     *      
     *      for (int& i : copied) {
     *          std::cout << i << "\n";
     *      }
     * 
     *      assert(copied.size() == ints.size());
     *      assert(copied.get_allocator() == ints.get_allocator());
     * 
     * @example 
     *      tc::array_list<int> ints;
     *      tc::array_list<int> copied = ints;
     *      assert(copied.size() == 0);
     *      assert(copied.get_allocator() == ints.get_allocator());
     */
    array_list(const array_list<E>& list);
    
    /**
     * Move constructor.
     * 
     * Passes the allocated memory, size, capacity and allocator to the current array_list.
     * 
     * After moving 'list' is guaranteed to retain its allocator
     * and will remain in a valid but unspecified state.
     * 
     * @example
     *      tc::array_list<int> ints = {0, 1, 2, 3, 4};
     *      tc::array_list<int> moved = std::move(its);
     * 
     *      assert(moved.size() == 5);
     *      assert(moved.get_allocator() == ints.get_allocator());
     */
    array_list(array_list<E>&& list);

    /**
     * Copies the elements from 'list' to this array_list.
     * 
     * Strong exceptions guarantee maintained.
     * If an exception occurs while allocating memory or copying an element, 
     * the current object remains unchanged.
     * 
     * When copying, the allocator of the current object does not change.
     * To create a copy, the allocator of the current object is used.
     * 
     * After successful copying, the old contents of the current object
     * is freed and replaced with a copy of the contents of 'list'.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::array_list<int> ints = {0, 1, 2, 3, 4};
     *      tc::array_list<ints> copied;
     *      copied = ints;
     * 
     *      for (int& i : copied) {
     *          std::cout << i << "\n";
     *      }
     * 
     * assert(compied.size() == ints.size());
     */
    array_list<E>& operator=(const array_list<E>& list);
    
    /**
     * Moves data from the 'list' object to this array_list.
     * There is a strong exceptions guarantee when moving.
     * If an exception occurs while moving, both objects remain unchanged.
     * 
     * The allocator of the current object does not change when moved.
     * 
     * If the current object's allocator and 'a' are the same, 
     * memory is transferred to the current object without copying.
     * 
     * If the allocators do not match, a copy is performed.
     * 
     * After moving, 'list' remains in a valid but unspecified state.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws 
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::array_list<int> ints = {0, 1, 2, 3, 4};
     *      
     *      tc::array_list<int> moved;
     *      tc::allocator* alloc = moved.get_allocator(); 
     * 
     *      moved = std::move(ints);
     * 
     *      assert(moved.size() == 5);
     *      assert(moved.get_allocator() == alloc);
     */
    array_list<E>& operator=(array_list<E>&& list);
    
    /**
     * 
     * Destroys all elements and deallocates memory.
     */
    ~array_list();
    
    /**
     * Adds an element to the end of the list.
     * 
     * @tparam _E
     *      Element type (deduced from argument).
     * 
     * @param e
     *      Element to add (forwarded to preserve value category).
     * 
     * @throws out_of_memory_error
     *      If reallocation fails.
     * 
     * @throws
     *      Any exception thrown by E's move/copy constructor.
     * 
     * @note
     *      If capacity is insufficient, the list grows automatically
     *      using grow() which increases capacity by 1.5x.
     * 
     * @note
     *      Amortized O(1) time complexity.
     * 
     * @example
     *      array_list<int> list;
     *      list.add(10);
     *      list.add(20);
     *      list.add(30);  // list = {10, 20, 30}
     */
    template<typename _E>
    void add(_E&& e);
    
    /**
     * Inserts an element at the specified position.
     * 
     * @tparam _E
     *      Element type (deduced from argument).
     * 
     * @param idx
     *      Position to insert at (0-based).
     * 
     * @param e
     *      Element to insert (forwarded to preserve value category).
     * 
     * @throws index_out_of_bound_exception 
     *      If idx > size().
     * 
     * @throws out_of_memory_error
     *      If reallocation fails.
     * 
     * @throws
     *      Any exception thrown by E's move/copy constructor.
     * 
     * @note
     *      All elements from idx to end are shifted right by one position.
     *      O(n) time complexity.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      list.add(1, 99);  // list = {1, 99, 2, 3}
     */
    template<typename _E>
    void add(std::size_t idx, _E&& e);
    
    /**
     * Finds the last occurrence of an element.
     * 
     * @param e
     *      Element to search for.
     * 
     * @return
     *      Index of the last occurrence, or npos() if not found.
     * 
     * @note
     *      O(n) time complexity.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3, 2, 1};
     *      size_t pos = list.last_index_of(2);  // pos = 3
     */
    std::size_t last_index_of(const E& e) const;
    
    /**
     * Finds the first occurrence of an element.
     * 
     * @param e
     *      Element to search for.
     * 
     * @return
     *      Index of the first occurrence, or npos() if not found.
     * 
     * @note
     *      O(n) time complexity.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3, 2, 1};
     *      size_t pos = list.index_of(2);  // pos = 1
     */
    std::size_t index_of(const E& e) const;
    
    /**
     * Checks if the list contains an element.
     * 
     * @param e
     *      Element to search for.
     * 
     * @return
     *      true if found, false otherwise.
     * 
     * @note
     *      O(n) time complexity.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      bool has = list.contains(2);  // true
     *      bool has2 = list.contains(5); // false
     */
    bool contains(const E& e) const {
        return index_of(e) != npos();
    }
    
    /**
     * Removes the first occurrence of an element.
     * 
     * @param e
     *      Element to remove.
     * 
     * @return
     *      true if removed, false if not found.
     * 
     * @note
     *      O(n) time complexity due to shifting elements.
     *      If you need fast removal, use fast_remove_at().
     * 
     * @example
     *      array_list<int> list = {1, 2, 3, 2, 1};
     *      bool removed = list.remove(2);  // true, list = {1, 3, 2, 1}
     *      bool removed2 = list.remove(5); // false
     */
    bool remove(const E& e);
    
    /**
     * Removes the element at the specified position.
     * 
     * @param idx
     *      Position of the element to remove.
     * 
     * @return
     *      true on success.
     * 
     * @throws index_out_of_bound_exception
     *      If idx >= size().
     * 
     * @note
     *      All elements after idx are shifted left by one position.
     *      O(n) time complexity.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3, 4, 5};
     *      list.remove_at(2);  // old_value = 3, list = {1, 2, 4, 5}
     */
    bool remove_at(std::size_t idx);
    
    /**
     * Removes at position by swapping with the last element.
     * 
     * Faster than remove_at as it swaps the element with the last
     * element and removes the last. Order of elements is not preserved.
     * 
     * @param idx
     *      Position of the element to remove.
     * 
     * @return
     *      true on success.
     * 
     * @throws index_out_of_bound_exception
     *      If idx >= size().
     * 
     * @note
     *      O(1) time complexity.
     *      Does not preserve the order of remaining elements.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3, 4, 5};
     *      list.fast_remove_at(1);  // list = {1, 5, 3, 4}
     */
    bool fast_remove_at(std::size_t idx);
    
    /**
     * Sets the element at the specified position.
     * 
     * @tparam _E
     *      Element type (deduced from argument).
     * 
     * @param idx
     *      Position to set.
     * 
     * @param e
     *      New element value.
     * 
     * @return
     *      true on success.
     * 
     * @throws index_out_of_bound_exception
     *      If idx >= size().
     * 
     * @note
     *      The old value is moved out if ret_old_value is provided.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      list.set(1, 99);  // list = {1, 99, 3}
     */
    template<typename _E>
    bool set(std::size_t idx, _E&& e);
    
    /**
     * Accesses element at index (const).
     * 
     * @param idx
     *      Position to access.
     * 
     * @return
     *      Reference to the element.
     * 
     * @throws index_out_of_bound_exception
     *      If idx >= size().
     * 
     * @example
     *      const array_list<int>& list = get_list();
     *      int value = list.at(0);
     */
    E& at(std::size_t idx) const;
    
    /**
     * Returns the current size.
     * 
     * @return
     *      Number of elements in the list.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      size_t sz = list.size();  // 3
     */
    std::size_t size() const;
    
    /**
     * Reserves capacity for future growth.
     * 
     * @param new_capacity
     *      New capacity.
     * 
     * @throws out_of_memory_error
     *      If allocation fails.
     * 
     * @note
     *      If new_capacity <= current capacity, no action is taken.
     *      Existing elements are moved to the new buffer.
     * 
     * @example
     *      array_list<int> list;
     *      list.reserve(100);  // pre-allocate space for 100 elements
     *      for (int i = 0; i < 100; ++i) {
     *          list.add(i);  // no reallocations
     *      }
     */
    void reserve(std::size_t new_capacity);
    
    /**
     * Clears all elements.
     * 
     * Destroys all elements but keeps capacity.
     * 
     * @note
     *      O(n) time complexity where n is the number of elements.
     *      Memory is not deallocated, allowing efficient reuse.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      list.clear();  // list is empty, capacity remains
     *      assert(list.is_empty());
     */
    void clear();
    
    /**
     * Trims capacity to fit the current size.
     * 
     * Reduces memory usage by reallocating to exactly fit size.
     * 
     * @throws out_of_memory_error
     *      If allocation fails.
     * 
     * @note
     *      If size == capacity, no action is taken.
     *      Elements are moved to the new buffer.
     * 
     * @example
     *      array_list<int> list;
     *      list.reserve(100);
     *      // Add 10 elements...
     *      list.trim_to_size();  // capacity becomes 10
     */
    void trim_to_size();
    
    /**
     * Checks if the list is empty.
     * 
     * @return
     *      true if empty, false otherwise.
     * 
     * @example
     *      array_list<int> list;
     *      bool empty = list.is_empty();  // true
     *      list.add(1);
     *      empty = list.is_empty();  // false
     */
    bool is_empty() const;
    
    /**
     * Compares this list with another for equality.
     * 
     * @param other
     *      The list to compare with.
     * 
     * @return
     *      true if equal, false otherwise.
     * 
     * @note
     *      Two lists are equal if they have the same size and
     *      all corresponding elements are equal (using equal_to<E>).
     *      The allocator is not considered in the comparison.
     * 
     * @example
     *      array_list<int> a = {1, 2, 3};
     *      array_list<int> b = {1, 2, 3};
     *      array_list<int> c = {3, 2, 1};
     *      a.equals(b);  // true
     *      a.equals(c);  // false
     */
    bool equals(const array_list<E>& other) const;
    
    /**
     * Computes a hash code for the list.
     * 
     * @return
     *      Hash code of all elements.
     * 
     * @note
     *      Uses hash_for<E> for each element.
     *      Empty lists return 0.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      size_t h = list.hashcode();
     */
    std::size_t hashcode() const;
    
    /**
     * Returns the allocator used by this list.
     * 
     * @return
     *      Pointer to the allocator.
     * 
     * @example
     *      tca::allocator* alloc = list.get_allocator();
     */
    tca::allocator* get_allocator() const {
        return m_allocator;
    }

    /**
     * Returns pointer to the underlying data.
     * 
     * @return
     *      Pointer to the first element, or nullptr if empty.
     * 
     * @note
     *      The pointer is valid for the lifetime of the list.
     *      Modifying the data through this pointer is allowed.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      int* ptr = list.data();
     *      ptr[1] = 100;  // modifies list[1]
     */
    E* data() {
        return m_data;
    }

    /**
     * Returns const pointer to the underlying data.
     * 
     * @return
     *      Const pointer to the first element, or nullptr if empty.
     * 
     * @example
     *      const array_list<int>& list = get_list();
     *      const int* ptr = list.data();
     */
    const E* data() const {
        return m_data;   
    }

    /**
     * Returns a const iterator to the beginning.
     * 
     * @return
     *      Const iterator pointing to the first element.
     * 
     * @example
     *      const array_list<int>& list = get_list();
     *      for (auto it = list.begin(); it != list.end(); ++it) {
     *          std::cout << *it << " ";
     *      }
     */
    list::ptr_iterator<const E> begin() const {
        return list::ptr_iterator<const E>(m_data, m_data + m_size);
    }

    /**
     * Returns a const iterator to the end.
     * 
     * @return
     *      Const iterator pointing to one past the last element.
     */
    list::ptr_iterator<const E> end() const {
        return list::ptr_iterator<const E>(m_data + m_size, m_data + m_size);
    }

    /**
     * Returns an iterator to the beginning.
     * 
     * @return
     *      Iterator pointing to the first element.
     * 
     * @example
     *      array_list<int> list = {1, 2, 3};
     *      for (auto it = list.begin(); it != list.end(); ++it) {
     *          *it *= 2;  // doubles all elements
     *      }
     */
    list::ptr_iterator<E> begin() {
        return list::ptr_iterator<E>(m_data, m_data + m_size);
    }

    /**
     * Returns an iterator to the end.
     * 
     * @return
     *      Iterator pointing to one past the last element.
     */
    list::ptr_iterator<E> end() {
        return list::ptr_iterator<E>(m_data + m_size, m_data + m_size);
    }

    /**
    * Index access operator.
    *
    * Returns a reference to the element at the given index.
    *
    * @param idx
    *       The element's index.
    *
    * @throws index_out_of_bound_exception
    *       If idx >= size()
    */
    E& operator[] (std::size_t idx) {
        return at(idx);
    }
    
    /**
    * Index access operator.
    *
    * Returns a constant reference to the element at the given index.
    *
    * @param idx
    *       The element index.
    *
    * @throws index_out_of_bound_exception
    *       If idx >= size()
    */
    const E& operator[] (std::size_t idx) const {
        return at(idx);
    }

};

    template<typename E>
    array_list<E>::array_list(tca::allocator* allocator) : m_allocator(allocator), m_data(nullptr), m_capacity(0), m_size(0) {

    }

    template<typename E>
    array_list<E>::array_list(std::size_t init_capacity, tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_data(nullptr), 
    m_capacity(0), 
    m_size(0) {
        if (init_capacity > 0)
            reserve(init_capacity);
    }

    template<typename E>
    array_list<E>::array_list(std::initializer_list<E> init_list, tca::allocator* allocator) : array_list<E>(allocator) {
        
        if (init_list.size() > 0) {
            reserve(init_list.size());
        }

        try {
            for (const E& e : init_list)
                add(e);
        } catch (...) {
            cleanup();
            throw;
        }

    }

    template<typename E>
    array_list<E>::array_list(const array_list<E>& list) : array_list<E>(list.get_allocator()) {
        
        if (list.size() > 0)
        {
            Evalue* data = allocate_and_copy_n<Evalue>(list.data(), list.size(), m_allocator);
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
            
            m_data      = data;
            m_capacity  = list.size();
            m_size      = list.size();
        }

    }
    
    template<typename E>
    array_list<E>::array_list(array_list<E>&& list) : 
    m_allocator(list.m_allocator),
    m_data(list.m_data),
    m_capacity(list.m_capacity),
    m_size(list.m_size) {
        list.m_data         = nullptr;
        list.m_capacity     = 0;
        list.m_size         = 0;
    }
    
    template<typename E>
    array_list<E>& array_list<E>::operator= (const array_list<E>& list) {
        if (&list != this)
        {
            Evalue* data = allocate_and_copy_n<Evalue>(list.data(), list.size(), m_allocator);
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
            
            assert(m_allocator != nullptr);
            deallocate_and_destroy_n(m_data, m_size, m_allocator);
            
            m_data      = data;
            m_capacity  = list.size();
            m_size      = list.size();
        }
        return *this;
    }
    

    template<typename E>
    array_list<E>& array_list<E>::operator= (array_list<E>&& list) {
        if (&list == this)
            return *this;
        if (get_allocator() == list.get_allocator())
        {
            std::swap(m_data,       list.m_data);
            std::swap(m_capacity,   list.m_capacity);
            std::swap(m_size,       list.m_size);
        }
        else
        {
            *this = list;
        }
        return *this;
    }

    template<typename E>
    template<typename _E>
    bool array_list<E>::set(std::size_t idx, _E&& e) {
        JSTD_DEBUG_CODE(check_index(idx, m_size));
        static_assert(!is_const<E>::value, "Cannot modified const type");
        m_data[idx] = std::forward<_E>(e);
        return true;
    }

    template<typename E>
    void array_list<E>::grow() {
        const std::size_t new_capacity = m_capacity > 0 ? m_capacity + (m_capacity >> 1) : DEFAULT_CAPACITY;
        reserve(new_capacity);
    }

    template<typename E>
    std::size_t array_list<E>::size() const {
        return m_size;
    }

    template<typename E>
    void array_list<E>::trim_to_size() {
        assert(m_allocator != nullptr);
        if (m_capacity == m_size)
            return;
        Evalue* new_data = allocate_and_move_n(m_data, m_size, m_allocator);
        if (!new_data)
            throw_except<out_of_memory_error>("Out of memory");
        deallocate_and_destroy_n(m_data, m_size, m_allocator);
        m_data      = new_data;
        m_capacity  = m_size;
    }

    template<typename E>
    E& array_list<E>::at(std::size_t idx) const {
        JSTD_DEBUG_CODE(check_index(idx, m_size));
        return m_data[idx];
    }
    
    template<typename E>
    void array_list<E>::clear() {
        destroy_n(m_data, m_size);
        m_size = 0;
    }

    template<typename E>
    std::size_t array_list<E>::last_index_of(const E& e) const {
        equal_to<E> equals;
        for (std::size_t i = m_size; i > 0; --i)
            if (equals(e, m_data[i - 1]))
                return i - 1;
        return npos();
    }

    template<typename E>
    std::size_t array_list<E>::index_of(const E& e) const {
        equal_to<E> equals;
        for (std::size_t i = 0; i < m_size; ++i)
            if (equals(e, m_data[i]))
                return i;
        return npos();
    }
    
    template<typename E>
    bool array_list<E>::remove(const E& e) {
        std::size_t finded_index = index_of(e);
        if (finded_index == npos())
            return false;
        return remove_at(finded_index);
    }
    
    template<typename E>
    bool array_list<E>::remove_at(std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, m_size));

        m_data[idx].~Evalue();

        for (std::size_t i = idx; i < m_size - 1; ++i)
        {
            new(&m_data[i]) Evalue(std::move(m_data[i + 1]));
            m_data[i + 1].~Evalue();
        }

        --m_size;
        
        return true;
    }
    
    template<typename E>
    bool array_list<E>::fast_remove_at(std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, m_size));
    
        // Call destructor for removal element
        m_data[idx].~Evalue();
    
        if (idx != m_size - 1)
        {
            // Move last element to current
            new(&m_data[idx]) Evalue(std::move(m_data[m_size - 1]));
            
            // Destroy last element
            m_data[m_size - 1].~Evalue();
        }

        --m_size;
        
        return true;
    }

    template<typename E>
    template<typename _E>
    void array_list<E>::add(_E&& e) {
        if (m_size >= m_capacity) 
            grow();
        new (&m_data[m_size]) Evalue(std::forward<_E>(e));
        ++m_size;
    }

    template<typename E>
    template<typename _E>
    void array_list<E>::add(std::size_t idx, _E&& e) {
        JSTD_DEBUG_CODE(check_index(idx, m_size + 1););
        
        if (m_size >= m_capacity)
        {
            grow();
        }

        for (std::size_t i = m_size; i > idx; --i)
        {
            new (&m_data[i]) Evalue(std::move(m_data[i - 1]));
            m_data[i - 1].~Evalue();
        }

        try {
            new (&m_data[idx]) Evalue(std::forward<_E>(e));
        } catch (...)
        {
            //Rollback
            for (std::size_t i = idx; i < m_size; ++i)
            {
                new (&m_data[i]) Evalue(std::move(m_data[i + 1]));
                m_data[i + 1].~Evalue();
            }
            throw;
        }

        ++m_size;
    }

    template<typename E>
    bool array_list<E>::is_empty() const {
        return m_size == 0;
    }

    template<typename E>
    void array_list<E>::reserve(std::size_t new_capacity) {
        assert(m_allocator != nullptr);

        if (new_capacity <= m_capacity)
            return;
        
        Evalue* new_data = (Evalue*) m_allocator->allocate_align(sizeof(E) * new_capacity, alignof(E));
        if(!new_data)
            throw_except<out_of_memory_error>("Out of memory");

        try {
            uninitialized_move_n(new_data, m_data, m_size);
        } catch (...) {
            m_allocator->deallocate(new_data);
            throw;
        }
        deallocate_and_destroy_n(m_data, m_size, m_allocator);

        m_data     = new_data;
        m_capacity = new_capacity;
    }

    template<typename E>
    void array_list<E>::cleanup() {
        assert(m_allocator != nullptr);
        deallocate_and_destroy_n(m_data, m_size, m_allocator);
        
        m_data      = nullptr;
        m_capacity  = 0;
        m_size      = 0;
    }

    template<typename E>
    bool array_list<E>::equals(const array_list<E>& other) const {
        if (size() != other.size())
            return false;
        if (data() && other.data())
        {
            return objects::equals(begin(), end(), other.begin(), other.end(), equal_to<E>());
        }
        else
        {
            return data() == other.data();
        }
    }

    template<typename E>
    std::size_t array_list<E>::hashcode() const {
        if (size() == 0)
            return 0;
        return objects::hashcode(begin(), end(), hash_for<E>());
    }

    template<typename E>
    array_list<E>::~array_list() {
        cleanup();
    }
}
#endif//JSTD_CPP_LANG_UTILS_ARRAY_LIST_H