#ifndef JSTD_CPP_LANG_UTILS_OBJECTS_H
#define JSTD_CPP_LANG_UTILS_OBJECTS_H

#include <cstdint>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/cond_compile.hpp>

namespace tc
{

class null_pointer_exception;
class illegal_argument_exception;

template<typename T>
void throw_except(const char* format, ...);

namespace objects
{

    template<typename IT, typename HASH_FOR>
    std::size_t hashcode(IT begin, IT end, const HASH_FOR& hash_for) {
        std::size_t hash = (std::size_t) 0xcbf29ce484222325;
        while (begin != end)
        {
            hash = (hash ^ hash_for(*begin)) * 0x100000001b3;
            ++begin;
        }
        return hash;
    }

    // template<>
    // std::size_t hashcode<const float*, hash_for<float>>(const float* begin, const float* end, const hash_for<float>& hasher);

    // /**
    //  * @see 
    //  *      template<typename T>
    //  *      std::size_t hashcode(const T* array, int64_t len);
    //  */
    // template<>
    // std::size_t hashcode<const double*>(const double* begin, const double* end, const hash_for<double>& hasher);

    template<typename IT>
    bool equals(IT b0, IT e0, IT b1, IT e1) {

        while (b0 != e0 && b1 != e1)
        {
            if (!(*b0 == *b1))
                return false;
            ++b0;
            ++b1;
        }
        
        return (b0 == e0) && (b1 == e1);
    }

    template<typename IT, typename EQUAL_TO>
    bool equals(IT b0, IT e0, IT b1, IT e1, const EQUAL_TO& equals_to) {
    
        while (b0 != e0 && b1 != e1)
        {
            if (!equals_to(*b0, *b1))
                return false;
            ++b0;
            ++b1;
        }
        
        return (b0 == e0) && (b1 == e1);
    }
}//namespace objects

}//namespace jstd

#endif//JSTD_CPP_LANG_UTILS_OBJECTS_H