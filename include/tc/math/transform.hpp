#ifndef F873C0FA_74E7_43A5_9182_41DA60053862
#define F873C0FA_74E7_43A5_9182_41DA60053862

#include <tc/internal/cpp/lang/math/base_mat.hpp>
#include <tc/internal/cpp/lang/math/base_vec.hpp>
#include <tc/math/quat.hpp>
#include <tc/math.hpp>

namespace tc
{
    template<typename Ttype>
    base_mat<Ttype, 2, 2> scale(
        const base_mat<Ttype, 2, 2>& mat, const Ttype& x, const Ttype& y
    ) {
        return mat * base_mat<Ttype, 2, 2>(
            x, 0,
            0, y
        );
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 2, 2> scale(
        const base_mat<Ttype, 2, 2>& mat,
        const tc::detail::base_vec<Ttype, 2, Tderived>& vec
    ) {
        return scale(mat, vec[0], vec[1]);
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 2, 2> rotate(
        const base_mat<Ttype, 2, 2>& mat,
        const Ttype& angle
    ) {
        Ttype sin = math::sin(angle);
        Ttype cos = math::cos(angle);
        return mat * base_mat<Ttype, 2, 2>(
            cos, -sin,
            sin,  cos
        );
    }
}

namespace tc
{
    template<typename Ttype>
    base_mat<Ttype, 3, 3> scale(
        const base_mat<Ttype, 3, 3>& mat, Ttype x, Ttype y
    ) {
        return mat * base_mat<Ttype, 3, 3>(
            x, 0, 0,
            0, y, 0,
            0, 0, 1
        );
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 3, 3> scale(
        const base_mat<Ttype, 3, 3>& mat,
        const tc::detail::base_vec<Ttype, 2, Tderived>& vec
    ) {
        return scale(mat, vec[0], vec[1]);
    }
    
    template<typename Ttype>
    base_mat<Ttype, 3, 3> translate(
        const base_mat<Ttype, 3, 3>& mat, Ttype x, Ttype y
    ) {
        return mat * base_mat<Ttype, 3, 3>(
            1, 0, x,
            0, 1, y,
            0, 0, 1
        );
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 3, 3> translate(
        const base_mat<Ttype, 3, 3>& mat,
        const tc::detail::base_vec<Ttype, 2, Tderived>& vec
    ) {
        return translate(mat, vec[0], vec[1]);
    }

    template<typename Ttype>
    base_mat<Ttype, 3, 3> rotate(
        const base_mat<Ttype, 3, 3>& mat, Ttype angle
    ) {
        Ttype sin = math::sin(angle);
        Ttype cos = math::cos(angle);
        return mat * base_mat<Ttype, 3, 3>(
            cos, -sin, 0,
            sin,  cos, 0,
            0,    0,   1
        );
    }
}

namespace tc
{
    template<typename Ttype>
    base_mat<Ttype, 4, 4> scale(
        const base_mat<Ttype, 4, 4>& mat, 
        Ttype x, Ttype y, Ttype z
    ) {
        return mat * base_mat<Ttype, 4, 4>(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 4, 4> scale(
        const base_mat<Ttype, 4, 4>& mat, 
        const tc::detail::base_vec<Ttype, 3, Tderived>& vec
    ) {
        return scale(mat, vec[0], vec[1], vec[2]);
    }
    
    template<typename Ttype>
    base_mat<Ttype, 4, 4> translate(
        const base_mat<Ttype, 4, 4>& mat, 
        Ttype x, Ttype y, Ttype z
    ) {
        return mat * base_mat<Ttype, 4, 4>(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        );
    }
    
    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 4, 4> translate(
        const base_mat<Ttype, 4, 4>& mat, 
        const tc::detail::base_vec<Ttype, 3, Tderived>& vec
    ) {
        return translate(mat, vec[0], vec[1], vec[2]);
    }

    template<typename Ttype>
    base_mat<Ttype, 4, 4> rotate(
        const base_mat<Ttype, 4, 4>& mat,
        Ttype angle, Ttype x, Ttype y, Ttype z
    ) {
        Ttype c = math::cos(angle);
        Ttype s = math::sin(angle);
        Ttype t = Ttype(1) - c;

        Ttype x2 = x*x;
        Ttype y2 = y*y;
        Ttype z2 = z*z;

        return mat * base_mat<Ttype, 4, 4>(
            t*x2  + c,      t*x*y - s*z,    t*x*z + s*y,    0,
            t*x*y + s*z,    t*y2 + c,       t*y*z - s*x,    0,
            t*x*z - s * y,  t*y*z + s*x,    t*z2 + c,       0,
            0,              0,              0,              1
        );
    }

    template<typename Ttype, typename Tderived>
    base_mat<Ttype, 4, 4> rotate(
        const base_mat<Ttype, 4, 4>& mat, 
        Ttype angle,
        const tc::detail::base_vec<Ttype, 3, Tderived>& vec
    ) {
        return rotate(mat, angle, vec[0], vec[1], vec[2]);
    }
}

namespace tc
{    
#ifdef TC_FORCE_DEPTH_ZERO_TO_ONE
    inline base_mat<float, 4, 4> frustum(float left, float right, float bottom, float top, float near, float far) {
        const float _00    = (2 * near)            /   (right - left);
        const float _11    = (2 * near)           /   (top - bottom); 
        const float A      = (right + left)        /   (right - left);
        const float B      = -((top + bottom)       /   (top - bottom));
        const float C      = -(far / (far - near));
        const float D      = -((far * near)        /   (far - near));

        return base_mat<float, 4, 4>(
            _00, 0,     A,      0, 
            0,  _11,    B,      0,
            0,   0,     C,      D,
            0,   0,    -1,      0
        );
    }

    inline base_mat<float, 4, 4> perspective(float fov, float aspect, float near, float far) {
        const float half_fov_tan = tan(fov / 2);
        const float _00 = 1.0f / (aspect * half_fov_tan);
        const float _11 = 1.0f / half_fov_tan; 
        const float _22 = -(far / (far - near));
        const float _23 = -((far * near) / (far - near));

        return base_mat<float, 4, 4>(
            _00,    0,    0,    0, 
            0,     _11,   0,    0,
            0,      0,   _22,  _23,
            0,      0,   -1,    0
        );
    }
    inline base_mat<float, 4, 4> ortho(float left, float right, float bottom, float top, float near, float far) {
        const float _m00 = 2.0f / (right - left);
        const float _m11 = 2.0f / (top - bottom); 
        const float _m22 = -1.0f / (far - near);

        const float tx = -((right + left) / (right - left));
        const float ty = ((top + bottom) / (top - bottom)); 
        const float tz = -(near / (far - near));

        return base_mat<float, 4, 4>(
            _m00,   0,      0,      tx, 
            0,      _m11,   0,      ty,
            0,      0,      _m22,   tz,
            0,      0,      0,      1
        );
    }
#else
    inline base_mat<float, 4, 4> ortho(float left, float right, float bottom, float top, float near, float far) {
        const float _m00 = 2.0f / (right - left);
        const float _m11 = 2.0f / (top - bottom);
        const float _m22 =-2.0f / (far - near);

        const float tx = -((right + left) / (right - left));
        const float ty = -((top + bottom) / (top - bottom));
        const float tz = -((far + near)   / (far - near));

        return base_mat<float, 4,4>(
            _m00,   0,      0,      tx, 
            0,      _m11,   0,      ty,
            0,      0,      _m22,   tz,
            0,      0,      0,      1
        );
    }     

    inline base_mat<float, 4, 4> frustum(float left, float right, float bottom, float top, float near, float far) {
        const float _00    = (2 * near)            /   (right -  left);
        const float _11    = (2 * near)            /   (top - bottom);
        const float A      = (right + left)        /   (right - left);
        const float B      = (top + bottom)        /   (top - bottom);
        const float C      = -((far + near)        /   (far - near));
        const float D      = -((2 * far * near)    /   (far - near));

        return base_mat<float, 4, 4>(
            _00, 0,     A,      0, 
            0,  _11,    B,      0,
            0,   0,     C,      D,
            0,   0,    -1,      0
        );
    } 
    
    inline base_mat<float, 4, 4> perspective(float fov, float aspect, float near, float far) {
        const float half_fov_tan = tan(fov / 2);
        const float _00 = 1.0f / (aspect * half_fov_tan);
        const float _11 = 1.0f / half_fov_tan;
        const float _22 = -((far + near) / (far - near));
        const float _23 = -((2 * far * near) / (far - near));

        return base_mat<float, 4, 4>(
            _00,    0,    0,    0, 
            0,     _11,   0,    0,
            0,      0,   _22,  _23,
            0,      0,   -1,    0
        );
    } 
#endif
}

namespace tc
{

    inline quat from_equler_x(float rad) {
        const float w = cos(rad * 0.5f);
        const float x = sin(rad * 0.5f);
        const float y = 0;
        const float z = 0;
        return quat(x, y, z, w);
    }

    inline quat from_equler_y(float rad) {
        const float w = cos(rad * 0.5f);
        const float x = 0;
        const float y = sin(rad * 0.5f);
        const float z = 0;
        return quat(x, y, z, w);
    }

    inline quat from_equler_z(float rad) {
        const float w = cos(rad * 0.5f);
        const float x = 0;
        const float y = 0;
        const float z = sin(rad * 0.5f);
        return quat(x, y, z, w);
    }

    inline base_mat<float, 4, 4> quat_to_matrix(const quat& q){
        const float x = q.x;
        const float y = q.y;
        const float z = q.z;
        const float w = q.w;

        const float m0 = 1.0f - 2.0f * (y*y + z*z);
        const float m1 = 2.0f * (x * y - w * z);
        const float m2 = 2.0f * (w * y + x * z);
        const float m3 = 0.0f;
        
        const float m4 = 2.0f * (x * y + w * z);
        const float m5 = 1.0f - 2.0f * (x*x + z*z);
        const float m6 = 2.0f * (y * z - w * x);
        const float m7 = 0.0f;

        const float m8  = 2.0f * (x * z - w * y);
        const float m9  = 2.0f * (w * x + y * z);
        const float m10 = 1.0f - 2.0f * (x*x + y*y);
        const float m11 = 0.0f;
        
        const float m12 = 0.0f;
        const float m13 = 0.0f;
        const float m14 = 0.0f;
        const float m15 = 1.0f;

        return base_mat<float, 4, 4>(
                    m0, m1, m2,  m3,
                    m4, m5, m6,  m7,
                    m8, m9, m10, m11,
                    m12,m13,m14, m15
                );
    }
}

#endif /* F873C0FA_74E7_43A5_9182_41DA60053862 */
