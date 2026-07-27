#ifndef JSTD_CPP_NUMBERS_H
#define JSTD_CPP_NUMBERS_H

#include <cstdint>
#include <climits>
#include <cpp/lang/math/math.hpp>
#include <cpp/lang/exceptions.hpp>

namespace jstd 
{

#if __SIZEOF_FLOAT__ == __SIZEOF_INT__
 typedef unsigned int uint_float_bits;
 typedef signed   int sint_float_bits;
#elif __SIZEOF_FLOAT__ == __SIZEOF_LONG__
 typedef unsigned long uint_float_bits;
 typedef signed   long sint_float_bits;
#elif __SIZEOF_FLOAT__ == __SIZEOF_LONG_LONG__
 typedef unsigned long long uint_float_bits;
 typedef signed   long long sint_float_bits;
#else
 #error None of the integer types are suitable for storing 'float' bits.
#endif

#if __SIZEOF_DOUBLE__ == __SIZEOF_INT__
 typedef unsigned int uint_double_bits;
 typedef signed   int sint_double_bits;
#elif __SIZEOF_DOUBLE__ == __SIZEOF_LONG__
 typedef unsigned long uint_double_bits;
 typedef signed   long sint_double_bits;
#elif __SIZEOF_DOUBLE__ == __SIZEOF_LONG_LONG__
 typedef unsigned long long uint_double_bits;
 typedef signed   long long sint_double_bits;
#else
 #error None of the integer types are suitable for storing 'double' bits.
#endif

namespace num 
{

class q16 {
    using store_type = signed long;
    using op_type    = signed long long;
    
    /**
     * 
     */
    static const int SCALE         = 0x10000;
    
    /**
     * 
     */
    static const int SCALE_SHIFT   = 16;
    
    /**
     * 
     */
    store_type val;

public:  
    
    /**
     * 
     */
    q16() : val(0) {}

    /**
     * 
     */
    q16(float v) : val((store_type) (v * SCALE)) {
        
    }
    
    /**
     * 
     */
    q16(double v) : val((store_type) (v * SCALE)) {
        
    }

    /**
     * 
     */
    q16(long v) : val(v * SCALE) {
        
    }
    
    /**
     * 
     */
    q16(int v) : val(v * SCALE) {
        
    }

    /**
     * 
     */
    q16(const q16&) = default;
    
    /**
     * 
     */
    q16& operator= (const q16&) = default;
    
    /**
     * 
     */
    operator float () const {
        return (float) val / (float) SCALE;
    }
    
    /**
     * 
     */
    q16 operator+ (const q16& q) const {
        return of_raw( val + q.val );
    }
    
    /**
     * 
     */
    q16 operator- (const q16& q) const {
        return of_raw( val - q.val );
    }
    
    /**
     * 
     */
    q16 operator* (const q16& q) const {
        op_type a = val;
        op_type b = q.val;
        op_type r = (a * b) >> SCALE_SHIFT;
        return of_raw( (store_type) r );
    }
    
    /**
     * 
     */
    q16 operator/ (const q16& q) const {
        op_type a = val;
        op_type b = q.val;
        op_type r = ((a << SCALE_SHIFT) / b);
        return of_raw( (store_type) r );
    }

    /**
     * 
     */
    q16& operator+= (const q16& q) {
        val += q.val;
        return *this;
    }
    
    /**
     * 
     */
    q16& operator-= (const q16& q) {
        val -= q.val;
        return *this;
    }
    
    /**
     * 
     */
    q16& operator*= (const q16& q) {
        const op_type a = val;
        const op_type b = q.val;
        const op_type r = (a * b) >> SCALE_SHIFT;
        val = (store_type) r;
        return *this;
    }
    /**
     * 
     */
    q16& operator/= (const q16& q) {
        const op_type a = val;
        const op_type b = q.val;
        const op_type r = ((a << SCALE_SHIFT) / b);
        val = (store_type) r;
        return *this;
    }
    
    static q16 of_raw(store_type x) {
        q16 v;
        v.val = x;
        return v;
    }
};

    /**
     * Преобразует значение float в его побитовое представление как uint_float_bits.
     *
     * Эта функция копирует байты из значения float в uint_float_bits без изменения битов.
     * Используется, когда необходимо проанализировать или сохранить точное битовое
     * представление числа с плавающей точкой.
     *
     * @param v 
     *      Значение типа float, которое нужно интерпретировать как uint_float_bits.
     * 
     * @return 
     *      Побитовое представление float в виде uint_float_bits.
     * 
     * @since 1.0
     */
    uint_float_bits float_to_uint_bits(float v);

    /**
     * Преобразует значение double в его побитовое представление как uint_double_bits.
     *
     * Эта функция копирует байты из значения double в uint_double_bits без изменения битов.
     * Используется, когда необходимо сохранить или передать точное битовое
     * представление double.
     *
     * @param v 
     *      Значение типа double, которое нужно интерпретировать как uint_double_bits.
     * 
     * @return 
     *      Побитовое представление double в виде uint_double_bits.
     * 
     * @since 1.0
     */
    uint_double_bits double_to_uint_bits(double v);

    /**
     * Является ли сивол числом в 10-ричной системе счисления.
     */
    template<typename CHAR_T>
    bool is_digit10(const CHAR_T& ch) {
        return ch >= (CHAR_T) '0' && ch <= (CHAR_T) '9';
    }
    
    /**
     * Является ли сивол числом в 16-ричной системе счисления.
     */
    template<typename CHAR_T>
    bool is_digit16(const CHAR_T& ch) {
        return
            (ch >= (CHAR_T) '0' && ch <= (CHAR_T) '9') ||
            (ch >= (CHAR_T) 'a' && ch <= (CHAR_T) 'f') ||
            (ch >= (CHAR_T) 'A' && ch <= (CHAR_T) 'F');
    }

    /**
     * Является ли сивол знаком минуса.
     */
    template<typename CHAR_T>
    bool is_sign(const CHAR_T& ch) {
        return ch == (CHAR_T) '-';
    }

namespace internal
{
    /**
     * 
     */
    template<typename CHAR_T>
    std::size_t str_len(const CHAR_T* s) {
        std::size_t len = 0;
        while (*s++) ++len;
        return len;
    }
}
    template<typename INT_T, typename STR_T>
    INT_T parse_int(const STR_T* s, std::size_t len = ~(std::size_t) 0) {
        if (len == ~(std::size_t) 0)
        {
            len = internal::str_len(s);
        }
        
        INT_T result     = 0;
        INT_T digit      = 1;
        bool is_negative = false;

        for (std::size_t i = len; i > 0;)
        {
            --i;
            if (!is_digit10(s[i]))
            {
                if (i == 0 && is_sign(s[i]))
                {
                    is_negative = true;
                    continue;
                }
                throw_except<number_format_exception>("Invalid integer string");
            }
            result += (INT_T) ((s[i] - 48) * digit);
            digit *= 10;
        }

        return is_negative ? -result : result;
    }

    template<typename FLOAT_T, typename STR_T>
    FLOAT_T parse_float(const STR_T* s, std::size_t len = ~(std::size_t) 0) {
        if (len == ~(std::size_t) 0)
        {
            len = internal::str_len(s);
        }

        static const STR_T DOT       = 46;
        static const STR_T START_NUM = 48;

        bool is_negative      = false;
        FLOAT_T result        = 0;
        std::size_t dot_index = len;

        for (std::size_t i = 0; i < len; ++i)
        {
            if (s[i] == DOT)
            {
                dot_index = i;
                break;
            }
        }

        {//parse ceil part
            FLOAT_T digit = 1;
            for (std::size_t i = dot_index; i > 0; )
            {
                --i;
                if (!is_digit10(s[i]))
                {
                    if (i == 0 && is_sign(s[i]))
                    {
                        is_negative = true;
                        continue;
                    }
                    throw_except<number_format_exception>("Invalid floating string");
                }
                result += (s[i] - START_NUM) * digit;
                digit  *= 10;
            }
        }

        {//parse fract part
            if (dot_index < len)
            {
                FLOAT_T digit = (FLOAT_T) 0.1;
                for (std::size_t i = dot_index + 1; i < len; ++i)
                {
                    if (!is_digit10(s[i]))
                        throw_except<number_format_exception>("Invalid floating string");
                        
                    result += (s[i] - START_NUM) * digit;
                    digit  *= (FLOAT_T) 0.1;
                }
            }
        }

        return is_negative ? -result : result;
    }

}//namespace num

}//namespace jstd

#endif//JSTD_CPP_NUMBERS_H