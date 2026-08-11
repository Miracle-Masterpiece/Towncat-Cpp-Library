#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/numbers.hpp>
#include <cpp/lang/utils/comparator.hpp>

namespace tc 
{
namespace objects 
{

    #if 0
    template<>
    std::size_t hashcode<const float*>(const float* begin, const float* end, const hash_for<float>& hasher) {
        JSTD_DEBUG_CODE(
            check_non_null(begin, "begin == null");
            check_non_null(begin, "end == null");
        )

        std::size_t hash = 0xcbf29ce484222325;
        while (begin != end)
        {
            hash = (hash ^ (std::size_t) num::float_to_uint_bits(*begin)) * 0x100000001b3;
        }

        return hash;
    }

    template<>
    std::size_t hashcode<double>(const double* array, std::size_t len) {
#ifndef NDEBUG
        if (array == nullptr)
            throw_except<null_pointer_exception>("array must be != null");        
#endif//NDEBUG
        std::size_t hash = 0xcbf29ce484222325;
        for (std::size_t i = 0; i < len; ++i)
            hash = (hash ^ (std::size_t) num::double_to_uint_bits(array[i])) * 0x100000001b3;
        return hash;
    }
    #endif

}// namespace objects
}// namespace jstd 