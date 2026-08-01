#include <cpp/lang/numbers.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstring>

namespace tc 
{

namespace num 
{
    uint_float_bits float_to_uint_bits(float v) {
        static_assert(sizeof(v) == sizeof(uint_float_bits), "sizeof(v) == sizeof(uint_float_bits)");
        uint_float_bits value;
        std::memcpy(&value, &v, sizeof(float));
        return value;
    }
    
    uint_double_bits double_to_uint_bits(double v) {
        static_assert(sizeof(v) == sizeof(uint_double_bits), "sizeof(v) == sizeof(uint_double_bits)");
        uint_double_bits value;
        std::memcpy(&value, &v, sizeof(double));
        return value;
    }
}//namespace num

}//namespace jstd

