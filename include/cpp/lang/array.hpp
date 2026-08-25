#ifndef JSTDLIB_CPP_LANG_ARRAY_H_
#define JSTDLIB_CPP_LANG_ARRAY_H_

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/traits/cv_traits.hpp>
#include <initializer_list>
#include <utility>
#include <new>

namespace tc
{
    
/**
 * Dynamic array container with allocator support.
 * 
 * A dynamically-sized array that manages its own memory using a custom allocator.
 * Provides value semantics with copy and move operations, iterator support,
 * and bounds-checked element access.
 * 
 * 
 * @tparam T
 *      The type of elements stored in the array.
 * 
 * @note
 *      The array stores elements contiguously in memory.
 *      Empty arrays have data() == nullptr and length == 0.
 * 
 * @note
 *      The allocator is stored and used for all memory operations.
 *      Copy operations use the same allocator as the source.
 * 
 * @example
 *      // Create array of 10 integers
 *      array<int> arr(10);
 *      arr[0] = 42;
 *      arr[5] = 100;
 * 
 *      // Create from initializer list
 *      array<int> arr2 = {1, 2, 3, 4, 5};
 * 
 *      // Iterate
 *      for (int& i : arr) {
 *          i *= 2;
 *      }
 */
template<typename T>
class array {
protected:
    
    typedef typename remove_cv<T>::type Tvalue;

    tca::allocator* const _allocator;
    Tvalue* _data;
 
public:

    std::size_t length;
    
    /**
     * Default constructor.
     * 
     * Constructs an empty array.
     * 
     * @param alloc
     *      Allocator to use for memory management.
     *      If not provided, uses the global default allocator.
     * 
     * @note
     *      The allocator is stored and used for all subsequent memory operations.
     * 
     * @example
     *      array<int> arr;  // empty array
     *      assert(arr.length == 0);
     *      assert(arr.data() == nullptr);
     */
    array(tca::allocator* alloc = tca::get_default_allocator());
    
    /**
     * Constructs an array with specified size.
     * 
     * Allocates memory for sz elements and default-initializes them.
     * 
     * @param sz
     *      Number of elements.
     * 
     * @param allocator
     *      Allocator to use for memory management.
     *      If not provided, uses the global default allocator.
     * 
     * @throws out_of_memory_error
     *      if memory allocation fails.
     * 
     * @throws 
     *      Any exception thrown by T's default constructor.
     * 
     * @note
     *      If sz == 0, no memory is allocated and data() returns nullptr.
     * 
     * @example
     *      array<std::string> arr(10);  // 10 default-constructed strings
     */
    array(std::size_t sz, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Constructs an array from an initializer list.
     * 
     * @param init_list
     *      Initializer list of elements.
     * 
     * @param allocator
     *      Allocator to use for memory management.
     *      If not provided, uses the global default allocator.
     * 
     * @throws out_of_memory_error
     *      if memory allocation fails.
     * 
     * @throws
     *      Any exception thrown by T's copy constructor.
     * 
     * @example
     *      array<int> arr = {1, 2, 3, 4, 5};
     *      assert(arr.length == 5);
     */
    array(const std::initializer_list<T>& init_list, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Copy constructor.
     * Creates a copy of the array.
     * 
     * The allocator of the copied object is used for the newly created array.
     * 
     * If the length of the source array is 0, only the allocator is copied.
     * No memory is allocated for the array, and the data() function returns nullptr.
     * 
     * @throws out_of_memory_error
     *      If there is insufficient memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example 
     *      tc::array<int> ints = {1, 2, 3, 4};
     *      tc::array<int> copied = ints;
     *      
     *      for (int& i : copied) {
     *          std::cout << i << "\n";
     *      }
     * 
     *      assert(copied.length == ints.length);
     *      assert(copied.get_allocator() == ints.get_allocator());
     * 
     * @example 
     *      tc::array<int> ints;
     *      tc::array<int> copied = ints;
     *      assert(copied.data() == nullptr);
     *      assert(copied.length == 0);
     *      assert(copied.get_allocator() == ints.get_allocator());
     */
    array(const array<T>& a);

    /**
     * Move constructor.
     * 
     * Transfers the allocated memory, size, and allocator to the current object.
     * 
     * After the move, 'a' is guaranteed to retain its allocator.
     * 
     * @example
     *      tc::array<int> ints = {0, 1, 2, 3, 4};
     *      tc::array<int> moved = std::move(ints);
     *      
     *      assert(moved.length == 5);
     *      assert(moved.get_allocator() == ints.get_allocator());
     */
    array(array<T>&& a);
    
    /**
     * Copies elements from object 'a' into this object.
     * 
     * Provides the strong exception guarantee.
     * If an exception is thrown during memory allocation or element copying,
     * the current object remains unchanged.
     * 
     * The allocator of the current object is not modified during copying.
     * The allocator of the current object is used to create the copy.
     * 
     * After a successful copy, the old contents of the current object
     * are deallocated and replaced with a copy of the contents of 'a'.
     * 
     * If array 'a' has a length of 0,
     * then after copying this object will have a length of 0, and data() will return nullptr.
     * 
     * @throws out_of_memory_error
     *      If there is insufficient memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::array<int>  ints = {0, 1, 2, 3, 4};
     *      tc::array<int> copied;
     *      copied = ints;
     * 
     *      for (int& i : copied) {
     *          std::cout << i << "\n";
     *      }
     * 
     *      assert(copied.length == ints.length);
     */
    array<T>& operator=(const array<T>& a);
    
    /**
     * Moves data from object 'a' into this object.
     * The move operation provides the strong exception guarantee.
     * If an exception is thrown during the move, both objects remain unchanged.
     * 
     * The allocator of the current object is not modified during the move.
     * 
     * If the allocator of the current object and that of 'a' are equal,
     * the memory is transferred to the current object without copying.
     * 
     * If the allocators are not equal, a copy is performed instead.
     * 
     * @throws out_of_memory_error
     *      If there is insufficient memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::array<int> ints = {0, 1, 2, 3, 4};
     *      
     *      tc::array<int> moved;
     *      tc::allocator* alloc = moved.get_allocator(); 
     * 
     *      moved = std::move(ints);
     * 
     *      assert(moved.length == 5);
     *      assert(moved.get_allocator() == alloc);
     */
    array<T>& operator=(array<T>&& a);

    /**
     * Destroys all elements and deallocates memory.
     */
    ~array();
    
    /**
     * Accesses element at specified index with bounds checking.
     * 
     * @param idx
     *      Index of the element (0-based).
     * 
     * @return
     *      Reference to the element.
     * 
     * @throws index_out_of_bound_exception if
     *      idx >= length.
     * 
     * @warning
     *      The caller must ensure the array is not empty.
     * 
     * @example
     *      array<int> arr = {1, 2, 3};
     *      int x = arr[0];  // x = 1
     *      arr[0] = 100;
     *      // arr[100] would throw an exception
     */
    T& operator[](std::size_t idx);
    
    /**
     * Accesses element at specified index with bounds checking (const).
     * 
     * @param idx
     *      Index of the element (0-based).
     * 
     * @return
     *      Const reference to the element.
     * 
     * @throws index_out_of_bound_exception
     *      if idx >= length.
     * 
     * @example
     *      const array<int>& arr = get_array();
     *      int x = arr[0];  // read-only access
     */
    const T& operator[](std::size_t idx) const;
    
    /**
     * Returns pointer to the underlying data.
     * 
     * @return Pointer to the first element, or nullptr if the array is empty.
     * 
     * @note
     *      The pointer is valid for the lifetime of the array.
     *      Modifying the data through this pointer is allowed.
     * 
     * @example
     *      array<int> arr = {1, 2, 3};
     *      int* ptr = arr.data();
     *      ptr[1] = 100;  // modifies arr[1]
     */
    T* data() const;
    
    /**
     * Sets all elements to the given value.
     * 
     * @param value
     *      The value to assign to all elements.
     * 
     * @note
     *      If the array is empty, this function does nothing.
     * 
     * @example
     *      array<int> arr(5);
     *      arr.set(42);  // all elements become 42
     */
    void set(const T& value);
    
    /**
     * Returns the allocator used by this array.
     * 
     * @return
     *      Pointer to the allocator.
     * 
     * @note
     *      The allocator is used for all memory operations.
     * 
     * @example
     *      array<int> arr(10, my_allocator);
     *      tca::allocator* alloc = arr.get_allocator();
     */
    tca::allocator* get_allocator() const;
    
    /**
     * Computes a hash code for the array.
     * 
     * Computes a hash code by combining the hash codes of all elements.
     * 
     * @return
     *      Hash code of the array's contents, or 0 if the array is empty.
     * 
     * @note
     *      The hash function uses hash_for<T> for each element.
     *      Two equal arrays always produce the same hash code.
     * 
     * @example
     *      array<int> arr = {1, 2, 3};
     *      std::size_t h = arr.hashcode();
     */
    std::size_t hashcode() const;
    
    /**
     * Compares this array with another for equality.
     * 
     * Two arrays are equal if:
     *      They have the same length
     *      All corresponding elements are equal (using equal_to<T>)
     * 
     * @param a
     *      The array to compare with.
     * 
     * @return
     *      true if arrays are equal, false otherwise.
     * 
     * 
     * @example
     *      array<int> a = {1, 2, 3};
     *      array<int> b = {1, 2, 3};
     *      array<int> c = {3, 2, 1};
     *      a.equals(b);  // true
     *      a.equals(c);  // false
     */
    bool equals(const array<T>& a) const;
    
    /**
     * Returns a const iterator to the beginning.
     * 
     * @return
     *      Const pointer to the first element, or nullptr if empty.
     * 
     * @note
     *      The iterator is a simple pointer, not a class.
     *      Provides STL-style iteration support.
     */
    const T* begin() const;
    
    /**
     * Returns a const iterator to the end.
     * 
     * @return
     *      Const pointer to one past the last element.
     * 
     * @note
     *      For empty arrays, begin() == end().
     *      The end iterator should not be dereferenced.
     */
    const T* end() const;
    
    /**
     * Returns an iterator to the beginning.
     * 
     * @return
     *      Pointer to the first element, or nullptr if empty.
     * 
     * @note
     *      The iterator is a simple pointer, not a class.
     *      Provides STL-style iteration support.
     */
    T* begin();
    
    /**
     * Returns an iterator to the end.
     * 
     * @return
     *      Pointer to one past the last element.
     * 
     * @note
     *      For empty arrays, begin() == end().
     *      The end iterator should not be dereferenced.
     */
    T* end();
};

    template<typename T>
    array<T>::array(tca::allocator* alloc) : _allocator(alloc), _data(nullptr), length(0) {}

    template<typename T>
    array<T>::array(std::size_t sz, tca::allocator* allocator) : array<T>(allocator) {
        if (sz > 0)
        {
            _data  = allocate_and_initialize_n<Tvalue>(sz, allocator);
            if (!_data)
                throw_except<out_of_memory_error>("Out of memory");
            length = sz;
        }
    }

    template<typename T>
    array<T>::array(const std::initializer_list<T>& init_list, tca::allocator* allocator) : _allocator(allocator) {
        JSTD_DEBUG_CODE(check_non_null(allocator));
        
        Tvalue* data = nullptr;
        std::size_t sz = init_list.size();

        if (sz > 0)
        {
            data = allocate_and_copy_n<Tvalue>(init_list, _allocator);
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
        }

        _data       = data;
        length      = sz;  
    }

    template<typename T>
    array<T>::array(const array<T>& a) : array<T>(a._allocator) {
    
        Tvalue* data    = nullptr;
        std::size_t len = a.length;

        if (len > 0)
        {
        
            data = allocate_and_copy_n<Tvalue>(a.data(), a.length, _allocator); 
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
        }
        
        _data   = data;
        length  = len;
    }
    
    template<typename T>
    array<T>::array(array<T>&& a) : _allocator(a._allocator), _data(a._data), length(a.length) {
        a._data         = nullptr;
        a.length        = 0;
    }
    
    template<typename T>
    array<T>& array<T>::operator= (const array<T>& a) {
        if (&a == this)
            return *this;

        std::size_t new_len = a.length;
        Tvalue* new_data    = nullptr;
        if (a.length > 0)
        {
            new_data = allocate_and_copy_n<Tvalue>(a.data(), new_len, _allocator);
            if (!new_data)
                throw_except<out_of_memory_error>("Out of memory");
        }
        
        deallocate_and_destroy_n(_data, length, _allocator);

        _data   = new_data;
        length  = new_len;
        
        return *this;
    }
    
    template<typename T>
    array<T>& array<T>::operator= (array<T>&& a) {
        if (&a == this)
            return *this;
        if (get_allocator() == a.get_allocator())
        {
            std::swap(_data,        a._data);
            std::swap(length,       a.length);
        }
        else
        {
            *this = a;
        }
        return *this;
    }
    
    template<typename T>
    array<T>::~array() {
        deallocate_and_destroy_n(_data, length, _allocator);
    }

    template<typename T>
    T& array<T>::operator[] (std::size_t idx) {
        check_index(idx, length);
        return _data[idx];
    }
    
    template<typename T>
    const T& array<T>::operator[] (std::size_t idx) const {
        check_index(idx, length);
        return _data[idx];
    }

    template<typename T>
    T* array<T>::data() const {
        return _data;
    }

    template<typename T>
    void array<T>::set(const T& value) {
        for (std::size_t i = 0, len = length; i < len; ++i)
            _data[i] = value;
    }

    template<typename T>
    std::size_t array<T>::hashcode() const {
        return length > 0 ? objects::hashcode(begin(), end(), hash_for<T>()) : 0;
    }

    template<typename T>
    tca::allocator* array<T>::get_allocator() const {
        return _allocator;
    }

    template<typename T>
    bool array<T>::equals(const array<T>& a) const {
        if (length != a.length)
        {
            return false;
        }
        if (data() != nullptr && a.data() != nullptr)
        {
            return objects::equals(begin(), end(), a.begin(), a.end(), equal_to<T>());
        }
        else
        {
            return data() == a.data();
        }
    }

    template<typename T>
    const T* array<T>::begin() const {
        return _data;
    }
    
    template<typename T>
    const T* array<T>::end() const {
        return _data + length;
    }
    
    template<typename T>
    T* array<T>::begin() {
        return _data;
    }
    
    template<typename T>
    T* array<T>::end() {
        return _data + length;
    }
}
#endif//JSTDLIB_CPP_LANG_ARRAY_H_