#ifndef AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362
#define AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362

#include <cpp/lang/utils/cond_compile.hpp>

#ifndef NDEBUG
 #include <cpp/lang/exceptions.hpp>
#endif //NDEBUG

namespace tc
{

/**
 * A template class for representing an array.
 *
 * The class provides functions for safe access with bounds checking in the DEBUG build.
 */
template<typename T>
class array_view {
    /**
     * Pointer to an existing array.
     */
    T* m_data;
    
    /**
     * Array size.
     */
    std::size_t m_length;
public:

    /**
     * Initializes this view with default values.
     * The pointer is set to nullptr and the length is 0.
     */
    array_view() : m_data(nullptr), m_length(0) {}

    /**
     * Creates a view over an existing array of elements.
     *
     * @param p
     *      Pointer to the first element of the array.
     *
     * @param length
     *      Number of elements in the array.
     *
     * @example
     *      int ints[] = {1, 2, 3, 4}
     *      tc::array_view<int> arr(ints, sizeof(ints) / sizeof(*ints));
     *      std::cout << arr[2] << "\n"; //output 3
     */
    array_view(T* p, std::size_t length) : m_data(p), m_length(length) {}

    /**
     * @return
     *       Length of the view array.
     */
    std::size_t length() const {
        return m_length;
    }

    /**
     * Index access operator.
     *
     * @param idx
     *      Element index.
     *
     * @return
     *      Reference to the array element.
     *
     * @throws index_out_of_bound_exception (in DEBUG build)
     *      If idx >= length()
     *
     * @throws null_pointer_exception (in DEBUG build)
     *      If view does not contain a pointer to an array.
     */
    T& operator[] (std::size_t idx) {
        JSTD_DEBUG_CODE(
            check_non_null(m_data);
            check_index(idx, length());
        );
        return m_data[idx];
    }
    
    /** 
     * Index access operator. 
     * 
     * @param idx 
     *      Element index. 
     * 
     * @return 
     *      Constant reference to an array element. 
     * 
     * @throws index_out_of_bound_exception (in DEBUG build) 
     *      If idx >= length() 
     * 
     * @throws null_pointer_exception (in DEBUG build)
     *      If view does not contain a pointer to an array. 
     */
    const T& operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE(
            check_non_null(m_data);
            check_index(idx, length());
        );
        return m_data[idx];
    }
};

}

#endif /* AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362 */
