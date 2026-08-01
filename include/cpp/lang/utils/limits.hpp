#ifndef JSTD_CPP_LANG_UTIL_LIMITS_H
#define JSTD_CPP_LANG_UTIL_LIMITS_H

#include <climits>
#include <cfloat>

namespace tc
{

template<typename T>
struct num_limits {
    
};

template<>
struct num_limits<float> {
    static constexpr float max() {
        return FLT_MAX;
    }
    static constexpr float min() {
        return FLT_MIN;
    }
    static constexpr float epsilon() {
        return FLT_EPSILON;
    }
};  

template<>
struct num_limits<double> {
    static constexpr double max() {
        return DBL_MAX;
    }
    static constexpr double min() {
        return DBL_MIN;
    }
    static constexpr double epsilon() {
        return DBL_EPSILON;
    }
};  

template<>
struct num_limits<long double> {
    static constexpr long double max() {
        return LDBL_MAX;
    }
    static constexpr long double min() {
        return LDBL_MIN;
    }
    static constexpr long double epsilon() {
        return LDBL_EPSILON;
    }
};  

template<>
struct num_limits<char> {
    static constexpr char max() {
        return CHAR_MAX;
    }
    static constexpr char min() {
        return CHAR_MIN;
    }
    static constexpr unsigned char bits() {
        return CHAR_BIT;
    }
};  

#define ___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(type)                                            \
template<>                                                                                  \
struct num_limits<type> {                                                                   \
    static constexpr type max() {                                                           \
        return (type)~((type) 1 << (sizeof(type) * (CHAR_BIT) - (type) 1));                 \
    }                                                                                       \
    static constexpr type min() {                                                           \
        return (type) ((type) 1 << (sizeof(type) * CHAR_BIT - 1));                          \
    }                                                                                       \
};                                                                                          \

___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(signed char)
___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(signed short)
___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(signed int)
___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(signed long)
___JSTD_MAKE_NUMERIC_LIMITS_SIGNED(signed long long)

#undef ___JSTD_MAKE_NUMERIC_LIMITS_SIGNED

#define ___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(type)\
template<>\
struct num_limits<type> {\
    static constexpr type max() {\
        return (type) ~((type) 0);\
    }\
    static constexpr type min() {\
        return 0;\
    }\
};\

___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(unsigned char)
___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(unsigned short)
___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(unsigned int)
___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(unsigned long)
___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED(unsigned long long)

#undef ___JSTD_MAKE_NUMERIC_LIMITS_UNSIGNED



}
#endif