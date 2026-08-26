#ifndef JSTD_CPP_LANG_TYPES_H
#define JSTD_CPP_LANG_TYPES_H

#include <cpp/lang/traits/primitive_traits.hpp>

namespace tc
{
    typedef unsigned long long timepoint;
} //namespace tc

namespace tc
{
namespace internal
{
    typedef typename int_of<32>::utype len_type;
} //namespace internal
} //namespace tc


#endif//JSTD_CPP_LANG_TYPES_H