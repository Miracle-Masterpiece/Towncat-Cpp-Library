#ifndef E9332171_F057_47FF_A991_D66BB9A08AC9
#define E9332171_F057_47FF_A991_D66BB9A08AC9

#include <tc/internal/cpp/lang/math/base_vec.hpp>
#include <tc/traits/primitive_traits.hpp>
#include <tc/string.hpp>

namespace tc
{

template<typename Ttype>
struct base_vec1 : detail::tvec<Ttype, 1, base_vec2<Ttype>> {
    
    typedef detail::tvec<Ttype, 1, base_vec1<Ttype>> base_vec;
    typedef base_vec1<Ttype> vec1;

    base_vec1() : base_vec() { }
    base_vec1(const Ttype& x) : base_vec(x) {}

#define TC_VECTOR_1_MAKE_ACCESS_FUNC(name, index)\
    Ttype& name() {return this->data()[index];}\
    Ttype name() const {return this->data()[index];}
    TC_VECTOR_1_MAKE_ACCESS_FUNC(x, 0);
    TC_VECTOR_1_MAKE_ACCESS_FUNC(r, 0);
#undef TC_VECTOR_2_MAKE_ACCESS_FUNC

    vec1 x() const {return vec1(x());}

    tc::string to_string(tca::allocator* alloc = tca::get_default_allocator()) const {
        tc::string result(alloc);
        result
        .append("[x=").append(tc::to_string(x()))
        .append("]");
        return result;
    }
};

} // namespace tc

/**
 * traits
 */
namespace tc
{
namespace detail
{
    template<typename Ttype>
    struct vec_for<Ttype, 1> {
        typedef base_vec1<Ttype> type;
    };
} //namespace detail
} //namespace tc

/**
 * typedefs
 */
namespace tc
{
    typedef base_vec1<float>   vec1;
    typedef base_vec1<double> dvec1;
    
    typedef base_vec1<typename int_of<8>::stype> i8vec1;
    typedef base_vec1<typename int_of<8>::utype> u8vec1;
    
    typedef base_vec1<typename int_of<16>::stype> i16vec1;
    typedef base_vec1<typename int_of<16>::utype> u16vec1;
    
    typedef base_vec1<typename int_of<32>::stype> i32vec1;
    typedef base_vec1<typename int_of<32>::utype> u32vec1;
    
    typedef base_vec1<typename int_of<64>::stype> i64vec1;
    typedef base_vec1<typename int_of<64>::utype> u64vec1;
} // namespace tc

#endif /* E9332171_F057_47FF_A991_D66BB9A08AC9 */
