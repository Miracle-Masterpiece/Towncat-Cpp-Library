#ifndef JSTD_CPP_LANG_MATH_MATH_H
#define JSTD_CPP_LANG_MATH_MATH_H

#include <cmath>

namespace tc
{
namespace math
{

    /**
     * 
     */
    constexpr double PI = 3.141592653589793;
    
    /**
     * 
     */
    constexpr double HALF_PI  = PI / 2.0;

    /**
     * 
     */
    constexpr double E = 2.718281828459045;

    /**
     * 
     */
    template<typename T>
    constexpr T lerp(const T& a, const T& b, const T& t) {
        return (b - a) * t + a;
    }

    /**
     * 
     */
    constexpr double to_radians(double angle) {
        return angle * (PI / 180.0);
    }

    /**
     * 
     */
    constexpr double to_degrees(double rad) {
        return rad * (180.0 / PI);
    }

    /**
     * 
     */
    constexpr float to_radians(float angle) {
        return angle * (static_cast<float>(PI) / 180.0f);
    }

    /**
     * 
     */
    constexpr float to_degrees(float rad) {
        return rad * (180.0f / static_cast<float>(PI));
    }

    /**
     * 
     */
    constexpr long double to_radians(long double angle) {
        return angle * (PI / 180.0);
    }

    /**
     * 
     */
    constexpr long double to_degrees(long double rad) {
        return rad * (180.0 / PI);
    }

    /**
     * 
     */
    inline long double sin(long double x) {
        return ::sinl(x);
    }

    /**
     * 
     */
    inline double sin(double x) {
        return ::sin(x);
    }

    /**
     * 
     */
    inline float sin(float x) {
        return ::sinf(x);
    }
    
    /**
     * 
     */
    inline long double cos(long double x) {
        return ::cosl(x);
    }

    /**
     * 
     */
    inline double cos(double x) {
        return ::cos(x);
    }

    /**
     * 
     */
    inline float cos(float x) {
        return ::cosf(x);
    }

    /**
     * 
     */
    inline double tan(double x) {
        return ::tan(x);
    }

    /**
     * 
     */
    inline long double tan(long double x) {
        return ::tanl(x);
    }
    
    /**
     * 
     */
    inline float tan(float x) {
        return ::tanf(x);
    }

    /**
     * 
     */
    inline double ctg(double x) {
        return cos(x) / sin(x);
    }

    /**
     * 
     */
    inline long double ctg(long double x) {
        return cos(x) / sin(x);
    }

    /**
     * 
     */
    inline float ctg(float x) {
        return cos(x) / sin(x);
    }

    /**
     * 
     */
    inline float asin(float x) {
        return ::asinf(x);
    }

    /**
     * 
     */
    inline double asin(double x) {
        return ::asin(x);
    }

    /**
     * 
     */
    inline long double asin(long double x) {
        return ::asinl(x);
    }

    /**
     * 
     */
    inline float acos(float x) {
        return ::acosf(x);
    }

    /**
     * 
     */
    inline double acos(double x) {
        return ::acos(x);
    }

    /**
     * 
     */
    inline long double acos(long double x) {
        return ::acosl(x);
    }

    /**
     * 
     */
    template<typename T>
    inline T sqrt(T val) {
        return (T) ::sqrt((double) val);
    }

    /**
     * 
     */
    template<>
    inline double sqrt<double>(double x) {
        return ::sqrt(x);
    }    

    /**
     * 
     */
    template<>
    inline long double sqrt<long double>(long double x) {
        return ::sqrtl(x);
    }

    /**
     * 
     */
    template<>
    inline float sqrt<float>(float x) {
        return ::sqrtf(x);
    }

    /**
     * 
     */
    inline double pow(double x, double power) {
        return ::pow(x, power);
    }

    /**
     * 
     */
    inline float pow(float x, float power) {
        return ::powf(x, power);
    }

    /**
     * 
     */
    inline long double pow(long double x, long double power) {
        return ::powl(x, power);
    }

    /**
     * 
     */
    inline long double atan2(long double y, long double x) {
        return ::atan2l(y, x);
    }

    /**
     * 
     */
    inline double atan2(double y, double x) {
        return ::atan2(y, x);
    }

    /**
     * 
     */
    inline float atan2(float y, float x) {
        return ::atan2f(y, x);
    }
    
    /**
     * 
     */
    template<typename T>
    constexpr T quintic(const T& x) {
        return x * x * x * (x * (x * T(6) - T(15)) + T(10));
    }

    /**
     * 
     */
    inline double cosine(double x) {
        return (1 - cos(PI * x)) / 2;
    }

    /**
     * 
     */
    inline float cosine(float x) {
        return (1 - cos(static_cast<float>(PI) * x)) / 2;
    }

    /**
     * 
     */
    inline long double cosine(long double x) {
        return (1 - cos(PI * x)) / 2;
    }

    /**
     * 
     */
    template<typename T>
    T min(const T& a, const T& b) {
        return a < b ? a : b;
    }
    
    /**
     * 
     */
    template<typename T>
    T max(const T& a, const T& b) {
        return a > b ? a : b;
    }

    /**
     * 
     */
    template<typename T>
    T abs(const T& x) {
        return x < 0 ? -x : x;
    }

    /**
     * 
     */
    inline float floor(float x) {
        return ::floorf(x);
    }

    /**
     * 
     */
    inline double floor(double x) {
        return ::floor(x);
    }

    /**
     * 
     */
    inline long double floor(long double x) {
        return ::floorl(x);
    }
    
    /**
     * 
     */
    inline float ceil(float x) {
        return ::ceilf(x);
    }

    /**
     * 
     */
    inline double ceil(double x) {
        return ::ceil(x);
    }

    /**
     * 
     */
    inline long double ceil(long double x) {
        return ::ceill(x);
    }
    
    /**
     * 
     */
    inline float round(float x) {
        return ::roundf(x);
    }
    
    /**
     * 
     */
    inline double round(double x) {
        return ::round(x);
    }
    
    /**
     * 
     */
    inline long double round(long double x) {
        return ::roundl(x);
    }

    /**
     * 
     */
    template<typename T>
    T hypot(const T& x, const T& y) {
        return sqrt<T>(x*x + y*y);
    }

    /**
     * 
     */
    template<typename T>
    T hypot(const T& x, const T& y, const T& z) {
        return sqrt<T>(x*x + y*y + z*z);
    }

    /**
     * 
     */
    template<typename T>
    T clamp(const T& val, const T& min, const T& max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    /**
     * @return
     *      Псевдо-случайное значение в диапазоне [0, 1 (не включительно!)]
     */
    double random();

}// namespace math
}// namespace jstd

#endif//JSTD_CPP_LANG_MATH_MATH_H