#ifndef JSTD_CPP_LANG_UTILS_COMPARATOR_H
#define JSTD_CPP_LANG_UTILS_COMPARATOR_H

namespace tc {

template<typename T>
struct compare_to {
    int operator() (const T& a, const T& b) const;
};

#define MAKE_PRIMITIVE_COMPARE_FUNCS(type_name)                                             \
                        template<>                                                          \
                        struct compare_to<type_name> {                                      \
                            int operator() (const type_name& a, const type_name& b) const { \
                                if (a < b)                                                  \
                                    return -1;                                              \
                                else if (a > b)                                             \
                                    return 1;                                               \
                                else                                                        \
                                    return 0;                                               \
                            }                                                               \
                        };                                                                  \
                        template<>                                                          \
                        struct compare_to<const type_name> {                                \
                            int operator() (const type_name& a, const type_name& b) const { \
                                if (a < b)                                                  \
                                    return -1;                                              \
                                else if (a > b)                                             \
                                    return 1;                                               \
                                else                                                        \
                                    return 0;                                               \
                            }                                                               \
                        };                                                                  \

MAKE_PRIMITIVE_COMPARE_FUNCS(char)
MAKE_PRIMITIVE_COMPARE_FUNCS(unsigned char)
MAKE_PRIMITIVE_COMPARE_FUNCS(signed char)

MAKE_PRIMITIVE_COMPARE_FUNCS(wchar_t)

MAKE_PRIMITIVE_COMPARE_FUNCS(short int)
MAKE_PRIMITIVE_COMPARE_FUNCS(unsigned short int)

MAKE_PRIMITIVE_COMPARE_FUNCS(int)
MAKE_PRIMITIVE_COMPARE_FUNCS(unsigned int)

MAKE_PRIMITIVE_COMPARE_FUNCS(long int)
MAKE_PRIMITIVE_COMPARE_FUNCS(unsigned long int)

MAKE_PRIMITIVE_COMPARE_FUNCS(long long int)
MAKE_PRIMITIVE_COMPARE_FUNCS(unsigned long long int)

MAKE_PRIMITIVE_COMPARE_FUNCS(float)
MAKE_PRIMITIVE_COMPARE_FUNCS(double)
MAKE_PRIMITIVE_COMPARE_FUNCS(long double)

MAKE_PRIMITIVE_COMPARE_FUNCS(bool)

#undef MAKE_PRIMITIVE_COMPARE_FUNCS

}

#endif//JSTD_CPP_LANG_UTILS_COMPARATOR_H