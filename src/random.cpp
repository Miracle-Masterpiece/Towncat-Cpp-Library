#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/math/math.hpp>

namespace tc
{

    unsigned long long random::random_seed() {
        return (unsigned long long) (system::current_time_millis() & 0xffff);
    }
    
    unsigned long long random::next() {
        const unsigned long long a = 0x2363239423U;
        const unsigned long long c = 17U;
        m_seed = (m_seed * a + c);
        unsigned long long result = m_seed;
        result ^= result >> 16;
        result *= 0xa68bU;
        return result;
    }
    
    random::random(unsigned long long seed) : m_seed(seed) {

    }
    
    random::random(const random& rnd) : m_seed(rnd.m_seed)  {

    }
    
    random::random(random&& rnd) : m_seed(rnd.m_seed) {

    }
    
    random& random::operator=(const random& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random& random::operator=(random&& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random::~random() {

    }

    template<>
    float random::next() {
        unsigned long long v = next();
        return (float) math::abs(v) / (float) num_limits<unsigned long long>::max();
    }

    template<>
    double random::next() {
        unsigned long long v = next();
        return (double) math::abs(v) / (double) num_limits<unsigned long long>::max();
    }

    template<>
    long double random::next() {
        unsigned long long v = next();
        return (long double) math::abs(v) / (long double) num_limits<unsigned long long>::max();
    }

    template<>
    bool random::next() {
        return ((next() >> 31) & 1) != 0;
    }
}