#ifndef EC3C7740_24AC_41AC_AC95_712E816FB1D5
#define EC3C7740_24AC_41AC_AC95_712E816FB1D5

namespace tc
{
    
template<typename T>
struct is_empty {
    static const bool value = __is_empty(T);
};

template<typename T>
struct is_final {
    static const bool value = __is_final(T);
};

template<typename T>
struct is_trivial {
    static const bool value = __is_trivial(T);
};

template<typename T>
struct is_array {
    static const bool value = false;
};

template<typename T>
struct is_array<T[]> {
    static const bool value = true;
};


} //namespace tc

#endif /* EC3C7740_24AC_41AC_AC95_712E816FB1D5 */
