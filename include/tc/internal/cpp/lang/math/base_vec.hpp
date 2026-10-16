#ifndef F885541C_D893_416D_BF39_5F486BA62175
#define F885541C_D893_416D_BF39_5F486BA62175

#include <cstddef>
#include <type_traits>
#include <cmath>
#include <cstring>
#include <tc/objects.hpp>

namespace tc
{
namespace detail
{
    struct float_policy {};
    struct integral_policy {};

    /**
     * 
     */
    template<typename T>
    struct policy_for {typedef integral_policy type;};

    /**
     * 
     */
    template<>
    struct policy_for<float> {typedef float_policy type;};

    /**
     * 
     */
    template<>
    struct policy_for<double> {typedef float_policy type;};

    /**
     * 
     */
    template<>
    struct policy_for<long double> {typedef float_policy type;};
} //namespace detail
} //namespace tc

namespace tc
{
namespace detail
{

template<typename Ttype, std::size_t Tsize>
struct vec_for;

template<typename Ttype, std::size_t Tsize, typename Tderived>
struct base_vec {
    
    static constexpr std::size_t SIZE = Tsize;

    Ttype* data()  {
        return m_data;
    }
    
    const Ttype* data()  const {
        return m_data;
    }

    Tderived operator+ (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data() = data()[i] + rhs.data()[i];
        return result;
    }
    
    Tderived operator- (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = data()[i] - rhs.data()[i];
        return result;
    }
    
    Tderived operator* (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = data()[i] * rhs.data()[i];
        return result;
    }
    
    Tderived operator/ (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = data()[i] / rhs.data()[i];
        return result;
    }

    Tderived operator-() const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = -data()[i];
        return result;
    }

    Ttype length_sqr() const {
        Ttype result = Ttype(0);
        for (std::size_t i = 0; i < Tsize; ++i)
        {
            const Ttype component = data()[i];
            result += component * component;
        }
        return result;
    }
    
    Ttype dot(const Tderived& rhs) const {
        Ttype result = Ttype(0);
        for (std::size_t i = 0; i < Tsize; ++i)
        {
            result += data()[i] * rhs.data()[i];
        }
        return result;
    }

    Ttype& operator[] (std::size_t idx) {
        return data()[idx];
    }
    
    const Ttype& operator[] (std::size_t idx) const {
        return data()[idx];
    }
    
    Ttype& operator() (std::size_t idx) {
        return data()[idx];
    }
    
    const Ttype& operator() (std::size_t idx) const {
        return data()[idx];
    }

    template<typename... Args>
    base_vec(Args&&... args) : m_data {std::forward<Args>(args)...} {
    
    }

    std::size_t hashcode() const {
        return objects::hashcode(data(), data() + Tsize, hash_for<Ttype>());
    }

    template<typename Tvec>
    bool equals(const base_vec<Ttype, Tsize, Tvec>& other) const {
        return objects::equals(
            data(), data() + Tsize,
            other.data(), other.data() + Tsize,
            equal_to<Ttype>()
        );
    }

private:
    Ttype m_data[Tsize];
};

template<typename Ttype, std::size_t Tsize, typename Tderived, typename Tpolicy = typename policy_for<Ttype>::type>
struct tvec;

template<typename Ttype, std::size_t Tsize, typename Tderived>
struct tvec<Ttype, Tsize, Tderived, float_policy> : base_vec<Ttype, Tsize, Tderived> {
    
    typedef base_vec<Ttype, Tsize, Tderived> base_vec;
    
    template<typename... Args>
    tvec(Args&&... args) :
        base_vec(std::forward<Args>(args)...)
    {
    }

    Ttype length() const {
        Ttype lensqr = this->length_sqr();
        return static_cast<Ttype>(std::sqrt(lensqr));
    }
    
    Tderived normalized() const {
        Tderived result;
        Ttype len    = length();
        for (std::size_t i = 0; i < Tsize; ++i)
        {
            result.data()[i] = this->data()[i] / len;
        }
        return result;
    }
};

template<typename Ttype, std::size_t Tsize, typename Tderived>
struct tvec<Ttype, Tsize, Tderived, integral_policy> : base_vec<Ttype, Tsize, Tderived> {
    
    typedef base_vec<Ttype, Tsize, Tderived> base_vec;
    
    template<typename... Args>
    tvec(Args&&... args) :
        base_vec(std::forward<Args>(args)...)
    {
    }

    Tderived operator& (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] & rhs.data()[i];
        return result;
    }
    
    Tderived operator| (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] | rhs.data()[i];
        return result;
    }
    
    Tderived operator^ (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] ^ rhs.data()[i];
        return result;
    }
    
    Tderived operator<< (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] << rhs.data()[i];
        return result;
    }
    
    Tderived operator>> (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] >> rhs.data()[i];
        return result;
    }
    
    Tderived operator% (const Tderived& rhs) const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = this->data()[i] % rhs.data()[i];
        return result;
    }
    
    Tderived& operator++ () {
        for (std::size_t i = 0; i < Tsize; ++i)
            ++this->data()[i];
        return static_cast<Tderived&>(*this);
    }
    
    Tderived operator++ (int) {
        Tderived result = (*this);
        for (std::size_t i = 0; i < Tsize; ++i)
            this->data()[i]++;
        return result;
    }
    
    Tderived& operator-- () {
        for (std::size_t i = 0; i < Tsize; ++i)
            --this->data()[i];
        return static_cast<Tderived&>(*this);
    }
    
    Tderived operator-- (int) {
        Tderived result = (*this);
        for (std::size_t i = 0; i < Tsize; ++i)
            this->data()[i]--;
        return result;
    }
    
    Tderived operator~ () const {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = ~this->data()[i];
        return result;
    }

};

} // namespace detail
} // namespace tc

/**
 * Base vector operators
 */
namespace tc
{
    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator* (const Ttype& s, const detail::base_vec<Ttype, Tsize, Tderived>& v) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] * s;
        return result;
    }
    
    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator* (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Ttype& s) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] * s;
        return result;
    }

    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator/ (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Ttype& s) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] / s;
        return result;
    }

    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator+ (const Ttype& s, const detail::base_vec<Ttype, Tsize, Tderived>& v) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] + s;
        return result;
    }
    
    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator+ (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Ttype& s) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] + s;
        return result;
    }
    
    template<typename Tderived, typename Ttype, std::size_t Tsize>
    Tderived operator- (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Ttype& s) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] - s;
        return result;
    }
} //namespace tc

/**
 * Integral vector operators
 */
namespace tc
{
    /**
     *          A N D
     */

    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator& (const Tmask& mask, const detail::base_vec<Ttype, Tsize, Tderived>& v) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] & static_cast<Ttype>(mask);
        return result;
    }
    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator& (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& mask) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] & static_cast<Ttype>(mask);
        return result;
    }
    
    /**
     *          O R
     */

    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator| (const Tmask& mask, const detail::base_vec<Ttype, Tsize, Tderived>& v) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] | static_cast<Ttype>(mask);
        return result;
    }
    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator| (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& mask) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] | static_cast<Ttype>(mask);
        return result;
    }
    
    /**
     *          X O R
     */

    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator^ (const Tmask& mask, const detail::base_vec<Ttype, Tsize, Tderived>& v) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] ^ static_cast<Ttype>(mask);
        return result;
    }
    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator^ (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& mask) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] ^ static_cast<Ttype>(mask);
        return result;
    }
    
    /**
     *          L S H I F T
     */    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator<< (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& mask) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] << static_cast<Ttype>(mask);
        return result;
    }
    
    /**
     *          R S H I F T
     */    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator>> (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& mask) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] >> static_cast<Ttype>(mask);
        return result;
    }
    
    /**
     *          M O D
     */    
    template<
        typename Tderived,
        typename Ttype,
        std::size_t Tsize,
        typename Tmask,
        typename = typename std::enable_if<std::is_integral<Ttype>::value>::type
    >
    Tderived operator% (const detail::base_vec<Ttype, Tsize, Tderived>& v, const Tmask& m) {
        Tderived result;
        for (std::size_t i = 0; i < Tsize; ++i)
            result.data()[i] = v.data()[i] % static_cast<Ttype>(m);
        return result;
    }

} //namespace tc

#endif /* F885541C_D893_416D_BF39_5F486BA62175 */