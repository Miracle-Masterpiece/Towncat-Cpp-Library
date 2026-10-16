#ifndef AA32D2B0_9A63_41AC_8654_5420D057DE9A
#define AA32D2B0_9A63_41AC_8654_5420D057DE9A

#include <tc/internal/cpp/lang/math/base_vec.hpp>
#include <tc/traits/primitive_traits.hpp>
#include <tc/math/vec2.hpp>
#include <tc/math/vec3.hpp>
#include <tc/string.hpp>

namespace tc
{

template<typename Ttype>
struct base_vec4 : detail::tvec<Ttype, 4, base_vec4<Ttype>> {
    
    typedef detail::tvec<Ttype, 4, base_vec4<Ttype>> base_vec;

    typedef typename detail::vec_for<Ttype, 2>::type vec2;
    typedef typename detail::vec_for<Ttype, 3>::type vec3;
    typedef typename detail::vec_for<Ttype, 4>::type vec4;

    base_vec4() :
        base_vec() 
    {
    }
    
    base_vec4(const base_vec2<Ttype>& v, const Ttype& z, const Ttype& w) :
        base_vec(v.x(), v.y(), z, w) 
    {
    }
    
    base_vec4(const base_vec2<Ttype>& v1, const base_vec2<Ttype>& v2) :
        base_vec(v1.x(), v1.y(), v2.x(), v2.y()) 
    {
    }
    
    base_vec4(const base_vec3<Ttype>& v, const Ttype& w) :
        base_vec(v.x(), v.y(), v.z(), w) 
    {
    }

    base_vec4(
        const Ttype& x,
        const Ttype& y,
        const Ttype& z,
        const Ttype& w
    ) :
        base_vec(x, y, z, w)
    {
    }

#define TC_VECTOR_4_MAKE_ACCESS_FUNC(name, index)\
    Ttype& name() {return this->data()[index];}\
    Ttype name() const {return this->data()[index];}
    TC_VECTOR_4_MAKE_ACCESS_FUNC(x, 0);
    TC_VECTOR_4_MAKE_ACCESS_FUNC(r, 0);

    TC_VECTOR_4_MAKE_ACCESS_FUNC(y, 1);
    TC_VECTOR_4_MAKE_ACCESS_FUNC(g, 1);
    
    TC_VECTOR_4_MAKE_ACCESS_FUNC(z, 2);
    TC_VECTOR_4_MAKE_ACCESS_FUNC(b, 2);
    
    TC_VECTOR_4_MAKE_ACCESS_FUNC(w, 3);
    TC_VECTOR_4_MAKE_ACCESS_FUNC(a, 3);
#undef TC_VECTOR_4_MAKE_ACCESS_FUNC

    base_vec4<Ttype> cross(const base_vec4<Ttype>& rhs) const {
        Ttype x0 = this->x();
        Ttype y0 = this->y();
        Ttype z0 = this->z();
        
        Ttype x1 = rhs.x();
        Ttype y1 = rhs.y();
        Ttype z1 = rhs.z();

        Ttype xr = y0 * z1 - z0 * y1;
        Ttype yr = z0 * x1 - z1 * x0;
        Ttype zr = x0 * y1 - y0 * x1;
        
        return base_vec4<Ttype>(xr, yr, zr, Ttype(0));
    }

    vec2 xy() const {return vec2(x(), y());}
    vec2 xz() const {return vec2(x(), z());}
    vec2 xw() const {return vec2(x(), w());}
    vec2 yx() const {return vec2(y(), x());}
    vec2 zx() const {return vec2(z(), x());}
    vec2 wx() const {return vec2(w(), x());}
    vec2 zy() const {return vec2(z(), y());}
    vec2 wy() const {return vec2(w(), y());}
    vec2 yz() const {return vec2(y(), z());}
    vec2 wz() const {return vec2(w(), z());}
    vec2 zw() const {return vec2(z(), w());}
    vec2 yw() const {return vec2(y(), w());}

    vec3 xyz() const {return vec3(x(), y(), z());}
    vec3 xyw() const {return vec3(x(), y(), w());}
    vec3 xzy() const {return vec3(x(), z(), y());}
    vec3 xzw() const {return vec3(x(), z(), w());}
    vec3 xwz() const {return vec3(x(), w(), z());}
    vec3 xwy() const {return vec3(x(), w(), y());}
    vec3 yxz() const {return vec3(y(), x(), z());}
    vec3 yxw() const {return vec3(y(), x(), w());}
    vec3 zxy() const {return vec3(z(), y(), y());}
    vec3 zxw() const {return vec3(z(), x(), w());}
    vec3 wxz() const {return vec3(w(), x(), z());}
    vec3 wxy() const {return vec3(w(), x(), y());}
    vec3 zyx() const {return vec3(z(), y(), x());}
    vec3 wyx() const {return vec3(w(), y(), x());}
    vec3 yzx() const {return vec3(y(), z(), x());}
    vec3 wzx() const {return vec3(w(), z(), x());}
    vec3 zwx() const {return vec3(z(), w(), x());}
    vec3 ywx() const {return vec3(y(), w(), x());}
    vec3 wzy() const {return vec3(w(), z(), y());}
    vec3 zwy() const {return vec3(z(), w(), y());}
    vec3 wyz() const {return vec3(w(), y(), z());}
    vec3 ywz() const {return vec3(y(), w(), z());}
    vec3 yzw() const {return vec3(y(), z(), w());}
    vec3 zyw() const {return vec3(z(), y(), w());}

    vec4 xyzw() const {return vec4(x(), y(), z(), w());}
    vec4 xywz() const {return vec4(x(), y(), w(), z());}
    vec4 xzyw() const {return vec4(x(), z(), y(), w());}
    vec4 xzwy() const {return vec4(x(), z(), w(), y());}
    vec4 xwzy() const {return vec4(x(), w(), z(), y());}
    vec4 xwyz() const {return vec4(x(), w(), y(), z());}
    vec4 yxzw() const {return vec4(y(), x(), z(), w());}
    vec4 yxwz() const {return vec4(y(), x(), w(), z());}
    vec4 zxyw() const {return vec4(z(), x(), y(), w());}
    vec4 zxwy() const {return vec4(z(), x(), w(), y());}
    vec4 wxzy() const {return vec4(w(), x(), z(), y());}
    vec4 wxyz() const {return vec4(w(), x(), y(), z());}
    vec4 zyxw() const {return vec4(z(), y(), x(), w());}   
    vec4 wyxz() const {return vec4(w(), y(), x(), z());}
    vec4 yzxw() const {return vec4(y(), z(), x(), w());}
    vec4 wzxy() const {return vec4(w(), z(), x(), y());}
    vec4 zwxy() const {return vec4(z(), w(), x(), y());}
    vec4 ywxz() const {return vec4(y(), w(), x(), z());}
    vec4 wzyx() const {return vec4(w(), z(), y(), x());}
    vec4 zwyx() const {return vec4(z(), w(), y(), x());}
    vec4 wyzx() const {return vec4(w(), y(), z(), x());}
    vec4 ywzx() const {return vec4(y(), w(), z(), x());}
    vec4 yzwx() const {return vec4(y(), z(), w(), x());}
    vec4 zywx() const {return vec4(z(), y(), w(), x());}

    tc::string to_string(tca::allocator* alloc = tca::get_default_allocator()) const {
        tc::string result(alloc);
        result
        .append("[x=").append(tc::to_string(x()))
        .append(", y=").append(tc::to_string(y()))
        .append(", z=").append(tc::to_string(z()))
        .append(", w=").append(tc::to_string(w()))
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
    struct vec_for<Ttype, 4> {
        typedef base_vec4<Ttype> type;
    };
} //namespace detail
} //namespace tc

namespace tc
{
    typedef base_vec4<float>   vec4;
    typedef base_vec4<double> dvec4;
    
    typedef base_vec4<typename int_of<8>::stype> i8vec4;
    typedef base_vec4<typename int_of<8>::utype> u8vec4;
    
    typedef base_vec4<typename int_of<16>::stype> i16vec4;
    typedef base_vec4<typename int_of<16>::utype> u16vec4;
    
    typedef base_vec4<typename int_of<32>::stype> i32vec4;
    typedef base_vec4<typename int_of<32>::utype> u32vec4;
    
    typedef base_vec4<typename int_of<64>::stype> i64vec4;
    typedef base_vec4<typename int_of<64>::utype> u64vec4;
} // namespace tc

#endif /* AA32D2B0_9A63_41AC_8654_5420D057DE9A */
