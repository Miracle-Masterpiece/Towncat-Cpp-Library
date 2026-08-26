#ifndef E48BE300_4482_4C64_BC3D_90198EE230EC
#define E48BE300_4482_4C64_BC3D_90198EE230EC

namespace tc
{

/**
 * 
 */
template<bool expr, typename T = void>
struct enable_if {};

/**
 * 
 */
template<typename T>
struct enable_if<true, T> {
    typedef T type;
};

template<bool Cond, typename TTrue, typename TFalse>
struct select_if {};

template<typename TTrue, typename TFalse>
struct select_if<true, TTrue, TFalse> {
    typedef TTrue type;
};

template<typename TTrue, typename TFalse>
struct select_if<false, TTrue, TFalse> {
    typedef TFalse type;
};

}

#endif /* E48BE300_4482_4C64_BC3D_90198EE230EC */
