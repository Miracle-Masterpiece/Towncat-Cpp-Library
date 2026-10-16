#ifndef C1CFFCB7_990B_439F_9A32_AD10EE06BC32
#define C1CFFCB7_990B_439F_9A32_AD10EE06BC32

#include <tc/math/vec2.hpp>
#include <tc/traits/primitive_traits.hpp>
#include <tc/string.hpp>

namespace tc
{

template<typename Ttype>
struct base_vec3 : detail::tvec<Ttype, 3, base_vec3<Ttype>> {
    
    typedef detail::tvec<Ttype, 3, base_vec3<Ttype>> base_vec;

    typedef typename detail::vec_for<Ttype, 2>::type vec2;
    typedef typename detail::vec_for<Ttype, 3>::type vec3;

    base_vec3() :
        base_vec()
    {
    }
    
    base_vec3(const base_vec2<Ttype>& vec, const Ttype& z) :
        base_vec(vec.x(), vec.y(), z)
    {
    }

    base_vec3(const Ttype& x, const Ttype& y, const Ttype& z) :
        base_vec(x, y, z)
    {
    }

#define TC_VECTOR_3_MAKE_ACCESS_FUNC(name, index)\
    Ttype& name() {return this->data()[index];}\
    Ttype name() const {return this->data()[index];}
    
    TC_VECTOR_3_MAKE_ACCESS_FUNC(x, 0);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(r, 0);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(u, 0);

    TC_VECTOR_3_MAKE_ACCESS_FUNC(y, 1);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(g, 1);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(v, 1);
    
    TC_VECTOR_3_MAKE_ACCESS_FUNC(z, 2);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(b, 2);
    TC_VECTOR_3_MAKE_ACCESS_FUNC(w, 2);
#undef TC_VECTOR_3_MAKE_ACCESS_FUNC

    base_vec3<Ttype> cross(const base_vec3<Ttype>& rhs) const {
        Ttype x0 = this->x();
        Ttype y0 = this->y();
        Ttype z0 = this->z();
        
        Ttype x1 = rhs.x();
        Ttype y1 = rhs.y();
        Ttype z1 = rhs.z();

        Ttype xr = y0 * z1 - z0 * y1;
        Ttype yr = z0 * x1 - z1 * x0;
        Ttype zr = x0 * y1 - y0 * x1;
        
        return base_vec3<Ttype>(xr, yr, zr);
    }

    vec2 xy() const {return vec2(x(), y());}
    vec2 xz() const {return vec2(x(), z());}
    vec2 yx() const {return vec2(y(), x());}
    vec2 yz() const {return vec2(y(), z());}
    vec2 zx() const {return vec2(z(), x());}
    vec2 zy() const {return vec2(z(), y());}
    vec3 xyz() const {return vec3(x(), y(), z());}
    vec3 xzy() const {return vec3(x(), z(), y());}
    vec3 yxz() const {return vec3(y(), x(), z());}
    vec3 yzx() const {return vec3(y(), z(), x());}
    vec3 zxy() const {return vec3(z(), x(), y());}
    vec3 zyx() const {return vec3(z(), y(), x());}

    tc::string to_string(tca::allocator* alloc = tca::get_default_allocator()) const {
        tc::string result(alloc);
        result
        .append("[x=").append(tc::to_string(x()))
        .append(", y=").append(tc::to_string(y()))
        .append(", z=").append(tc::to_string(z()))
        .append("]");
        return result;
    }
};

} // namespace tc

namespace tc
{
namespace detail
{
    template<typename Ttype>
    struct vec_for<Ttype, 3> {
        typedef base_vec3<Ttype> type;
    };
} //namespace detail
} //namespace tc

namespace tc
{
    typedef base_vec3<float>   vec3;
    typedef base_vec3<double> dvec3;
    
    typedef base_vec3<typename int_of<8>::stype> i8vec3;
    typedef base_vec3<typename int_of<8>::utype> u8vec3;
    
    typedef base_vec3<typename int_of<16>::stype> i16vec3;
    typedef base_vec3<typename int_of<16>::utype> u16vec3;
    
    typedef base_vec3<typename int_of<32>::stype> i32vec3;
    typedef base_vec3<typename int_of<32>::utype> u32vec3;
    
    typedef base_vec3<typename int_of<64>::stype> i64vec3;
    typedef base_vec3<typename int_of<64>::utype> u64vec3;
}

#endif /* C1CFFCB7_990B_439F_9A32_AD10EE06BC32 */