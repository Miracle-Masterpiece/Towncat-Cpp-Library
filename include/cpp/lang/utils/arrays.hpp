#ifndef JSTD_CPP_LANG_UTILS_ARRAYS_H
#define JSTD_CPP_LANG_UTILS_ARRAYS_H

#include <cpp/lang/traits/cv_traits.hpp>
#include <cctype>
#include <type_traits>
#include <cstring>
#include <initializer_list>
#include <cstdint>
#include <allocators/allocator.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <cpp/lang/common.hpp>

#ifndef NDEBUG
    #include <cpp/lang/exceptions.hpp>
#endif

namespace tc
{

    /**
     * Constructs a range of objects using default initialization.
     * 
     * Constructs N objects of type T in the uninitialized memory region
     * starting at array. Provides strong exception guarantee.
     * 
     * @tparam T
     *      The type of objects to construct.
     * 
     * @param array
     *      Pointer to uninitialized memory.
     * 
     * @param length
     *      Number of objects to construct.
     * 
     * @throws Any exception thrown by T's default constructor.
     * 
     * @note
     *      If an exception occurs, all successfully constructed objects 
     *      are destroyed before rethrowing.
     * 
     * @warning
     *      The memory region must be large enough for length objects
     *      and must be properly aligned for type T.
     */
    template<typename T>
    void uninitialized_construct_n(T* array, std::size_t length) {
        JSTD_DEBUG_CODE(check_non_null(array, "'array' is null"));

        std::size_t constructed = 0;
        try {
            for (;constructed < length; ++constructed)
                new (array + constructed) T();
        } catch (...) {
            while (constructed > 0)
                array[--constructed].~T();
            throw;
        }
    }

    /**
     * Allocates memory and default-initializes objects.
     * 
     * Allocates memory for n objects of type T and default-constructs
     * them in the allocated memory. Provides strong exception guarantee.
     * 
     * @tparam T
     *      The type of objects to allocate and initialize.
     * 
     * @param n
     *      Number of objects to allocate and construct.
     * 
     * @param alloc
     *      Allocator to use for memory allocation.
     * 
     * @return
     *      Pointer to the allocated and initialized memory,
     *      or nullptr if memory allocation fails.
     * 
     * @throws
     *      Any exception thrown by T's default constructor.
     * 
     * @note
     *      If an exception occurs during construction, the allocated
     *      memory is automatically deallocated before rethrowing.
     * 
     * @warning
     *      The caller is responsible for deallocating the memory using
     *      deallocate_and_destroy_n() or equivalent.
     * 
     * @example
     *      // Allocate and construct 10 strings
     *      tca::allocator* alloc = tca::get_default_allocator();
     *      std::string* arr = allocate_and_initialize_n<std::string>(10, alloc);
     *      if (arr) {
     *          // Use arr...
     *          deallocate_and_destroy_n(arr, 10, alloc);
     *      }
     */
    template<typename T>
    T* allocate_and_initialize_n(std::size_t n, tca::allocator* alloc) {
        JSTD_DEBUG_CODE(check_non_null(alloc, "'alloc' is null"));

        typedef typename remove_cv<T>::type Tvalue;

        Tvalue* data = (Tvalue*) alloc->allocate_align(sizeof(T) * n, alignof(T));
        if (!data)
            return nullptr;

        try {
            uninitialized_construct_n(data, n);
        } catch(...) {
            alloc->deallocate(data);
            throw;
        }

        return data;
    }

    /**
     * Destroys a range of objects.
     * 
     * Calls destructors for N objects of type T in the array.
     * 
     * @tparam T
     *      The type of objects to destroy.
     * 
     * @param array
     *      Pointer to the array of objects.
     * 
     * @param length
     *      Number of objects to destroy.
     * 
     * @note
     *      Safe to call with nullptr or length 0 (no-op).
     * 
     * @warning
     *      Objects must have been previously constructed.
     */
    template<typename T>
    void destroy_n(const T* array, std::size_t length) {
        if (length == 0)        return;
        
        JSTD_DEBUG_CODE(check_non_null(array, "'array' is null"));

        std::size_t destructs = length;
        while (destructs > 0)
        {
            array[--destructs].~T();
        }
    }

    /**
     * Copies a range of objects into uninitialized memory.
     * 
     * Copy-constructs length objects from src to dst in uninitialized memory.
     * Provides strong exception guarantee.
     * 
     * @tparam T
     *      The type of objects to copy.
     * 
     * @param dst
     *      Destination uninitialized memory.
     * 
     * @param src
     *      Source objects to copy from.
     * 
     * @param length
     *      Number of objects to copy.
     * 
     * @throws
     *      Any exception thrown by T's copy constructor.
     * 
     * @note
     *      If an exception occurs, all successfully constructed objects
     *      are destroyed before rethrowing.
     * 
     * @warning
     *      dst must be large enough for length objects.
     *      dst and src must not overlap.
     */
    template<typename T>
    void uninitialized_copy_n(T* dst, const T* src, std::size_t length) {
        if (!length) return;
        
        JSTD_DEBUG_CODE(check_non_null(dst, "'dst' is null"));
        JSTD_DEBUG_CODE(check_non_null(src, "'src' is null"));
        
        std::size_t copied = 0;
        try {
            for (;copied < length; ++copied)
                new (dst + copied) T(src[copied]);
        } catch (...) {
            while (copied > 0)
                dst[--copied].~T();
            throw;
        }
    }
    
    /**
     * Moves a range of objects into uninitialized memory.
     * 
     * Move-constructs length objects from src to dst in uninitialized memory.
     * Provides strong exception guarantee.
     * 
     * @tparam T
     *      The type of objects to move.
     * 
     * @param dst
     *      Destination uninitialized memory.
     * 
     * @param src
     *      Source objects to move from.
     * 
     * @param length
     *      Number of objects to move.
     * 
     * @throws
     *      Any exception thrown by T's move constructor.
     * 
     * @note
     *      If an exception occurs, all successfully constructed objects
     *      are destroyed before rethrowing.
     * 
     * @warning
     *      dst must be large enough for length objects.
     *      dst and src must not overlap.
     */
    template<typename T>
    void uninitialized_move_n(T* dst, T* src, std::size_t length) {
        if (!length) return;        
        
        JSTD_DEBUG_CODE(check_non_null(dst, "'dst' is null"));
        JSTD_DEBUG_CODE(check_non_null(src, "'src' is null"));
        
        std::size_t moved = 0;
        try {
            for (;moved < length; ++moved)
                new (dst + moved) T(std::move(src[moved]));
        } catch (...) {
            while (moved > 0)
                dst[--moved].~T();
            throw;
        }
    }

    /**
     * Allocates memory and moves objects into it.
     * 
     * Allocates memory for length objects of type T and moves them
     * from src to the newly allocated memory.
     * 
     * @tparam T
     *      The type of objects to move.
     * 
     * @param src
     *      Source objects to move from.
     * 
     * @param len
     *      Number of objects to move.
     * 
     * @param alloc
     *      Allocator to use for memory allocation.
     * 
     * @return
     *      Pointer to the newly allocated and populated memory,
     *      or nullptr if allocation fails.
     * 
     * @throws Any exception thrown by T's move constructor.
     * 
     * @note
     *      If an exception occurs during construction, memory is deallocated.
     */
    template<typename T>
    T* allocate_and_move_n(T* src, std::size_t len, tca::allocator* alloc) {
        
        typedef typename remove_cv<T>::type Tvalue;

        Tvalue* data = (Tvalue*) alloc->allocate_align(sizeof(T) * len, alignof(T));
        if (!data)
        {
            return nullptr;
        }

        try {
            uninitialized_move_n(data, src, len);
        } catch (...) {
            alloc->deallocate(data);
            throw;
        }
        
        return data;
    }
    
    /**
     * Allocates memory and copies objects into it.
     * 
     * Allocates memory for length objects of type T and copies them
     * from src to the newly allocated memory.
     * 
     * @tparam T
     *      The type of objects to copy.
     * 
     * @param src
     *      Source objects to copy from.
     * 
     * @param len
     *      Number of objects to copy.
     * 
     * @param alloc
     *      Allocator to use for memory allocation.
     * 
     * @return
     *      Pointer to the newly allocated and populated memory,
     *      or nullptr if allocation fails.
     * 
     * @throws Any exception thrown by T's copy constructor.
     * 
     * @note
     *      If an exception occurs during construction, memory is deallocated.
     */
    template<typename T>
    T* allocate_and_copy_n(const T* src, std::size_t len, tca::allocator* alloc) {
        JSTD_DEBUG_CODE(check_non_null(src, "'src' is null"));
        JSTD_DEBUG_CODE(check_non_null(alloc, "'alloc' is null"));

        typedef typename remove_cv<T>::type Tvalue;

        Tvalue* data = (Tvalue*) alloc->allocate_align(sizeof(T) * len, alignof(T));
        if (!data)
        {
            return nullptr;
        }

        try {
            uninitialized_copy_n(data, src, len);
        } catch (...) {
            alloc->deallocate(data);
            throw;
        }
        
        return data;
    }
    
    template<typename T, typename E>
    T* allocate_and_copy_n(std::initializer_list<E> src, tca::allocator* alloc) {
            
        typedef typename remove_cv<T>::type Tvalue;

        Tvalue* data = (Tvalue*) alloc->allocate_align(sizeof(T) * src.size(), alignof(T));
        if (!data)
        {
            return nullptr;
        }

        try {
            uninitialized_copy_n(data, src.begin(), src.size());
        } catch (...) {
            alloc->deallocate(data);
            throw;
        }
        
        return data;
    }
    
    /**
     * Destroys objects and deallocates memory.
     * 
     * Destroys all objects in the array and then deallocates the memory.
     * 
     * @tparam T
     *      The type of objects to destroy.
     * 
     * @param src
     *      Pointer to the array of objects.
     * 
     * @param len
     *      Number of objects in the array.
     * 
     * @param alloc
     *      Allocator to use for memory deallocation.
     * 
     * @note
     *      Safe to call with src == nullptr (no-op).
     * 
     * @warning
     *      Objects must have been previously constructed.
     */
    template<typename T>
    void deallocate_and_destroy_n(T* src, std::size_t len, tca::allocator* alloc) {
        JSTD_DEBUG_CODE(check_non_null(alloc, "'alloc' is null"));
        if (src)
        {
            while (len > 0)
                src[--len].~T();
            alloc->deallocate(const_cast<typename remove_cv<T>::type*>(src));
        }
    }

    /**
     * Copy-constructs objects from an initializer list.
     * 
     * Constructs objects of type T from elements in the initializer list.
     * Provides strong exception guarantee.
     * 
     * @tparam T
     *      The type of objects to construct.
     * 
     * @tparam E
     *      The element type (deduced).
     * 
     * @param array
     *      Destination uninitialized memory.
     * 
     * @param init_list
     *      Initializer list of elements to copy from.
     * 
     * @throws Any exception thrown by T's constructor.
     * 
     * @note
     *      If an exception occurs, all successfully constructed objects
     *      are destroyed before rethrowing.
     */
    template<typename T, typename E>
    void uninitialized_copy_n(T* array, std::initializer_list<E> init_list) {
        JSTD_DEBUG_CODE(check_non_null(array, "'array' is null"));
        std::size_t copied = 0;
        try {
            for (const E& e : init_list)
            {
                new(array + copied) T(e); 
                ++copied;
            }
        } catch (...) {
            std::size_t i = copied;
            while (i > 0)
                array[--i].~T();
            throw;
        }
    }

    template<typename T>
    void copy(T* dst, const T* src, std::size_t length) {
        JSTD_DEBUG_CODE(
            check_non_null(dst, "'dst' is null");
            check_non_null(src, "'src' is null");
        )
        for (std::size_t i = 0; i < length; ++i)
            dst[i] = src[i];
    }

    /**
     * Copies null-terminated string with buffer limit.
     * 
     * Copies a null-terminated string from src to dst, ensuring dst
     * is null-terminated and does not overflow the destination buffer.
     * 
     * @tparam T
     *      The character type.
     * 
     * @param dst
     *      Destination buffer.
     * 
     * @param src
     *      Source null-terminated string.
     * 
     * @param dst_max
     *      Maximum capacity of dst buffer (including null terminator).
     * 
     * @return
     *      Number of characters copied (excluding null terminator).
     * 
     * @note
     *      The destination is always null-terminated.
     *      If the source length >= dst_max - 1, the string is truncated.
     * 
     * @example
     *      char buf[10];
     *      size_t len = ncopy(buf, "hello world", 10);  // copies "hello wor"
     *      // buf = "hello wor", len = 9
     */
    template<typename T>
    std::size_t ncopy(T* dst, const T* src, std::size_t dst_max) {
        JSTD_DEBUG_CODE(
            check_non_null(src, "'src' is null");
            check_non_null(dst, "'dst' is null");
        )
        
        if (dst_max == 0)
            return 0;
        
        std::size_t i;
        for (i = 0; i < dst_max; ++i)
        {
            
            if (i == dst_max - 1)
            {
                dst[i] = 0;
                return i;
            }
            
            dst[i] = src[i];
            
            if (src[i] == 0)
            {
                break;
            }
        }

        return i;
    }

    /**
     * Sorts array using insertion sort algorithm
     * 
     * @tparam T
     *      Element type
     * 
     * @tparam
     *      T_COMPARATOR Comparator type (default: compare_to<T>)
     * 
     * @param array
     *      Pointer to array
     * 
     * @param len
     *      Number of elements
     * 
     * @example
     *      int arr[] = {5, 2, 8, 1, 9};
     *      intersect_sort(arr, 5);  // arr = {1, 2, 5, 8, 9}
     */
    template<typename T, typename T_COMPARATOR = compare_to<T>>
    void intersect_sort(T* array, std::size_t len) {
        if (len == 0)
            return;

        JSTD_DEBUG_CODE(
            check_non_null(array, "'array' is null");        
        );

        T_COMPARATOR compare;
        for (std::size_t i = 1; i < len; ++i) {
            std::size_t j = i;
            while (j > 0) {
                T& a = array[j - 1];
                T& b = array[j];
                int comp = compare(a, b);
                if (comp > 0) {
                    std::swap(a, b);
                    --j;
                    continue;
                }
                break;
            }
        }
    }

    /**
     * Searches for element in sorted array using binary search
     * 
     * @tparam E
     *      Element type
     * 
     * @tparam COMPARATOR_T
     *      Comparator type (default: compare_to<E>)
     * 
     * @param arr
     *      Pointer to sorted array
     * 
     * @param size
     *      Array size
     * 
     * @param searched
     *      Value to find
     * 
     * @return
     *      Index of found element, or npos() if not found
     * 
     * @note:
     *      Array MUST be sorted in ascending order
     * 
     * @example
     *      int arr[] = {1, 2, 5, 8, 9};
     *      size_t idx = binary_search(arr, 5, 8);   // idx = 3
     *      size_t not_found = binary_search(arr, 5, 10);  // = npos()
     */
    template<typename E, typename COMPARATOR_T = compare_to<E>>
    std::size_t binary_search(const E* arr, std::size_t size, const E& searched) {
        if (size == 0)
            return npos();
        
        COMPARATOR_T compare_to;
        std::size_t start   = 0;
        std::size_t end     = size - 1;
        while (start <= end)
        {
            
            const std::size_t mid       = (end - start) / 2 + start;
            const E& mid_value          = arr[mid];
            const int compare_result    = compare_to(searched, mid_value);
            
            if (compare_result == 0)
            {
                return mid;
            }
            else if (compare_result < 0)
            {
                end     = mid - 1;
            }
            else
            {
                start   = mid + 1;
            }
        }
        
        return npos();
    }

namespace internal
{
    template<typename T, typename T_COMPARATOR>
    void quick_sort_impl(T* array, std::size_t left, std::size_t right, T_COMPARATOR& compare) {
        
        if (left >= right) return;

        std::size_t i = left;
        std::size_t j = right;
        T& pivot = array[(left + right) / 2];

        while (i <= j)
        {
            while (compare(array[i], pivot) < 0) ++i;
            while (compare(array[j], pivot) > 0) --j;

            if (i <= j) {
                if (i != j) {
                    T tmp = std::move(array[i]);
                    array[i] = std::move(array[j]);
                    array[j] = std::move(tmp);
                }
                ++i;
                
                if (j == 0) break;
                --j;
            }
        }

        if (left < j)   quick_sort_impl(array, left, j, compare);
        if (i < right)  quick_sort_impl(array, i, right, compare);
    }
} //namespace internal

    /**
     * Sorts array using quick sort algorithm
     * 
     * @tparam T
     *      Element type
     * 
     * @tparam T_COMPARATOR
     *      Comparator type (default: compare_to<T>)
     * 
     * @param array
     *      Pointer to array
     * 
     * @param len
     *      Number of elements
     * 
     * @example
     *      int arr[] = {9, 2, 7, 1, 5, 3, 8, 4, 6};
     *      quick_sort(arr, 9);  // arr = {1, 2, 3, 4, 5, 6, 7, 8, 9}
     * 
     * @example
     * // Custom comparator for descending order
     * struct Descending {
     *     int operator()(int a, int b) const {
     *         return b - a;  // Reverse comparison
     *     }
     * };
     *      int arr[] = {1, 2, 3, 4, 5};
     *      quick_sort<int, Descending>(arr, 5);  // arr = {5, 4, 3, 2, 1}
     */
    template<typename T, typename T_COMPARATOR = compare_to<T>>
    void quick_sort(T* array, std::size_t len) {
        JSTD_DEBUG_CODE(
            if (array == nullptr)
                throw_except<null_pointer_exception>("array must be != null");
        )
        if (len <= 1) return;
        T_COMPARATOR compare;
        internal::quick_sort_impl(array, 0, len - 1, compare);
    }

}

#endif//JSTD_CPP_LANG_UTILS_ARRAYS_H