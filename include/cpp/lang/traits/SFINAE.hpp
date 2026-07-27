#ifndef E48BE300_4482_4C64_BC3D_90198EE230EC
#define E48BE300_4482_4C64_BC3D_90198EE230EC

namespace jstd
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

}
namespace tc = jstd;

#endif /* E48BE300_4482_4C64_BC3D_90198EE230EC */
