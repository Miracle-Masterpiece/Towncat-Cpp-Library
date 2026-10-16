#ifndef E7ED2F4F_C282_4CAC_8616_59DAD39E2E85
#define E7ED2F4F_C282_4CAC_8616_59DAD39E2E85

#include <tc/internal/cpp/lang/math/base_vec.hpp>
#include <cstddef>
#ifndef NDEBUG
# include <cassert>
#endif

namespace tc
{

template<typename Ttype, std::size_t Trow, std::size_t Tcol>
struct base_mat {
    
    typedef base_mat<Ttype, Tcol, Trow> Ttransposed;

    base_mat() :
        m_data {}
    {}

    base_mat(const Ttype& v) {
        for (std::size_t i = 0; i < Trow; ++i)
            for (std::size_t j = 0; j < Tcol; ++j)
                if (i == j)
                    at(i, j) = v;
                else
                    at(i, j) = 0;
    }

    template<typename... Args, typename = typename std::enable_if<sizeof...(Args) == Trow * Tcol>::type>
    base_mat(Args&&... args) {
        Ttype tmp[] {static_cast<Ttype>(args)...};
        std::size_t i = 0;
        for (std::size_t r = 0; r < Trow; ++r)
            for (std::size_t c = 0; c < Tcol; ++c)
                at(r, c) = tmp[i++]; 
    }
    
    const Ttype* data() const {
        return m_data;
    }
    
    Ttype* data() {
        return m_data;
    }

    Ttype& operator() (std::size_t row, std::size_t col) {
        return at(row, col);
    }
    
    const Ttype& operator() (std::size_t row, std::size_t col) const {
        return at(row, col);
    }

    Ttransposed transposed() const {
        Ttransposed result;
        for (std::size_t c = 0; c < Tcol; ++c)
            for (std::size_t r = 0; r < Trow; ++r)
                result(r, c) = at(c, r);
        return result;
    }

    template<std::size_t N>
    base_mat<Ttype, Trow, N> operator* (const base_mat<Ttype, Tcol, N>& rhs) const {
        base_mat<Ttype, Trow, N> result;

        for (std::size_t r = 0; r < Trow; ++r)
        {
            for (std::size_t c = 0; c < N; ++c)
            {
                Ttype sum = Ttype(0);
                for (std::size_t k = 0; k < Tcol; ++k)
                {
                    sum += at(r, k) * rhs.at(k, c);
                }
                result(r, c) += sum;
            }
        }

        return result;
    }

    base_mat<Ttype, Trow, Tcol> operator+ (const base_mat<Ttype, Trow, Tcol>& rhs) const {
        base_mat<Ttype, Trow, Tcol> result;
        for (std::size_t i = 0; i < Trow * Tcol; ++i)
            result.data()[i] = data()[i] + rhs.data()[i];
        return result;
    }
    
    base_mat<Ttype, Trow, Tcol>& operator+= (const base_mat<Ttype, Trow, Tcol>& rhs) {
        for (std::size_t i = 0; i < Trow * Tcol; ++i)
            data()[i] += rhs.data()[i];
        return *this;
    }
    
    base_mat<Ttype, Trow, Tcol> operator- (const base_mat<Ttype, Trow, Tcol>& rhs) const {
        base_mat<Ttype, Trow, Tcol> result;
        for (std::size_t i = 0; i < Trow * Tcol; ++i)
            result.data()[i] = data()[i] - rhs.data()[i];
        return result;
    }
    
    base_mat<Ttype, Trow, Tcol>& operator-= (const base_mat<Ttype, Trow, Tcol>& rhs) {
        for (std::size_t i = 0; i < Trow * Tcol; ++i)
            data()[i] -= rhs.data()[i];
        return *this;
    }

    Ttype& at(std::size_t row, std::size_t col) {
        return data()[index_at(row, col)];
    }

    const Ttype& at(std::size_t row, std::size_t col) const {
        return data()[index_at(row, col)];
    }

private:
    static std::size_t index_at(std::size_t row, std::size_t col) {
        #ifndef NDEBUG
        assert((row * Tcol + col) < (Trow * Tcol));
        #endif
        return row * Tcol + col;
    }
    Ttype m_data[Trow * Tcol];
};

}

/**
 * OPERATORS
 */
namespace tc
{
    template<typename Ttype, std::size_t Trow, std::size_t Tcol, typename Tderived>
    typename detail::vec_for<Ttype, Trow>::type
    operator* (const base_mat<Ttype, Trow, Tcol>& lhs, const detail::base_vec<Ttype, Tcol, Tderived>& vec){
        typename detail::vec_for<Ttype, Trow>::type result;

        for (std::size_t r = 0; r < Trow; ++r)
        {
            Ttype sum = Ttype(0);
            for (std::size_t k = 0; k < Tcol; ++k)
            {
                sum += lhs(r, k) * vec(k);
            }
            result(r) = sum;
        }

        return result;
    }

    template<typename Ttype, std::size_t N, std::size_t Tcol, typename Tderived>
    typename detail::vec_for<Ttype, Tcol>::type
    operator* (const detail::base_vec<Ttype, N, Tderived>& vec, const base_mat<Ttype, N, Tcol>& rhs) {
        typename detail::vec_for<Ttype, Tcol>::type result;
        
        for (std::size_t c = 0; c < Tcol; ++c)
        {
            Ttype sum = Ttype(0);
            for (std::size_t k = 0; k < N; ++k)
            {
                sum += vec(k) * rhs(k, c);
            }
            result(c) = sum;
        }
        
        return result;
    }
}

#endif /* E7ED2F4F_C282_4CAC_8616_59DAD39E2E85 */
