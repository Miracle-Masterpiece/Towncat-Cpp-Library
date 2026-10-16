#ifndef D827A90F_9E7E_4C96_8FF8_D5EC6E06EF3F
#define D827A90F_9E7E_4C96_8FF8_D5EC6E06EF3F

#include <tc/internal/cpp/lang/vectorize/vector.hpp>

#ifdef TC_ENABLE_VECTORIZE
#if __x86_64
#include <immintrin.h>

namespace tc
{

template<>
class vector<float, 4> {
    typedef vector<float, 4> Tvector;
    __m128 m_data;
public:
    
    vector() : vector(0, 0, 0, 0) {}

    vector(float a, float b, float c, float d) {
        m_data = _mm_set_ps(d, c, b, a);
    }

    Tvector operator+ (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm_add_ps(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator- (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm_sub_ps(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator* (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm_mul_ps(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator/ (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm_div_ps(m_data, rhs.m_data);
        return result;
    }

    void to_array_unaligned(float* arr) const {
        _mm_storeu_ps(arr, m_data);
    }
    
    void to_array_aligned(float* arr) const {
        _mm_store_ps(arr, m_data);
    }
};

template<>
class vector<double, 4> {
    typedef vector<double, 4> Tvector;
    __m256d m_data;
public:
    
    vector() : vector(0, 0, 0, 0) {}

    vector(double a, double b, double c, double d) {
        m_data = _mm256_set_pd(d, c, b, a);
    }

    Tvector operator+ (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm256_add_pd(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator- (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm256_sub_pd(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator* (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm256_mul_pd(m_data, rhs.m_data);
        return result;
    }
    
    Tvector operator/ (const Tvector& rhs) const {
        Tvector result;
        result.m_data = _mm256_div_pd(m_data, rhs.m_data);
        return result;
    }

    void to_array_unaligned(double* arr) const {
        _mm256_storeu_pd(arr, m_data);
    }
    
    void to_array_aligned(double* arr) const {
        _mm256_store_pd(arr, m_data);
    }
};

} //namespace tc

#endif //check x86_64
#endif //enable vectorize

#endif /* D827A90F_9E7E_4C96_8FF8_D5EC6E06EF3F */
