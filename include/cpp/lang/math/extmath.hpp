#ifndef JSTD_CPP_LANG_MATH_EXT_MATH_H
#define JSTD_CPP_LANG_MATH_EXT_MATH_H

#include <cpp/lang/math/math.hpp>
#include <cpp/lang/math/mat4.hpp>
#include <cpp/lang/math/vec4.hpp>
#include <cpp/lang/math/vec3.hpp>
#include <cpp/lang/math/quat.hpp>

namespace tc
{

namespace math
{
    inline mat4 scale(
        float x, 
        float y, 
        float z) {
        return mat4 (
                    x, 0, 0, 0, 
                    0, y, 0, 0,
                    0, 0, z, 0,
                    0, 0, 0, 1
                );
    }

    template<typename VEC>
    mat4 scale(const VEC& v) {
        return scale(v.x, v.y, v.z);
    }

    inline mat4 identity() {
        return scale(1, 1, 1);
    }

    inline mat4 translate(
        float x, 
        float y, 
        float z) {
        return mat4 (
                    1, 0, 0, x, 
                    0, 1, 0, y,
                    0, 0, 1, z,
                    0, 0, 0, 1
                );
    }

    template<typename VEC>
    mat4 translate(const VEC& v) {
        return translate(v.x, v.y, v.z);
    }
    
    inline mat4 x_rotate(float rad) {
        float s = sin(rad);
        float c = cos(rad);
        return mat4 (
                    1,  0,  0,  0, 
                    0,  c, -s, 0,
                    0,  s,  c,  0,
                    0,  0,  0,  1
                );
    }

    inline mat4 y_rotate(float rad) {
        float s = sin(rad);
        float c = cos(rad);
        return mat4 (
            c,  0,  s,  0, 
            0,  1,  0,  0,
           -s,  0,  c,  0,
            0,  0,  0,  1
        );
    }

    inline mat4 z_rotate(float rad) {
        float s = sin(rad);
        float c = cos(rad);
        return mat4(
            c, -s,  0,  0, 
            s,  c,  0,  0,
            0,  0,  1,  0,
            0,  0,  0,  1
        );
    }

    inline mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
        const float _00    = (2 * near)            /   (right -  left);
        const float _11    = (2 * near)            /   (top - bottom);
        const float A      = (right + left)        /   (right - left);
        const float B      = (top + bottom)        /   (top - bottom);
        const float C      = -((far + near)        /   (far - near));
        const float D      = -((2 * far * near)    /   (far - near));

        return mat4(
            _00, 0,     A,      0, 
            0,  _11,    B,      0,
            0,   0,     C,      D,
            0,   0,    -1,      0
        );
    }

    inline mat4 perspective(float fov, float aspect, float near, float far) {
        const float half_fov_tan = tan(fov / 2);
        const float _00 = 1.0f / (aspect * half_fov_tan);
        const float _11 = 1.0f / half_fov_tan;
        const float _22 = -((far + near) / (far - near));
        const float _23 = -((2 * far * near) / (far - near));

        return mat4(
            _00,    0,    0,    0, 
            0,     _11,   0,    0,
            0,      0,   _22,  _23,
            0,      0,   -1,    0
        );
    }
    
    inline mat4 ortho(float left, float right, float bottom, float top, float near, float far) {

        const float _m00 = 2.0f / (right - left);
        const float _m11 = 2.0f / (top - bottom);
        const float _m22 =-2.0f / (far - near);

        const float tx = -((right + left) / (right - left));
        const float ty = -((top + bottom) / (top - bottom));
        const float tz = -((far + near)   / (far - near));

        return mat4(
            _m00,   0,      0,      tx, 
            0,      _m11,   0,      ty,
            0,      0,      _m22,   tz,
            0,      0,      0,      1
        );
    }

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
        using internal::real_t;
        const float w = cos(rad * 0.5f);
        const float x = 0;
        const float y = 0;
        const float z = sin(rad * 0.5f);
        return quat(x, y, z, w);
    }

    inline double pow(double x, double power) {
        return ::pow(x, power);
    }

    inline long double pow(long double x, long double power) {
        return ::powl(x, power);
    }

    inline float pow(float x, float power) {
        return ::powf(x, power);
    }

    inline mat4 quat_to_matrix(const quat& q){
        const float x = q.x;
        const float y = q.y;
        const float z = q.z;
        const float w = q.w;

        const float _0 = 1.0f - 2.0f * (y*y + z*z);
        const float _1 = 2.0f * (x * y - w * z);
        const float _2 = 2.0f * (w * y + x * z);
        const float _3 = 0.0f;
        
        const float _4 = 2.0f * (x * y + w * z);
        const float _5 = 1.0f - 2.0f * (x*x + z*z);
        const float _6 = 2.0f * (y * z - w * x);
        const float _7 = 0.0f;

        const float _8  = 2.0f * (x * z - w * y);
        const float _9  = 2.0f * (w * x + y * z);
        const float _10 = 1.0f - 2.0f * (x*x + y*y);
        const float _11 = 0.0f;
        
        const float _12 = 0.0f;
        const float _13 = 0.0f;
        const float _14 = 0.0f;
        const float _15 = 1.0f;

        return mat4(
                    _0, _1, _2, _3,
                    _4, _5, _6, _7,
                    _8, _9, _10,_11,
                    _12,_13,_14,_15
                );
    }

}//namespace math
}//namespace jstd
#endif//JSTD_CPP_LANG_MATH_EXT_MATH_H