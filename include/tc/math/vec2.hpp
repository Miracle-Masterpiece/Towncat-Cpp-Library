#ifndef DC2D5C98_7CE7_435D_AC32_038712A77A32
#define DC2D5C98_7CE7_435D_AC32_038712A77A32

#include <tc/internal/cpp/lang/math/base_vec.hpp>
#include <tc/traits/primitive_traits.hpp>
#include <tc/string.hpp>

namespace tc
{

template<typename Ttype>
struct base_vec2 : detail::tvec<Ttype, 2, base_vec2<Ttype>> {
    
    typedef detail::tvec<Ttype, 2, base_vec2<Ttype>> base_vec;
    typedef base_vec2<Ttype> vec2;

    base_vec2() : base_vec() { }
    base_vec2(const Ttype& x, const Ttype& y) : base_vec(x, y) {}

#define TC_VECTOR_2_MAKE_ACCESS_FUNC(name, index)\
    Ttype& name() {return this->data()[index];}\
    Ttype name() const {return this->data()[index];}
    TC_VECTOR_2_MAKE_ACCESS_FUNC(x, 0);
    TC_VECTOR_2_MAKE_ACCESS_FUNC(r, 0);

    TC_VECTOR_2_MAKE_ACCESS_FUNC(y, 1);
    TC_VECTOR_2_MAKE_ACCESS_FUNC(g, 1);
#undef TC_VECTOR_2_MAKE_ACCESS_FUNC
    
    base_vec2<Ttype> cross() const {
        Ttype x_ = this->x();
        Ttype y_ = this->y();
        return base_vec2<Ttype>(-y_, x_);
    }

    vec2 xy() const {return vec2(x(), y());}
    vec2 yx() const {return vec2(y(), x());}

    tc::string to_string(tca::allocator* alloc = tca::get_default_allocator()) const {
        tc::string result(alloc);
        result
        .append("[x=").append(tc::to_string(x()))
        .append(", y=").append(tc::to_string(y()))
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
    struct vec_for<Ttype, 2> {
        typedef base_vec2<Ttype> type;
    };
} //namespace detail
} //namespace tc

/**
 * typedefs
 */
namespace tc
{
    typedef base_vec2<float>   vec2;
    typedef base_vec2<double> dvec2;
    
    typedef base_vec2<typename int_of<8>::stype> i8vec2;
    typedef base_vec2<typename int_of<8>::utype> u8vec2;
    
    typedef base_vec2<typename int_of<16>::stype> i16vec2;
    typedef base_vec2<typename int_of<16>::utype> u16vec2;
    
    typedef base_vec2<typename int_of<32>::stype> i32vec2;
    typedef base_vec2<typename int_of<32>::utype> u32vec2;
    
    typedef base_vec2<typename int_of<64>::stype> i64vec2;
    typedef base_vec2<typename int_of<64>::utype> u64vec2;
} // namespace tc

#endif /* DC2D5C98_7CE7_435D_AC32_038712A77A32 */
