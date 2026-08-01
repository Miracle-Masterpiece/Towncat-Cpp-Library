#include <cpp/lang/utils/smooth_noise.hpp>
#include <cpp/lang/math/math.hpp>

namespace tc
{

    smooth_noise::smooth_noise(curve_func curve) : m_curve_func(curve) {

    }

    smooth_noise::smooth_noise(unsigned long long seed, curve_func curve) : smooth_noise(curve) {
        set_seed(seed);
        generate_values();
    }

    smooth_noise::smooth_noise(const smooth_noise& n) : smooth_noise(n.m_seed, n.m_curve_func) {
        std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
    }

    smooth_noise::smooth_noise(smooth_noise&& n) : smooth_noise(n.m_seed, n.m_curve_func) {

    }
    
    smooth_noise& smooth_noise::operator=(const smooth_noise& n) {
        if (&n != this) {
            std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
            m_seed          = n.m_seed;
            m_curve_func    = n.m_curve_func;
        }
        return *this;
    }
    
    smooth_noise& smooth_noise::operator=(smooth_noise&& n) {
        if (&n != this) {
            std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
            m_seed          = n.m_seed;
            m_curve_func    = n.m_curve_func;
        }
        return *this;
    }
    
    smooth_noise::~smooth_noise() {

    }

    unsigned long long smooth_noise::get_seed() const {
        return m_seed;
    }
    
    void smooth_noise::set_seed(unsigned long long seed) {
        m_seed = seed;
    }

    void smooth_noise::generate_values() {
        random(m_seed).values<signed char>(m_buffer, BUF_SIZE, num_limits<signed char>::max());
    }

    static unsigned long hash2d(long x, long y) {
        const unsigned long ux = (unsigned  long) x;
        const unsigned long uy = (unsigned  long) y;
        unsigned long hash = (ux * 0x0B00B135U) + (uy * 0x50FFC001U);
        hash ^= hash >> 13U;
        hash += hash & 832749825U;
        hash ^= hash >> 16U;
        return hash;
    }

    signed char smooth_noise::value_at(long x, long y) const {
        unsigned long hash = hash2d(x, y);
        return m_buffer[hash & (BUF_SIZE - 1)];
    }

    static long floor_div(long num, long denom) {
        long   r = num / denom;
        return num >= 0 ? r : r - 1;
    }

    signed char smooth_noise::get0(long x, long y, int scale) const {
        if (scale == 0)
            return 0;

        long xChunk = floor_div(x, scale);
        long yChunk = floor_div(y, scale);

        long xStart = xChunk * scale;
        long yStart = yChunk * scale;

        long xc = x - xStart;
        long yc = y - yStart;

        signed char v0 = value_at(xChunk,     yChunk);
        signed char v1 = value_at(xChunk + 1, yChunk);

        signed char v2 = value_at(xChunk,     yChunk + 1);
        signed char v3 = value_at(xChunk + 1, yChunk + 1);

        signed char a = (signed char) ( ((v1 - v0) * xc / scale) + v0 );
        signed char b = (signed char) ( ((v3 - v2) * xc / scale) + v2 );        
        signed char c = (signed char) ( (( b - a ) * yc / scale) + a  );

        return c;
    }

    num::q16 smooth_noise::get1(long x, long y, int scale) const {
        if (scale == 0)
            return 0;

        long xChunk = floor_div(x, scale);
        long yChunk = floor_div(y, scale);

        long xStart = xChunk * scale;
        long yStart = yChunk * scale;

        long xc = x - xStart;
        long yc = y - yStart;

        signed char v0 = value_at(xChunk,     yChunk);
        signed char v1 = value_at(xChunk + 1, yChunk);

        signed char v2 = value_at(xChunk,     yChunk + 1);
        signed char v3 = value_at(xChunk + 1, yChunk + 1);

        using num::q16;

        q16 tx = q16(xc) / q16(scale);
        q16 ty = q16(yc) / q16(scale);

        tx = m_curve_func(tx);
        ty = m_curve_func(ty);

        q16 a = math::lerp<q16>(v0, v1, tx);
        q16 b = math::lerp<q16>(v2, v3, tx);
        q16 c = math::lerp<q16>(a,  b,  ty);

        return c / q16(127);
    }

    float smooth_noise::get(long x, long y, int scale) const {
        return get1(x, y, scale);
    }

    float smooth_noise::get(long x, long y, int scale, int octaves, int scale_factor /* = 2 */, int freq_factor /* = 2 */) const {
        JSTD_DEBUG_CODE(
            if (scale        <= 0) throw_except<illegal_argument_exception>("Invalid scale: %i",scale);
            if (octaves      <= 0) throw_except<illegal_argument_exception>("Invalid count octaves: %i", octaves);
            if (scale_factor < 0 ) throw_except<illegal_argument_exception>("Invalid scale_factor octaves: %f", scale_factor);
            if (freq_factor  < 0 ) throw_except<illegal_argument_exception>("Invalid freeq_factor octaves: %f", freq_factor);
        );

        using num::q16;

        int freq       = 1;
        q16 result     = 0;
        q16 amplitude  = 1;
        q16 weigth_sum = 0;

        while (octaves > 0)
        {
            int octave_scale = scale / freq;
            if (octave_scale == 0) break;
            
            result     += get1(x, y, octave_scale) * amplitude;
            weigth_sum += amplitude;
            amplitude  /= q16(scale_factor);
            freq       *= freq_factor;
            
            --octaves;
        }
        
        return (float) (result / weigth_sum);
    }

    void smooth_noise::set_curve_fuc(curve_func func) {
        JSTD_DEBUG_CODE(
            if (func == nullptr)
                throw_except<illegal_argument_exception>("func pointer == null");
        );
        m_curve_func = func;
    }

    /*static*/ num::q16 smooth_noise::no_smooth(const num::q16& x) {
        return x;
    }
    
}

// #include <cpp/lang/utils/images/image.hpp>
// #include <cpp/lang/utils/images/imageio.hpp>

// #include <stdio.h>
// #include <math.h>
// #include <stdlib.h>

// int main() {
//     using namespace tc;
//     int size = 1024;
//     image img(size, size, 1);
    
//     smooth_noise noise(system::current_time_millis(), math::quintic);

//     timepoint last = system::nano_time();
//     for (int i = 0; i < size; ++i)
//     {
//         int xo = -10000000;
//         int yo = 10000000;
        
//         for (int j = 0; j < size; ++j)
//         {
//             // float h = noise.get(xo + i, yo + j, 128, 8, 4, 4);
//             float h = noise.get(xo + i, yo + j, 1024, 8, 2, 2);
//             img.get_gray(i, j) = image::gray((unsigned char) (h * 255));

//         }
//     }
//     timepoint now = system::nano_time();
//     printf("passed: %f\n", (double) (now - last) / 1000000.0);

//     imageio::write_image(tc::file("./rnd.png"), &img, ".png");
// }