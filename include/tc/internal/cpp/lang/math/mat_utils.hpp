#ifndef B5267C80_8840_4ED8_992B_FE3F0DAF02F1
#define B5267C80_8840_4ED8_992B_FE3F0DAF02F1

#include <tc/internal/cpp/lang/math/base_mat.hpp>

namespace tc
{
    template<typename Ttype>
    Ttype determinant(const base_mat<Ttype, 2, 2>& mat) {
        Ttype a = mat(0, 0), b = mat(0, 1);
        Ttype c = mat(1, 0), d = mat(1, 1);
        Ttype det = a*d - b*c;
        return det;
    }

    template<typename Ttype>
    Ttype determinant(const base_mat<Ttype, 3, 3>& mat) {
        Ttype a = mat(0, 0), b = mat(0, 1), c = mat(0, 2);
        Ttype d = mat(1, 0), e = mat(1, 1), f = mat(1, 2);
        Ttype g = mat(2, 0), h = mat(2, 1), i = mat(2, 2);
        Ttype det = 
                (a * e * i) +  (b * f * g) +  (c * d * h) +
               -(c * e * g) + -(a * f * h) + -(b * d * i);
        return det;
    }

    template<typename Ttype>
    Ttype minor(
        const base_mat<Ttype, 3, 3>& mat,
        unsigned i,
        unsigned j
    ) {
        Ttype m[2][2];
        unsigned ri = 0, ci = 0;

        for (unsigned r = 0; r < 3; ++r)
        {
            if (r == i) continue;
            for (unsigned c = 0; c < 3; ++c)
            {
                if (c == j) continue;
                m[ri][ci++] = mat(r, c);
            }
            ++ri;
            ci = 0;
        }

        return m[0][0] * m[1][1] - 
            m[0][1] * m[1][0];
    }

    template<typename Ttype>
    Ttype cofactor(const base_mat<Ttype, 3, 3>& mat, unsigned i, unsigned j) {
        Ttype sign = ((i + j) % 2) == 0 ? Ttype(1) : Ttype(-1);
        return sign * minor(mat, i, j);
    }

    template<typename Ttype>
    base_mat<Ttype, 2, 2> inverse(const base_mat<Ttype, 2, 2>& mat) {
        base_mat<Ttype, 2, 2> result;

        Ttype det = determinant(mat);

        Ttype a = mat(0, 0), b = mat(0, 1);
        Ttype c = mat(1, 0), d = mat(1, 1);
        result(0,0) =  d / det; result(0,1) = -b / det;
        result(1,0) = -c / det; result(1,1) =  a / det;

        return result;
    }

    template<typename Ttype>
    base_mat<Ttype, 3, 3> inverse(const base_mat<Ttype, 3, 3>& mat) {
        base_mat<Ttype, 3, 3> result;

        const Ttype det       = determinant(mat);
        const Ttype inv_det   = Ttype(1) / det;

        for (unsigned r = 0; r < 3; ++r)
        {
            for (unsigned c = 0; c < 3; ++c)
            {
                result(c, r) = cofactor(mat, r, c) * inv_det;
            }
        }

        return result;
    }

#if 0
template<typename Ttype>
base_mat<Ttype, 4, 4> inverse(const base_mat<Ttype, 4, 4>& mat) {
    base_mat<Ttype, 4, 4> result;

    Ttype a = mat(0,0), b = mat(0,1), c = mat(0,2), d = mat(0,3);
    Ttype e = mat(1,0), f = mat(1,1), g = mat(1,2), h = mat(1,3);
    Ttype i = mat(2,0), j = mat(2,1), k = mat(2,2), l = mat(2,3);
    Ttype m = mat(3,0), n = mat(3,1), p = mat(3,2), q = mat(3,3);

    /*
        a b c d | 1 0 0 0
        e f g h | 0 1 0 0
        i j k l | 0 0 1 0
        m n o p | 0 0 0 1
    */

    return result;
}
#endif

}



#endif /* B5267C80_8840_4ED8_992B_FE3F0DAF02F1 */